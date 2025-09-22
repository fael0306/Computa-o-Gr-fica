from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys

glutInit(sys.argv)

glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)

glutInitWindowSize(600, 600)

glutCreateWindow("Cubo Rubik Ortogonal".encode('utf-8'))

glClearColor(0.0, 0.0, 0.0, 1.0)

glEnable(GL_DEPTH_TEST)

glMatrixMode(GL_PROJECTION)
glLoadIdentity()
glOrtho(-1.5, 1.5, -1.5, 1.5, -10.0, 10.0)

glMatrixMode(GL_MODELVIEW)

def desenhar():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()

    glRotatef(30, 1, 1, 0)

    glBegin(GL_QUADS)

    # Face de cima - branca
    glColor3f(1.0, 1.0, 1.0)
    glVertex3f(-0.5, 0.5, -0.5)
    glVertex3f(0.5, 0.5, -0.5)
    glVertex3f(0.5, 0.5, 0.5)
    glVertex3f(-0.5, 0.5, 0.5)

    # Face de baixo - amarela
    glColor3f(1.0, 1.0, 0.0)
    glVertex3f(-0.5, -0.5, -0.5)
    glVertex3f(0.5, -0.5, -0.5)
    glVertex3f(0.5, -0.5, 0.5)
    glVertex3f(-0.5, -0.5, 0.5)

    # Face da frente - vermelha
    glColor3f(1.0, 0.0, 0.0)
    glVertex3f(-0.5, -0.5, 0.5)
    glVertex3f(0.5, -0.5, 0.5)
    glVertex3f(0.5, 0.5, 0.5)
    glVertex3f(-0.5, 0.5, 0.5)

    # Face de trás - laranja
    glColor3f(1.0, 0.5, 0.0)
    glVertex3f(-0.5, -0.5, -0.5)
    glVertex3f(0.5, -0.5, -0.5)
    glVertex3f(0.5, 0.5, -0.5)
    glVertex3f(-0.5, 0.5, -0.5)

    # Direita - azul
    glColor3f(0.0, 0.0, 1.0)
    glVertex3f(0.5, -0.5, -0.5)
    glVertex3f(0.5, -0.5, 0.5)
    glVertex3f(0.5, 0.5, 0.5)
    glVertex3f(0.5, 0.5, -0.5)

    # Face da esquerda - verde
    glColor3f(0.0, 1.0, 0.0)
    glVertex3f(-0.5, -0.5, -0.5)
    glVertex3f(-0.5, -0.5, 0.5)
    glVertex3f(-0.5, 0.5, 0.5)
    glVertex3f(-0.5, 0.5, -0.5)

    glEnd()

    # Atualiza a tela
    glutSwapBuffers()

glutDisplayFunc(desenhar)

glutMainLoop()
