# -*- coding: utf-8 -*-
# render_obj_debug.py
# Versão com debugging: imprime VBO/IBO, calcula stride dinâmico,
# envia matrizes transpostas (numpy -> OpenGL) e desenha fallback de pontos.

import sys, os, math, traceback
import numpy as np, ctypes
import glfw
from OpenGL.GL import *
from OpenGL.GL.shaders import compileShader, compileProgram

# -------------------------
# utilitários (idem)
# -------------------------
def normalize(v):
    v = np.array(v, dtype=np.float32)
    n = np.linalg.norm(v)
    if n < 1e-8:
        return v
    return v / n

def perspective(fov_rad, aspect, znear, zfar):
    f = 1.0 / math.tan(fov_rad * 0.5)
    M = np.zeros((4,4), dtype=np.float32)
    M[0,0] = f / aspect
    M[1,1] = f
    M[2,2] = (zfar + znear) / (znear - zfar)
    M[2,3] = (2.0 * zfar * znear) / (znear - zfar)
    M[3,2] = -1.0
    return M

def look_at(eye, center, up):
    eye = np.array(eye, dtype=np.float32)
    center = np.array(center, dtype=np.float32)
    up = np.array(up, dtype=np.float32)
    f = normalize(center - eye)
    s = normalize(np.cross(f, up))
    u = np.cross(s, f)
    M = np.identity(4, dtype=np.float32)
    M[0,0:3] = s
    M[1,0:3] = u
    M[2,0:3] = -f
    T = np.identity(4, dtype=np.float32)
    T[0,3] = -eye[0]
    T[1,3] = -eye[1]
    T[2,3] = -eye[2]
    return M @ T

def quat_from_axis_angle(axis, angle):
    axis = normalize(axis)
    s = math.sin(angle * 0.5)
    return np.array([axis[0]*s, axis[1]*s, axis[2]*s, math.cos(angle*0.5)], dtype=np.float32)

def quat_mul(q2, q1):
    x1,y1,z1,w1 = q1
    x2,y2,z2,w2 = q2
    return np.array([
        w2*x1 + x2*w1 + y2*z1 - z2*y1,
        w2*y1 - x2*z1 + y2*w1 + z2*x1,
        w2*z1 + x2*y1 - y2*x1 + z2*w1,
        w2*w1 - x2*x1 - y2*y1 - z2*z1
    ], dtype=np.float32)

def quat_to_mat4(q):
    x,y,z,w = q
    xx, yy, zz = x*x, y*y, z*z
    xy, xz, yz = x*y, x*z, y*z
    wx, wy, wz = w*x, w*y, w*z
    M = np.identity(4, dtype=np.float32)
    M[0,0] = 1.0 - 2.0*(yy+zz)
    M[0,1] = 2.0*(xy - wz)
    M[0,2] = 2.0*(xz + wy)
    M[1,0] = 2.0*(xy + wz)
    M[1,1] = 1.0 - 2.0*(xx+zz)
    M[1,2] = 2.0*(yz - wx)
    M[2,0] = 2.0*(xz - wy)
    M[2,1] = 2.0*(yz + wx)
    M[2,2] = 1.0 - 2.0*(xx+yy)
    return M

def arcball_vec(x, y, w, h):
    X = (2.0 * x / float(w)) - 1.0
    Y = 1.0 - (2.0 * y / float(h))
    v = np.array([X, Y, 0.0], dtype=np.float32)
    d2 = X*X + Y*Y
    if d2 <= 1.0:
        v[2] = math.sqrt(max(0.0, 1.0 - d2))
    else:
        v = normalize(v)
    return v

# -------------------------
# OBJ loader (idem ao anterior)
# -------------------------
class Mesh:
    def __init__(self):
        self.vertices = np.array([], dtype=np.float32)
        self.indices = np.array([], dtype=np.uint32)
        self.has_normals = False
        self.has_uvs = False

class Model:
    def __init__(self):
        self.mesh = Mesh()
        self.bbox_min = np.array([np.inf, np.inf, np.inf], dtype=np.float32)
        self.bbox_max = np.array([-np.inf, -np.inf, -np.inf], dtype=np.float32)

