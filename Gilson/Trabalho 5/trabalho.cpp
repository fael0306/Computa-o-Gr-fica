#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
    #include <windows.h>
#endif
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "RgbImage.h"

using namespace std;

const char* arquivo_textura = "./metalTexture1.bmp";

GLuint textura_id;
GLUquadricObj *esfera;
GLUquadricObj *cilindro;

bool ligar_textura = true;
float andar = 0.0;
float angulo_cabeca = 0.0;
float angulo_antena_esq = 0.0;
float angulo_antena_dir = 0.0;

float distancia_olho = 15.0;
float olhoX, olhoY, olhoZ;
float angulo_visaoX = 0.0;
float angulo_visaoZ = 15.0;

// offsets de pan (movem o alvo da câmera)
float panX = 0.0;
float panY = 0.0;

GLuint carregarTextura(const char *nome) {
    GLuint id;
    RgbImage imagem(nome);

    if (imagem.GetNumRows() == 0) {
        printf("ERRO: Nao achei a textura %s. Vou desenhar sem textura.\n", nome);
        return 0;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagem.GetNumCols(), imagem.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, imagem.ImageData());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

void initRendering() {
    esfera = gluNewQuadric();
    cilindro = gluNewQuadric();
    textura_id = carregarTextura(arquivo_textura);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    // Permite que glColor3f altere as propriedades de material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat luz_ambiente[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat luz_difusa[]   = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat posicao_luz[]  = { 10.0f, 10.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, luz_ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luz_difusa);
    glLightfv(GL_LIGHT0, GL_POSITION, posicao_luz);
}

void desenhaChao() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-10.0f, -10.0f, 0.0f);
        glVertex3f( 10.0f, -10.0f, 0.0f);
        glVertex3f( 10.0f,  10.0f, 0.0f);
        glVertex3f(-10.0f,  10.0f, 0.0f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void desenhaPerna(float posX, float posY, float angulo) {
    glPushMatrix();
        glTranslatef(posX, posY, 1.4);

        // Aponta a perna para BAIXO e um pouco para FORA
        // (giro em X: -160 graus aponta para +Y e -Z; +160 aponta para -Y e -Z)
        if (posY > 0) glRotatef(-160, 1, 0, 0);
        else          glRotatef( 160, 1, 0, 0);

        // Balanço da caminhada (em torno de Y)
        glRotatef(angulo, 0, 1, 0);

        if (ligar_textura && textura_id != 0) {
            glBindTexture(GL_TEXTURE_2D, textura_id);
            gluQuadricTexture(cilindro, 1);
        } else {
            gluQuadricTexture(cilindro, 0);
            glColor3f(0.3f, 0.3f, 0.3f);
        }
        gluCylinder(cilindro, 0.1, 0.05, 1.5, 10, 10);

        glTranslatef(0, 0, 1.5);
        gluSphere(esfera, 0.1, 10, 10);

    glPopMatrix();
}

void drawScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    olhoX = distancia_olho * cos(angulo_visaoZ*3.1415/180) * cos(angulo_visaoX*3.1415/180);
    olhoY = distancia_olho * cos(angulo_visaoZ*3.1415/180) * sin(angulo_visaoX*3.1415/180);
    olhoZ = distancia_olho * sin(angulo_visaoZ*3.1415/180);

    if (angulo_visaoZ < 90)
        gluLookAt(olhoX, olhoY, olhoZ, panX, panY, 0.0, 0.0, 0.0,  1.0);
    else
        gluLookAt(olhoX, olhoY, olhoZ, panX, panY, 0.0, 0.0, 0.0, -1.0);

    desenhaChao();

    // ----- ABDOMEN (atras) -----
    glPushMatrix();
        glTranslatef(-1.5, 0, 1.4);
        glScalef(1.5, 1.0, 1.0);
        glColor3f(1.0f, 1.0f, 1.0f);
        if (ligar_textura && textura_id != 0) gluQuadricTexture(esfera, 1);
        else                                  gluQuadricTexture(esfera, 0);
        gluSphere(esfera, 0.6, 20, 20);
    glPopMatrix();

    // ----- TORAX (meio, mais alto) -----
    glPushMatrix();
        glTranslatef(0, 0, 1.5);
        glColor3f(1.0f, 1.0f, 1.0f);
        if (ligar_textura && textura_id != 0) gluQuadricTexture(esfera, 1);
        else                                  gluQuadricTexture(esfera, 0);
        gluSphere(esfera, 0.5, 20, 20);
    glPopMatrix();

    // ----- PERNAS (3 de cada lado, marcha alternada) -----
    desenhaPerna( 0.3,  0.4, sin(andar)        * 30);
    desenhaPerna( 0.0,  0.4, sin(andar + 3.14) * 30);
    desenhaPerna(-0.3,  0.4, sin(andar)        * 30);

    desenhaPerna( 0.3, -0.4, sin(andar + 3.14) * 30);
    desenhaPerna( 0.0, -0.4, sin(andar)        * 30);
    desenhaPerna(-0.3, -0.4, sin(andar + 3.14) * 30);

    // ----- CABECA (frente) -----
    glPushMatrix();
        glTranslatef(1.0, 0, 1.4);
        glRotatef(angulo_cabeca, 0, 1, 0);

        glColor3f(1.0f, 1.0f, 1.0f);
        if (ligar_textura && textura_id != 0) gluQuadricTexture(esfera, 1);
        else                                  gluQuadricTexture(esfera, 0);
        gluSphere(esfera, 0.4, 20, 20);

        // Olhos (preto, sem textura)
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
            glTranslatef(0.3, 0.2, 0.1);
            gluSphere(esfera, 0.1, 10, 10);
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.3, -0.2, 0.1);
            gluSphere(esfera, 0.1, 10, 10);
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);

        // Antenas
        glColor3f(1.0f, 1.0f, 1.0f);
        if (ligar_textura && textura_id != 0) gluQuadricTexture(cilindro, 1);
        else                                  gluQuadricTexture(cilindro, 0);

        glPushMatrix();
            glTranslatef(0.2, 0.15, 0.3);
            glRotatef(angulo_antena_esq, 0, 1, 0);
            glRotatef(30, 1, 0, 0);
            gluCylinder(cilindro, 0.02, 0.01, 1.0, 10, 10);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(0.2, -0.15, 0.3);
            glRotatef(angulo_antena_dir, 0, 1, 0);
            glRotatef(-30, 1, 0, 0);
            gluCylinder(cilindro, 0.02, 0.01, 1.0, 10, 10);
        glPopMatrix();

    glPopMatrix();

    glutSwapBuffers();
}

