import math
import pygame
from pygame.locals import DOUBLEBUF, OPENGL, QUIT, VIDEORESIZE
from OpenGL.GL import *

# parâmetros do sol
R_DISCO = 50   # raio do disco em pixels
R_RAIO = 100   # comprimento do raio em pixels
N_RAIOS = 12

# inicializa janela
pygame.init()
win_w, win_h = 800, 600
screen = pygame.display.set_mode((win_w, win_h), DOUBLEBUF | OPENGL | pygame.RESIZABLE)
pygame.display.set_caption("Sol iniciante - canto inferior esquerdo")

def draw(win_w, win_h):
    glClearColor(0, 0, 0, 1)
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()

    # projeção ortográfica em pixels
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0, win_w, 0, win_h, -1, 1)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    # posição do sol no canto inferior esquerdo
    cx, cy = R_RAIO + 10, R_RAIO + 10  # deslocamento para não cortar

    # círculo central
    glColor3f(1, 1, 0)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(cx, cy)
    for i in range(51):
        ang = 2 * math.pi * i / 50
        x = cx + R_DISCO * math.cos(ang)
        y = cy + R_DISCO * math.sin(ang)
        glVertex2f(x, y)
    glEnd()

    # raios triangulares
    for i in range(N_RAIOS):
        ang = 2 * math.pi * i / N_RAIOS
        tip_x = cx + R_RAIO * math.cos(ang)
        tip_y = cy + R_RAIO * math.sin(ang)
        base1_x = cx + R_DISCO * math.cos(ang) - 0.02*R_RAIO * math.sin(ang)
        base1_y = cy + R_DISCO * math.sin(ang) + 0.02*R_RAIO * math.cos(ang)
        base2_x = cx + R_DISCO * math.cos(ang) + 0.02*R_RAIO * math.sin(ang)
        base2_y = cy + R_DISCO * math.sin(ang) - 0.02*R_RAIO * math.cos(ang)
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
        elif event.type == VIDEORESIZE:
            win_w, win_h = event.w, event.h
            screen = pygame.display.set_mode((win_w, win_h), DOUBLEBUF | OPENGL | pygame.RESIZABLE)

    draw(win_w, win_h)

pygame.quit()
