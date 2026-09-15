# Nome: Rafael Manteiga Balbino 
# Matrícula: 201920649111

import numpy as np
import matplotlib.pyplot as plt

po = np.array([[1,1,1,1],[3,1,1,1],[3,3,1,1],[2,4,1,1],[1,3,1,1], # house: front points
               [1,1,4,1],[3,1,4,1],[3,3,4,1],[2,4,4,1],[1,3,4,1], # house: back points
               [1.7,1,1,1],[1.7,2.3,1,1],[2.3,2.3,1,1],[2.3,1,1,1]]) # house: door points

pFront = np.array([po[0],po[1],po[2],po[3],po[4],po[0]])
pBack = np.array([po[5],po[6],po[7],po[8],po[9],po[5]])
pCeil = np.array([po[4],po[2],po[7],po[9],po[4]])
pFloor = np.array([po[0],po[1],po[6],po[5],po[0]])
pRoof = np.array([po[3],po[8]])
pDoor = np.array([po[10],po[11],po[12],po[13]])

pFront = np.transpose(pFront)
pBack = np.transpose(pBack)
pCeil = np.transpose(pCeil)
pFloor = np.transpose(pFloor)
pRoof = np.transpose(pRoof)
pDoor = np.transpose(pDoor)

end_loop = False
projection_plane = 'XY'
perspective_x = 0.
perspective_y = 0.
perspective_z = 0.
scale_object = 1.
theta_y = 0
theta_x = 0

def on_key(event):
    global end_loop
    global projection_plane
    global perspective_x
    global perspective_y
    global perspective_z
    global scale_object
    global points
    global theta_x
    global theta_y

    if event.key == 'escape': 
        end_loop = True
    elif event.key == '0': 
        print('you pressed', event.key)
    elif event.key == '1': 
        print('you pressed', event.key)
        projection_plane = 'XY'
    elif event.key == '2': 
        print('you pressed', event.key)
        projection_plane = 'ZX'
    elif event.key == '3': 
        print('you pressed', event.key)
        projection_plane = 'ZY'
    elif event.key == 'x': 
        print('you pressed', event.key)
        perspective_x = perspective_x + 0.1
    elif event.key == 'X': 
        print('you pressed', event.key)
        perspective_x = perspective_x - 0.1
    elif event.key == 'y': 
        print('you pressed', event.key)
        perspective_y = perspective_y + 0.1
    elif event.key == 'Y': 
        print('you pressed', event.key)
        perspective_y = perspective_y - 0.1
    elif event.key == 'z': 
        print('you pressed', event.key)
        perspective_z = perspective_z + 0.1
    elif event.key == 'Z': 
        print('you pressed', event.key)
        perspective_z = perspective_z - 0.1
    elif event.key == 'up':
        print('you pressed', event.key)
        theta_x = theta_x + 5
    elif event.key == 'down':  
        print('you pressed', event.key)
        theta_x = theta_x - 5
    elif event.key == 'left':     
        print('you pressed', event.key)
        theta_y = theta_y + 5
    elif event.key == 'right': 
        print('you pressed', event.key)
        theta_y = theta_y - 5
        
def on_press(event):
    global scale_object
    if event.button==1: #pressed LEFT button
        print('you pressed left mouse button', event.xdata, event.ydata)
        scale_object = scale_object * 0.5
        if scale_object < 0.1:
            scale_object = 0.1
    elif event.button==3: #pressed RIGHT button
        print('you pressed right mouse button', event.xdata, event.ydata)
        scale_object = scale_object * 1.5
            
def anima(pBack,pFront,pCeil,pFloor,pRoof,pDoor):

    if not np.array_equal(pDoor[:, -1], pDoor[:, 0]):
        pDoor = np.hstack([pDoor, pDoor[:, 0:1]])

    # find house center
    todos = np.hstack([pBack,pFront,pCeil,pFloor,pRoof,pDoor])
    min_x = np.min(todos[0, :])
    max_x = np.max(todos[0, :])
    min_y = np.min(todos[1, :])
    max_y = np.max(todos[1, :])
    min_z = np.min(todos[2, :])
    max_z = np.max(todos[2, :])
    centro_x = (min_x + max_x) / 2.0
    centro_y = (min_y + max_y) / 2.0
    centro_z = (min_z + max_z) / 2.0

    # transformation to translate center of house to origin
    T = np.eye(4)
    T[0, 3] = -centro_x
    T[1, 3] = -centro_y
    T[2, 3] = -centro_z
    
    # closes all existing figures
    plt.close('all')    
    fig, ax = plt.subplots()
   
    # register callback functions
    cid = fig.canvas.mpl_connect('key_press_event', on_key)
    cid = fig.canvas.mpl_connect('button_press_event', on_press)
    
    partes = [pBack, pFront, pCeil, pFloor, pRoof, pDoor]
    
    while not end_loop:
        ax.clear()
        
        # assemble transformation matrixes
        rad_x = np.radians(theta_x)
        rad_y = np.radians(theta_y)

        Rx = np.array([[1, 0, 0, 0],
                       [0, np.cos(rad_x), -np.sin(rad_x), 0],
                       [0, np.sin(rad_x),  np.cos(rad_x), 0],
                       [0, 0, 0, 1]])

        Ry = np.array([[ np.cos(rad_y), 0, np.sin(rad_y), 0],
                       [ 0,           1, 0,           0],
                       [-np.sin(rad_y), 0, np.cos(rad_y), 0],
                       [ 0,           0, 0,           1]])

        S = np.diag([scale_object, scale_object, scale_object, 1.0])

        P = np.eye(4)
        P[3, 0] = perspective_x
        P[3, 1] = perspective_y
        P[3, 2] = perspective_z

        M = P @ S @ Ry @ Rx @ T
        
        # apply transformations to house points
        for parte in partes:
            q = M @ parte
            w = q[3, :]
            w = np.where(np.abs(w) < 1e-12, 1e-12, w)
            x = q[0, :] / w
            y = q[1, :] / w
            z = q[2, :] / w

            # plot transformed house points
            if projection_plane == 'XY':
                u = x
                v = y
            elif projection_plane == 'ZX':
                u = z
                v = x
            else:
                u = z
                v = y

            ax.plot(u, v, 'r-', linewidth=1.5)

        ax.set_aspect('equal', adjustable='box')
        ax.set_xlim(-5, 5)
        ax.set_ylim(-5, 5)
        ax.grid(True, linestyle='--', alpha=0.3)
        ax.set_title(f"Projection Plane: {projection_plane}\n"
                     f"Rotation: X = {theta_x:.0f}; Y = {theta_y:.0f}\n"
                     f"Perspective: X = {perspective_x:.1f}; Y = {perspective_y:.1f}; Z = {perspective_z:.1f}")
        
        plt.draw()
        plt.pause(0.01)

# execute animation
anima(pBack,pFront,pCeil,pFloor,pRoof,pDoor)
plt.close('all')    # Closes all existing figures
