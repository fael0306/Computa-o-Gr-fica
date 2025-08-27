import math
import pygame
from pygame.locals import DOUBLEBUF, OPENGL, QUIT
from OpenGL.GL import *

pygame.init()
screen = pygame.display.set_mode((800, 600), DOUBLEBUF | OPENGL)

glClearColor(0, 0, 0, 1)
glClear(GL_COLOR_BUFFER_BIT)
glLoadIdentity()

glMatrixMode(GL_PROJECTION)
glLoadIdentity()
glOrtho(-1, 1, -1, 1, -1, 1)
glMatrixMode(GL_MODELVIEW)
glLoadIdentity()

glColor3f(1, 1, 0)
glBegin(GL_TRIANGLE_FAN)
glVertex2f(0, 0)
for i in range(51):
    ang = 2 * math.pi * i / 50
    x = 0.3 * math.cos(ang)
    y = 0.3 * math.sin(ang)
    glVertex2f(x, y)
glEnd()

N_RAIOS = 12
for i in range(N_RAIOS):
    ang = 2 * math.pi * i / N_RAIOS
    tip_x = 0.6 * math.cos(ang)
    tip_y = 0.6 * math.sin(ang)
    base1_x = 0.3 * math.cos(ang) + 0.05 * math.sin(ang)
    base1_y = 0.3 * math.sin(ang) - 0.05 * math.cos(ang)
    base2_x = 0.3 * math.cos(ang) - 0.05 * math.sin(ang)
    base2_y = 0.3 * math.sin(ang) + 0.05 * math.cos(ang)
    glBegin(GL_TRIANGLES)
    glVertex2f(tip_x, tip_y)
    glVertex2f(base1_x, base1_y)
    glVertex2f(base2_x, base2_y)
    glEnd()

pygame.display.flip()

running = True
while running:
    for event in pygame.event.get():
        if event.type == QUIT:
            running = False

pygame.quit()