void atualiza(int valor) {
    andar += 0.15;
    if (andar > 6.28) andar = 0.0;

    angulo_antena_esq = 20.0 * sin(andar * 2);
    angulo_antena_dir = 20.0 * sin(andar * 2 + 3.14);

    glutPostRedisplay();
    glutTimerFunc(33, atualiza, 0);
}

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
    case 27: exit(0);
    case 'w': angulo_visaoZ += 3; break;
    case 's': angulo_visaoZ -= 3; break;
    case 'a': angulo_visaoX -= 3; break;
    case 'd': angulo_visaoX += 3; break;
    case '+': distancia_olho -= 1.0; break;
    case '-': distancia_olho += 1.0; break;
    case 't': ligar_textura = !ligar_textura; break;
    case '1': angulo_cabeca += 5; break;
    case '2': angulo_cabeca -= 5; break;
    case '3': angulo_antena_esq += 5; break;
    case '4': angulo_antena_esq -= 5; break;
    case '5': angulo_antena_dir += 5; break;
    case '6': angulo_antena_dir -= 5; break;
    }
    glutPostRedisplay();
}

// ---- Pan com as setas do teclado (requisito do enunciado) ----
void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:  panX -= 0.3f; break;
    case GLUT_KEY_RIGHT: panX += 0.3f; break;
    case GLUT_KEY_UP:    panY += 0.3f; break;
    case GLUT_KEY_DOWN:  panY -= 0.3f; break;
    }
    glutPostRedisplay();
}

void handleResize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 1.0, 50.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Trabalho de CG - Formiga Robo");

    initRendering();
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeypress);   // <-- pan
    glutReshapeFunc(handleResize);
    glutTimerFunc(33, atualiza, 0);

    glutMainLoop();
    return 0;
}
