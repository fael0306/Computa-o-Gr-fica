import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import math
import random

pygame.init()
pygame.display.set_mode((600, 600), DOUBLEBUF | OPENGL)
gluOrtho2D(-1, 1, -1, 1)
relogio = pygame.time.Clock()

distancia_terra = 0.55
distancia_lua = 0.25
giro_terra = 0
giro_lua = 0

rodando = True
while rodando:
    for evento in pygame.event.get():
        if evento.type == QUIT:
            rodando = False

    glClear(GL_COLOR_BUFFER_BIT)

    glColor3f(1, 1, 0)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(0, 0)
    for i in range(0, 361, 5):
        angulo = math.radians(i)
        x = math.cos(angulo) * 0.11
        y = math.sin(angulo) * 0.095
        x += random.uniform(-0.002, 0.002)
        y += random.uniform(-0.002, 0.002)
        glVertex2f(x, y)
    glEnd()

    terra_x = distancia_terra * math.cos(math.radians(giro_terra)) + random.uniform(-0.005, 0.005)
    terra_y = distancia_terra * math.sin(math.radians(giro_terra)) + random.uniform(-0.005, 0.005)

    glPushMatrix()
    glTranslatef(terra_x, terra_y, 0)
    glColor3f(0, 0, 1)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(0, 0)
    for i in range(0, 361, 5):
        angulo = math.radians(i)
        x = math.cos(angulo) * 0.05
        y = math.sin(angulo) * 0.05
        glVertex2f(x, y)
    glEnd()

    lua_x = distancia_lua * math.cos(math.radians(giro_lua)) + random.uniform(-0.007, 0.007)
    lua_y = distancia_lua * math.sin(math.radians(giro_lua)) + random.uniform(-0.007, 0.007)

    glPushMatrix()
    glTranslatef(lua_x, lua_y, 0)
    glColor3f(0.6, 0.6, 0.6)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(0, 0)
    for i in range(0, 361, 5):
        angulo = math.radians(i)
        x = math.cos(angulo) * 0.02
        y = math.sin(angulo) * 0.02
        glVertex2f(x, y)
    glEnd()
    glPopMatrix()

    glPopMatrix()

    pygame.display.flip()
    relogio.tick(60)

    giro_terra = (giro_terra + 0.5) % 360
    giro_lua = (giro_lua + 2.0) % 360

pygame.quit()