def load_obj(path):
    verts, norms, uvs, faces = [], [], [], []
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            s = line.strip()
            if not s or s.startswith('#'): continue
            parts = s.split()
            if parts[0] == 'v' and len(parts) >= 4:
                verts.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif parts[0] == 'vn' and len(parts) >= 4:
                norms.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif parts[0] == 'vt' and len(parts) >= 3:
                uvs.append((float(parts[1]), float(parts[2])))
            elif parts[0] == 'f':
                vlist = parts[1:]
                idxs = []
                for p in vlist:
                    sub = p.split('/')
                    v_idx = vt_idx = vn_idx = None
                    if len(sub) >= 1 and sub[0] != '':
                        v_idx = int(sub[0])
                    if len(sub) >= 2 and sub[1] != '':
                        vt_idx = int(sub[1])
                    if len(sub) >= 3 and sub[2] != '':
                        vn_idx = int(sub[2])
                    idxs.append((v_idx, vt_idx, vn_idx))
                for i in range(1, len(idxs)-1):
                    faces.append((idxs[0], idxs[i], idxs[i+1]))
    model = Model()
    mesh = model.mesh
    unique = {}
    verts_out = []
    inds_out = []
    has_uvs = len(uvs) > 0
    has_norms = len(norms) > 0
    def to_idx(i, n):
        if i is None: return None
        return i-1 if i > 0 else n + i
    for tri in faces:
        for tup in tri:
            vi, vti, vni = tup
            vi2 = to_idx(vi, len(verts))
            vti2 = to_idx(vti, len(uvs)) if has_uvs else None
            vni2 = to_idx(vni, len(norms)) if has_norms else None
            key = (vi2, vti2, vni2)
            if key not in unique:
                px,py,pz = verts[vi2]
                if has_norms and vni2 is not None:
                    nx,ny,nz = norms[vni2]
                else:
                    nx,ny,nz = (0.0,0.0,0.0)
                if has_uvs and vti2 is not None:
                    u,v = uvs[vti2]
                else:
                    u,v = (0.0,0.0)
                verts_out.extend([px,py,pz, nx,ny,nz, u,v])
                idx = len(verts_out)//8 - 1
                unique[key] = idx
                model.bbox_min = np.minimum(model.bbox_min, np.array([px,py,pz], dtype=np.float32))
                model.bbox_max = np.maximum(model.bbox_max, np.array([px,py,pz], dtype=np.float32))
            inds_out.append(unique[key])
    mesh.vertices = np.array(verts_out, dtype=np.float32) if verts_out else np.array([], dtype=np.float32)
    mesh.indices = np.array(inds_out, dtype=np.uint32)
    mesh.has_normals = has_norms
    mesh.has_uvs = has_uvs
    # compute normals if missing
    if not mesh.has_normals and mesh.vertices.size:
        V = mesh.vertices.reshape((-1,8))
        pos = V[:,0:3]
        norms_acc = np.zeros_like(pos)
        counts = np.zeros((pos.shape[0],), dtype=np.int32)
        if mesh.indices.size % 3 == 0:
            tri_idx = mesh.indices.reshape((-1,3))
            for a,b,c in tri_idx:
                p0,p1,p2 = pos[a], pos[b], pos[c]
                n = np.cross(p1-p0, p2-p0)
                n = normalize(n)
                norms_acc[a] += n; norms_acc[b] += n; norms_acc[c] += n
                counts[a]+=1; counts[b]+=1; counts[c]+=1
        for i in range(pos.shape[0]):
            if counts[i] > 0:
                n = normalize(norms_acc[i] / counts[i])
            else:
                n = np.array([0,0,1], dtype=np.float32)
            V[i,3:6] = n
        mesh.vertices = V.flatten().astype(np.float32)
        mesh.has_normals = True
    return model

# -------------------------
# shaders (same phong)
# -------------------------
VERT_SRC = """#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vPosW;
out vec3 vNormW;
out vec2 vUV;

void main() {
    vec4 posW = uModel * vec4(aPos, 1.0);
    vPosW = posW.xyz;
    vNormW = mat3(transpose(inverse(uModel))) * aNorm;
    vUV = aUV;
    gl_Position = uProj * uView * posW;
}
"""

FRAG_SRC = """#version 330 core
in vec3 vPosW;
in vec3 vNormW;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uNs;
uniform float uAlpha;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uEye;

void main() {
    vec3 N = normalize(vNormW);
    vec3 L = normalize(uLightPos - vPosW);
    vec3 V = normalize(uEye - vPosW);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(R, V), 0.0), uNs);

    vec3 ambient = uKa * 0.2;
    vec3 color = ambient + uKd * diff * uLightColor + uKs * spec * uLightColor;

    FragColor = vec4(color, uAlpha);
}
"""

# -------------------------
# App (com debug e correções)
# -------------------------
class App:
    def __init__(self, obj_path):
        if not glfw.init():
            raise RuntimeError("GLFW init failed")
        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
        if sys.platform == "darwin":
            glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)

        self.width, self.height = 1024, 768
        self.window = glfw.create_window(self.width, self.height, "OBJ Renderer - Debug", None, None)
        if not self.window:
            glfw.terminate(); raise RuntimeError("Failed to create window")
        glfw.make_context_current(self.window)

        print("GL Vendor:", glGetString(GL_VENDOR))
        print("GL Renderer:", glGetString(GL_RENDERER))
        print("GL Version:", glGetString(GL_VERSION))
        print("GLSL:", glGetString(GL_SHADING_LANGUAGE_VERSION))

        glfw.set_window_size_callback(self.window, self.on_resize)
        glfw.set_cursor_pos_callback(self.window, self.on_cursor)
        glfw.set_mouse_button_callback(self.window, self.on_mouse)
        glfw.set_scroll_callback(self.window, self.on_scroll)
        glfw.set_key_callback(self.window, self.on_key)

        glEnable(GL_DEPTH_TEST)
        glDisable(GL_CULL_FACE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
        glClearColor(0.08,0.08,0.10,1.0)

        # compile shaders with logs
        try:
            print("Compiling VS...")
            vs = compileShader(VERT_SRC, GL_VERTEX_SHADER); print("VS ok")
            print("Compiling FS...")
            fs = compileShader(FRAG_SRC, GL_FRAGMENT_SHADER); print("FS ok")
            print("Linking program...")
            self.program = compileProgram(vs, fs); print("Program linked")
        except Exception as e:
            print("Shader compile/link error:", e); traceback.print_exc(); glfw.terminate(); raise

        glUseProgram(self.program)

        # load model
        self.model = load_obj(obj_path)
        print("DEBUG: mesh.vertices.size =", self.model.mesh.vertices.size)
        print("DEBUG: mesh.indices.size =", self.model.mesh.indices.size)
        # print first floats and indices (helpful)
        if self.model.mesh.vertices.size:
            print("First 40 floats:", self.model.mesh.vertices[:40])
        if self.model.mesh.indices.size:
            print("First 60 indices:", self.model.mesh.indices[:60])

        if self.model.mesh.vertices.size == 0 or self.model.mesh.indices.size == 0:
            print("Empty model or no indices - drawing fallback points")
        self.vao, self.vbo, self.ebo, self.floats_per_vertex = self.create_buffers(self.model.mesh)

        # uniforms
        self.uModel = self.get_uniform("uModel")
        self.uView = self.get_uniform("uView")
        self.uProj = self.get_uniform("uProj")
        self.uLightPos = self.get_uniform("uLightPos")
        self.uLightColor = self.get_uniform("uLightColor")
        self.uEye = self.get_uniform("uEye")
        # material
        self.uKa = self.get_uniform("uKa"); self.uKd = self.get_uniform("uKd"); self.uKs = self.get_uniform("uKs")
        self.uNs = self.get_uniform("uNs"); self.uAlpha = self.get_uniform("uAlpha")

        if self.uKa != -1: glUniform3f(self.uKa, 0.1,0.1,0.1)
        if self.uKd != -1: glUniform3f(self.uKd, 0.8,0.75,0.7)
        if self.uKs != -1: glUniform3f(self.uKs, 0.2,0.2,0.2)
        if self.uNs != -1: glUniform1f(self.uNs, 32.0)
        if self.uAlpha != -1: glUniform1f(self.uAlpha, 1.0)

        # camera framing
        C = 0.5*(self.model.bbox_min + self.model.bbox_max)
        R = float(np.linalg.norm(self.model.bbox_max - C))
        self.center = C.astype(np.float32)
        self.radius = max(1e-6, R)
        # translate to center and auto-scale
        self.base_model = np.identity(4, dtype=np.float32)
        self.base_model[0,3] = -C[0]; self.base_model[1,3] = -C[1]; self.base_model[2,3] = -C[2]
        max_dim = np.max(self.model.bbox_max - self.model.bbox_min)
        scale = 1.0 if max_dim < 1e-6 else (1.0 / max_dim)
        S = np.identity(4, dtype=np.float32); S[0,0]=S[1,1]=S[2,2]=scale
        self.base_model = S @ self.base_model

        self.fov = math.radians(45.0)
        self.dist = float(self.radius / math.tan(self.fov*0.5) + 0.5*self.radius)
        self.eye = np.array([0.0,0.0,self.dist], dtype=np.float32)
        self.up = np.array([0.0,1.0,0.0], dtype=np.float32)

        self.view = look_at(self.eye, np.array([0.0,0.0,0.0], dtype=np.float32), self.up)
        self.proj = perspective(self.fov, self.width/self.height, 0.01, max(10.0, self.dist + 3.0*self.radius))

        # IMPORTANT: send matrices transposed (numpy uses row-major)
        if self.uView != -1: glUniformMatrix4fv(self.uView, 1, GL_FALSE, self.view.T)
        if self.uProj != -1: glUniformMatrix4fv(self.uProj, 1, GL_FALSE, self.proj.T)
        if self.uLightPos != -1: glUniform3f(self.uLightPos, self.dist*0.5, self.dist*0.7, self.dist*0.3)
        if self.uLightColor != -1: glUniform3f(self.uLightColor, 1.0,1.0,1.0)
        if self.uEye != -1: glUniform3f(self.uEye, self.eye[0], self.eye[1], self.eye[2])

        self.dragging = False; self.last_x = 0; self.last_y = 0
        self.rot_accum = np.array([0.0,0.0,0.0,1.0], dtype=np.float32)
        self.index_count = int(self.model.mesh.indices.size)
        self.show_points_fallback = (self.model.mesh.vertices.size == 0 or self.model.mesh.indices.size == 0)

        self.main_loop()

    def get_uniform(self, name):
        loc = glGetUniformLocation(self.program, name)
        if loc < 0:
            # don't spam, but useful for debug
            # print("Warning: uniform", name, "not found")
            return -1
        return loc

    def create_buffers(self, mesh):
        vao = glGenVertexArrays(1); vbo = glGenBuffers(1); ebo = glGenBuffers(1)
        glBindVertexArray(vao)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        if mesh.vertices.size:
            glBufferData(GL_ARRAY_BUFFER, mesh.vertices.nbytes, mesh.vertices, GL_STATIC_DRAW)
        else:
            glBufferData(GL_ARRAY_BUFFER, 4, None, GL_STATIC_DRAW)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
        if mesh.indices.size:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.nbytes, mesh.indices, GL_STATIC_DRAW)
        else:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4, None, GL_STATIC_DRAW)

        # infer floats per vertex: if vertex_count==0 default to 8
        floats_per_vertex = 8
        if mesh.vertices.size:
            # try common divisors 8,6,3
            for cand in (8,6,3):
                if (mesh.vertices.size % cand) == 0:
                    floats_per_vertex = cand
                    break
            # fallback: compute from vertex_count if possible
            # vertex_count = mesh.vertices.size // floats_per_vertex

        stride = floats_per_vertex * 4
        # enable attributes according to available floats
        # pos (3) always at offset 0
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(0))
        # normal?
        if floats_per_vertex >= 6:
            glEnableVertexAttribArray(1)
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(12))
        else:
            # disable if present
            try: glDisableVertexAttribArray(1)
            except: pass
        # uv?
        if floats_per_vertex >= 8:
            glEnableVertexAttribArray(2)
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(24))
        else:
            try: glDisableVertexAttribArray(2)
            except: pass

        glBindVertexArray(0)
        print(f"DEBUG: floats_per_vertex={floats_per_vertex}, stride={stride}")
        return vao, vbo, ebo, floats_per_vertex

    def on_resize(self, window, w, h):
        self.width, self.height = max(1,w), max(1,h)
        glViewport(0,0,self.width,self.height)
        self.proj = perspective(self.fov, self.width/self.height, 0.01, max(10.0, self.dist + 3.0*self.radius))
        if self.uProj != -1:
            glUniformMatrix4fv(self.uProj, 1, GL_FALSE, self.proj.T)

    def on_mouse(self, window, button, action, mods):
        if button == glfw.MOUSE_BUTTON_LEFT and action == glfw.PRESS:
            self.dragging = True
            x,y = glfw.get_cursor_pos(window)
            self.last_x, self.last_y = int(x), int(y)
        elif button == glfw.MOUSE_BUTTON_LEFT and action == glfw.RELEASE:
            self.dragging = False

    def on_cursor(self, window, x, y):
        if not self.dragging: return
        w,h = glfw.get_window_size(window)
        va = arcball_vec(self.last_x, self.last_y, w, h)
        vb = arcball_vec(int(x), int(y), w, h)
        dotp = float(np.clip(np.dot(va, vb), -1.0, 1.0))
        dotp = max(-1.0, min(1.0, dotp))
        angle = math.acos(dotp)
        axis = normalize(np.cross(va, vb))
        if not np.isnan(angle) and np.linalg.norm(axis) > 1e-6:
            q = quat_from_axis_angle(axis, angle)
            self.rot_accum = quat_mul(q, self.rot_accum)
        self.last_x, self.last_y = int(x), int(y)

    def on_scroll(self, window, xoffset, yoffset):
        self.eye[2] *= (1.0 - 0.1 * float(yoffset))
        self.view = look_at(self.eye, np.array([0.0,0.0,0.0], dtype=np.float32), self.up)
        if self.uView != -1:
            glUniformMatrix4fv(self.uView, 1, GL_FALSE, self.view.T)
        if self.uEye != -1:
            glUniform3f(self.uEye, self.eye[0], self.eye[1], self.eye[2])

    def on_key(self, window, key, scancode, action, mods):
        if action != glfw.PRESS: return
        if key == glfw.KEY_W:
            mode = glGetIntegerv(GL_POLYGON_MODE)
            self.show_wireframe = not getattr(self,'show_wireframe', False)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE if self.show_wireframe else GL_FILL)
            print("Wireframe:", self.show_wireframe)
        elif key == glfw.KEY_C:
            if glIsEnabled(GL_CULL_FACE):
                glDisable(GL_CULL_FACE); print("Cull disabled")
            else:
                glEnable(GL_CULL_FACE); glCullFace(GL_BACK); print("Cull enabled")
        elif key == glfw.KEY_ESCAPE:
            glfw.set_window_should_close(self.window, True)

    def main_loop(self):
        frame = 0
        while not glfw.window_should_close(self.window):
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            rotM = quat_to_mat4(self.rot_accum)
            modelM = rotM @ self.base_model
            # send model matrix transposed
            if self.uModel != -1:
                glUniformMatrix4fv(self.uModel, 1, GL_FALSE, modelM.T)
            # draw
            glBindVertexArray(self.vao)
            if self.model.mesh.vertices.size and self.model.mesh.indices.size:
                glDrawElements(GL_TRIANGLES, self.index_count, GL_UNSIGNED_INT, ctypes.c_void_p(0))
            else:
                # fallback: draw first N points so you see something
                glPointSize(4.0)
                glDrawArrays(GL_POINTS, 0, min(100, (self.model.mesh.vertices.size//(self.floats_per_vertex if self.floats_per_vertex>0 else 3))))
            glBindVertexArray(0)
            glfw.swap_buffers(self.window)
            glfw.poll_events()
            frame += 1
            if frame % 60 == 0:
                print("Frame:", frame)
        glfw.terminate()

# -------------------------
# entry
# -------------------------
def main():
    obj_file = "bunny.obj"
    print("Iniciando...")
    if not os.path.isfile(obj_file):
        print("Arquivo", obj_file, "não encontrado"); return
    model = load_obj(obj_file)
    print("Vertices (floats):", model.mesh.vertices.size)
    print("Indices:", model.mesh.indices.size)
    if model.mesh.vertices.size:
        print("Sample floats:", model.mesh.vertices[:32])
    if model.mesh.indices.size:
        print("Sample indices:", model.mesh.indices[:32])
    App(obj_file)

if __name__ == "__main__":
    main()
