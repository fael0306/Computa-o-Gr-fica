#define _CRT_SECURE_NO_WARNINGS
#define PI 3.141592654

#include <windows.h>
#include <stdlib.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <GL/glut.h>
#include "RgbImage.h"

using namespace std;


// Cena

const float GROUND_Z = -3.58f;


// Texturas

const char* filenameTexMetal =
"./metalTexture1.bmp";

const char* filenameTexShell =
"./antShellTexture.bmp";

GLuint textureMetal;
GLuint textureShell;
GLuint currentTexture;


bool textureOn = true;
bool lightingOn = true;

GLUquadric* quadSphere;
GLUquadric* quadCylinder;


// Camera

int windowWidth = 800;
int windowHeight = 800;

float eyeDistance = 12.0f;

float eyeX;
float eyeY;
float eyeZ;

float viewAngleX = 0.0f;
float viewAngleZ = 15.0f;

float cameraFov = 60.0f;

float cameraTargetX = 0.0f;
float cameraTargetY = 0.0f;
float cameraTargetZ = 0.0f;


// Formiga

float robotYaw = 0.0f;

float headAngle = 0.0f;
float walkHeadSway = 0.0f;

float bodyBob = 0.0f;
float bodyRoll = 0.0f;
float bodyYaw = 0.0f;


// Antenas

float antennaLeftBaseAngle = 25.0f;
float antennaLeftTipAngle = 20.0f;

float antennaRightBaseAngle = 25.0f;
float antennaRightTipAngle = 20.0f;


// Pernas

enum LegIndex {
    FRONT_LEFT = 0,
    MIDDLE_LEFT,
    REAR_LEFT,

    FRONT_RIGHT,
    MIDDLE_RIGHT,
    REAR_RIGHT,

    LEG_COUNT
};

float legHipAngle[LEG_COUNT] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f
};

float legKneeAngle[LEG_COUNT] = {
    -35.0f, -35.0f, -35.0f,
    -35.0f, -35.0f, -35.0f
};

float legAnkleAngle[LEG_COUNT] = {
    20.0f, 20.0f, 20.0f,
    20.0f, 20.0f, 20.0f
};

int selectedLeg = FRONT_LEFT;


// Caminhada

bool walking = false;
float walkPhase = 0.0f;


// Camera scanner

float scannerAngle = 0.0f;
float scannerDirection = 1.0f;
float scannerPulsePhase = 0.0f;
float scannerPulse = 0.20f;

struct Vec3 {
    float x;
    float y;
    float z;
};

Vec3 scannerWorldPos = { 0.0f, 0.0f, 0.0f };
Vec3 scannerWorldDir = { 1.0f, 0.0f, -0.2f };
Vec3 scannerGroundHit = { 0.0f, 0.0f, GROUND_Z };
bool scannerGroundHitValid = false;


bool fileExists(const char* filename) {
    FILE* file = fopen(
        filename,
        "rb"
    );

    if (file != NULL) {
        fclose(file);
        return true;
    }

    return false;
}



GLuint loadTexture(const char* filename) {
    RgbImage image(filename);

    GLuint textureId;

    glGenTextures(
        1,
        &textureId
    );

    glBindTexture(
        GL_TEXTURE_2D,
        textureId
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        image.GetNumCols(),
        image.GetNumRows(),
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        image.ImageData()
    );

    return textureId;
}



void setTextureScale(
    float scaleS,
    float scaleT
) {
    glMatrixMode(GL_TEXTURE);

    glLoadIdentity();

    glScalef(
        scaleS,
        scaleT,
        1.0f
    );

    glMatrixMode(GL_MODELVIEW);
}



void useMetalTexture(
    float scaleS = 2.0f,
    float scaleT = 2.0f
) {
    currentTexture =
        textureMetal;

    setTextureScale(
        scaleS,
        scaleT
    );
}



void useShellTexture(
    float scaleS = 2.0f,
    float scaleT = 2.0f
) {
    currentTexture =
        textureShell;

    setTextureScale(
        scaleS,
        scaleT
    );
}



void updateProjection(void) {
    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();

    float aspect =
        static_cast<float>(windowWidth) /
        static_cast<float>(windowHeight);

    gluPerspective(
        cameraFov,
        aspect,
        1.0,
        50.0
    );

    glMatrixMode(
        GL_MODELVIEW
    );
}



void initLighting(void) {
    GLfloat globalAmbient[] = {
        0.07f,
        0.07f,
        0.09f,
        1.0f
    };

    GLfloat lightAmbient[] = {
        0.09f,
        0.09f,
        0.11f,
        1.0f
    };

    GLfloat lightDiffuse[] = {
        0.94f,
        0.90f,
        0.84f,
        1.0f
    };

    GLfloat lightSpecular[] = {
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    glLightModelfv(
        GL_LIGHT_MODEL_AMBIENT,
        globalAmbient
    );

    glLightfv(
        GL_LIGHT0,
        GL_AMBIENT,
        lightAmbient
    );

    glLightfv(
        GL_LIGHT0,
        GL_DIFFUSE,
        lightDiffuse
    );

    glLightfv(
        GL_LIGHT0,
        GL_SPECULAR,
        lightSpecular
    );

    glColorMaterial(
        GL_FRONT_AND_BACK,
        GL_AMBIENT_AND_DIFFUSE
    );

    glEnable(
        GL_COLOR_MATERIAL
    );

    glEnable(
        GL_LIGHTING
    );

    glEnable(
        GL_LIGHT0
    );

    glShadeModel(
        GL_SMOOTH
    );
}



void initRendering(void) {
    glEnable(
        GL_DEPTH_TEST
    );

    glDepthFunc(
        GL_LESS
    );

    glEnable(
        GL_NORMALIZE
    );

    glClearColor(
        0.015f,
        0.018f,
        0.027f,
        1.0f
    );

    quadSphere =
        gluNewQuadric();

    quadCylinder =
        gluNewQuadric();

    gluQuadricNormals(
        quadSphere,
        GLU_SMOOTH
    );

    gluQuadricNormals(
        quadCylinder,
        GLU_SMOOTH
    );

    textureMetal =
        loadTexture(
            filenameTexMetal
        );

    if (
        fileExists(
            filenameTexShell
        )
        ) {
        textureShell =
            loadTexture(
                filenameTexShell
            );

    }
    else {
        textureShell =
            textureMetal;
    }

    currentTexture =
        textureMetal;

    glTexEnvf(
        GL_TEXTURE_ENV,
        GL_TEXTURE_ENV_MODE,
        GL_MODULATE
    );

    initLighting();
}



void applyMaterial(
    float r,
    float g,
    float b,

    float specR,
    float specG,
    float specB,

    float shininess,

    float emissionR,
    float emissionG,
    float emissionB
) {
    GLfloat specular[] = {
        specR,
        specG,
        specB,
        1.0f
    };

    GLfloat emission[] = {
        emissionR,
        emissionG,
        emissionB,
        1.0f
    };

    glMaterialfv(
        GL_FRONT_AND_BACK,
        GL_SPECULAR,
        specular
    );

    glMaterialf(
        GL_FRONT_AND_BACK,
        GL_SHININESS,
        shininess
    );

    glMaterialfv(
        GL_FRONT_AND_BACK,
        GL_EMISSION,
        emission
    );

    glColor3f(
        r,
        g,
        b
    );
}



void setCarapaceMaterial(void) {
    applyMaterial(
        0.26f,
        0.28f,
        0.31f,

        0.90f,
        0.92f,
        0.96f,

        82.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useShellTexture(
        2.5f,
        1.8f
    );
}



void setDarkMetalMaterial(void) {
    applyMaterial(
        0.12f,
        0.14f,
        0.17f,

        0.65f,
        0.68f,
        0.74f,

        52.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useMetalTexture(
        3.0f,
        2.0f
    );
}



void setJointMaterial(void) {
    applyMaterial(
        0.23f,
        0.25f,
        0.29f,

        0.94f,
        0.95f,
        1.0f,

        76.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useMetalTexture(
        3.8f,
        2.3f
    );
}



void setRedAccentMaterial(void) {
    applyMaterial(
        0.46f,
        0.025f,
        0.025f,

        1.0f,
        0.32f,
        0.32f,

        88.0f,

        0.015f,
        0.0f,
        0.0f
    );

    useMetalTexture(
        3.0f,
        3.0f
    );
}



void setEyeMaterial(void) {
    applyMaterial(
        0.025f,
        0.028f,
        0.034f,

        1.0f,
        1.0f,
        1.0f,

        115.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useShellTexture(
        4.5f,
        4.5f
    );
}



void setSensorMaterial(void) {
    applyMaterial(
        0.82f,
        0.018f,
        0.018f,

        1.0f,
        0.35f,
        0.35f,

        105.0f,

        0.20f,
        0.002f,
        0.002f
    );

    useMetalTexture(
        2.0f,
        2.0f
    );
}



void setGroundMaterial(void) {
    applyMaterial(
        0.085f,
        0.095f,
        0.12f,

        0.10f,
        0.10f,
        0.12f,

        6.0f,

        0.0f,
        0.0f,
        0.0f
    );
}


void setScannerHousingMaterial(void) {
    applyMaterial(
        0.88f,
        0.89f,
        0.91f,

        0.95f,
        0.95f,
        0.98f,

        88.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useMetalTexture(
        2.3f,
        2.3f
    );
}


void setScannerFaceMaterial(void) {
    applyMaterial(
        0.05f,
        0.06f,
        0.07f,

        0.95f,
        0.95f,
        0.98f,

        118.0f,

        0.0f,
        0.0f,
        0.0f
    );

    useMetalTexture(
        4.0f,
        4.0f
    );
}


void setScannerLensMaterial(void) {
    float pulseGlow =
        0.20f + 0.55f * scannerPulse;

    applyMaterial(
        0.55f,
        0.02f,
        0.02f,

        1.0f,
        0.35f,
        0.35f,

        120.0f,

        pulseGlow,
        0.01f,
        0.01f
    );

    useMetalTexture(
        2.0f,
        2.0f
    );
}


void drawScaledCube(
    float sizeX,
    float sizeY,
    float sizeZ
) {
    glPushMatrix();

    glScalef(
        sizeX,
        sizeY,
        sizeZ
    );

    glutSolidCube(
        1.0f
    );

    glPopMatrix();
}


void resetLegs(void) {
    for (
        int i = 0;
        i < LEG_COUNT;
        i++
        ) {
        legHipAngle[i] =
            0.0f;

        legKneeAngle[i] =
            -35.0f;

        legAnkleAngle[i] =
            20.0f;
    }
}



void resetWalkBody(void) {
    bodyBob =
        0.0f;

    bodyRoll =
        0.0f;

    bodyYaw =
        0.0f;

    walkHeadSway =
        0.0f;
}



void resetCamera(void) {
    eyeDistance =
        12.0f;

    viewAngleX =
        0.0f;

    viewAngleZ =
        15.0f;

    cameraFov =
        60.0f;

    cameraTargetX =
        0.0f;

    cameraTargetY =
        0.0f;

    cameraTargetZ =
        0.0f;

    updateProjection();

    glutPostRedisplay();
}



void setWalkingPose(
    int leg,
    float phase
) {
    float cycle =
        static_cast<float>(
            sin(phase)
            );

    float swing =
        0.0f;

    float stance =
        0.0f;

    if (cycle > 0.0f)
        swing = cycle;
    else
        stance = -cycle;

    float hipAmplitude =
        18.0f;

    if (
        leg == MIDDLE_LEFT ||
        leg == MIDDLE_RIGHT
        ) {
        hipAmplitude =
            15.0f;
    }

    legHipAngle[leg] =
        hipAmplitude *
        cycle;

    legKneeAngle[leg] =
        -35.0f
        + 26.0f * swing
        - 3.0f * stance;

    legAnkleAngle[leg] =
        20.0f
        - 18.0f * swing
        + 3.0f * stance;
}



Vec3 makeVec3(
    float x,
    float y,
    float z
) {
    Vec3 v = { x, y, z };
    return v;
}


Vec3 addVec3(
    Vec3 a,
    Vec3 b
) {
    return makeVec3(
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    );
}


Vec3 scaleVec3(
    Vec3 v,
    float s
) {
    return makeVec3(
        v.x * s,
        v.y * s,
        v.z * s
    );
}


float lengthVec3(Vec3 v) {
    return static_cast<float>(
        sqrt(
            v.x * v.x +
            v.y * v.y +
            v.z * v.z
        )
        );
}


Vec3 normalizeVec3(Vec3 v) {
    float len =
        lengthVec3(v);

    if (len < 0.0001f) {
        return makeVec3(
            1.0f,
            0.0f,
            0.0f
        );
    }

    return scaleVec3(
        v,
        1.0f / len
    );
}


Vec3 crossVec3(
    Vec3 a,
    Vec3 b
) {
    return makeVec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}


Vec3 rotateX(Vec3 v, float angleDeg) {
    float angle =
        angleDeg *
        static_cast<float>(PI) /
        180.0f;

    float c =
        static_cast<float>(cos(angle));

    float s =
        static_cast<float>(sin(angle));

    return makeVec3(
        v.x,
        v.y * c - v.z * s,
        v.y * s + v.z * c
    );
}


Vec3 rotateY(Vec3 v, float angleDeg) {
    float angle =
        angleDeg *
        static_cast<float>(PI) /
        180.0f;

    float c =
        static_cast<float>(cos(angle));

    float s =
        static_cast<float>(sin(angle));

    return makeVec3(
        v.x * c + v.z * s,
        v.y,
        -v.x * s + v.z * c
    );
}


Vec3 rotateZ(Vec3 v, float angleDeg) {
    float angle =
        angleDeg *
        static_cast<float>(PI) /
        180.0f;

    float c =
        static_cast<float>(cos(angle));

    float s =
        static_cast<float>(sin(angle));

    return makeVec3(
        v.x * c - v.y * s,
        v.x * s + v.y * c,
        v.z
    );
}


void computeScannerWorldState(void) {
    Vec3 localPos =
        makeVec3(
            0.42f,
            0.0f,
            0.0f
        );

    Vec3 localDir =
        makeVec3(
            1.0f,
            0.0f,
            0.0f
        );

    localPos =
        rotateY(
            localPos,
            -30.0f
        );

    localDir =
        rotateY(
            localDir,
            -30.0f
        );

    localPos =
        rotateZ(
            localPos,
            scannerAngle
        );

    localDir =
        rotateZ(
            localDir,
            scannerAngle
        );

    localPos =
        addVec3(
            localPos,
            makeVec3(
                0.50f,
                0.0f,
                0.69f
            )
        );

    localPos =
        addVec3(
            localPos,
            makeVec3(
                0.78f,
                0.0f,
                0.0f
            )
        );

    localPos =
        rotateZ(
            localPos,
            headAngle + walkHeadSway
        );

    localDir =
        rotateZ(
            localDir,
            headAngle + walkHeadSway
        );

    localPos =
        addVec3(
            localPos,
            makeVec3(
                1.42f,
                0.0f,
                0.12f
            )
        );

    localPos =
        rotateZ(
            localPos,
            bodyYaw
        );

    localDir =
        rotateZ(
            localDir,
            bodyYaw
        );

    localPos =
        rotateX(
            localPos,
            bodyRoll
        );

    localDir =
        rotateX(
            localDir,
            bodyRoll
        );

    localPos.z +=
        bodyBob;

    localPos =
        rotateZ(
            localPos,
            robotYaw
        );

    localDir =
        rotateZ(
            localDir,
            robotYaw
        );

    scannerWorldPos =
        localPos;

    scannerWorldDir =
        normalizeVec3(localDir);

    scannerGroundHitValid =
        false;

    if (
        scannerWorldDir.z <
        -0.01f
        ) {
        float t =
            (GROUND_Z - scannerWorldPos.z) /
            scannerWorldDir.z;

        if (t > 0.0f) {
            scannerGroundHit =
                addVec3(
                    scannerWorldPos,
                    scaleVec3(
                        scannerWorldDir,
                        t
                    )
                );

            scannerGroundHitValid =
                true;
        }
    }
}


void updateScannerLight(void) {
    if (!lightingOn) {
        glDisable(
            GL_LIGHT1
        );
        return;
    }

    glEnable(
        GL_LIGHT1
    );

    float pulsePower =
        0.35f + 0.95f * scannerPulse;

    GLfloat scannerAmbient[] = {
        0.04f * scannerPulse,
        0.0f,
        0.0f,
        1.0f
    };

    GLfloat scannerDiffuse[] = {
        1.05f * pulsePower,
        0.06f * pulsePower,
        0.06f * pulsePower,
        1.0f
    };

    GLfloat scannerSpecular[] = {
        1.15f * pulsePower,
        0.18f * pulsePower,
        0.18f * pulsePower,
        1.0f
    };

    GLfloat scannerPosition[] = {
        scannerWorldPos.x,
        scannerWorldPos.y,
        scannerWorldPos.z,
        1.0f
    };

    GLfloat scannerDirectionVec[] = {
        scannerWorldDir.x,
        scannerWorldDir.y,
        scannerWorldDir.z
    };

    glLightfv(
        GL_LIGHT1,
        GL_AMBIENT,
        scannerAmbient
    );

    glLightfv(
        GL_LIGHT1,
        GL_DIFFUSE,
        scannerDiffuse
    );

    glLightfv(
        GL_LIGHT1,
        GL_SPECULAR,
        scannerSpecular
    );

    glLightfv(
        GL_LIGHT1,
        GL_POSITION,
        scannerPosition
    );

    glLightfv(
        GL_LIGHT1,
        GL_SPOT_DIRECTION,
        scannerDirectionVec
    );

    glLightf(
        GL_LIGHT1,
        GL_SPOT_CUTOFF,
        17.0f
    );

    glLightf(
        GL_LIGHT1,
        GL_SPOT_EXPONENT,
        42.0f
    );

    glLightf(
        GL_LIGHT1,
        GL_CONSTANT_ATTENUATION,
        0.6f
    );

    glLightf(
        GL_LIGHT1,
        GL_LINEAR_ATTENUATION,
        0.08f
    );

    glLightf(
        GL_LIGHT1,
        GL_QUADRATIC_ATTENUATION,
        0.02f
    );
}


void drawScannerBeamAndMarker(void) {
    if (!scannerGroundHitValid) {
        return;
    }

    bool restoreLighting =
        lightingOn;

    glDisable(
        GL_TEXTURE_2D
    );

    if (restoreLighting) {
        glDisable(
            GL_LIGHTING
        );
    }

    glEnable(
        GL_BLEND
    );

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    glDepthMask(
        GL_FALSE
    );

    Vec3 up =
        makeVec3(
            0.0f,
            0.0f,
            1.0f
        );

    Vec3 side =
        normalizeVec3(
            crossVec3(
                scannerWorldDir,
                up
            )
        );

    if (lengthVec3(side) < 0.001f) {
        side =
            makeVec3(
                0.0f,
                1.0f,
                0.0f
            );
    }

    float beamHalfWidthNear =
        0.015f;

    float beamHalfWidthFar =
        0.11f + 0.08f * scannerPulse;

    Vec3 nearLeft =
        addVec3(
            scannerWorldPos,
            scaleVec3(
                side,
                -beamHalfWidthNear
            )
        );

    Vec3 nearRight =
        addVec3(
            scannerWorldPos,
            scaleVec3(
                side,
                beamHalfWidthNear
            )
        );

    Vec3 farLeft =
        addVec3(
            scannerGroundHit,
            scaleVec3(
                side,
                -beamHalfWidthFar
            )
        );

    Vec3 farRight =
        addVec3(
            scannerGroundHit,
            scaleVec3(
                side,
                beamHalfWidthFar
            )
        );

    glBegin(
        GL_QUADS
    );

    glColor4f(
        1.0f,
        0.05f,
        0.05f,
        0.34f * scannerPulse
    );
    glVertex3f(nearLeft.x, nearLeft.y, nearLeft.z);
    glVertex3f(nearRight.x, nearRight.y, nearRight.z);

    glColor4f(
        1.0f,
        0.08f,
        0.08f,
        0.02f
    );
    glVertex3f(farRight.x, farRight.y, farRight.z + 0.02f);
    glVertex3f(farLeft.x, farLeft.y, farLeft.z + 0.02f);

    glEnd();

    glLineWidth(
        2.8f
    );

    glBegin(
        GL_LINES
    );
    glColor4f(
        1.0f,
        0.15f,
        0.15f,
        0.95f * scannerPulse
    );
    glVertex3f(
        scannerWorldPos.x,
        scannerWorldPos.y,
        scannerWorldPos.z
    );
    glColor4f(
        1.0f,
        0.10f,
        0.10f,
        0.18f
    );
    glVertex3f(
        scannerGroundHit.x,
        scannerGroundHit.y,
        scannerGroundHit.z + 0.015f
    );
    glEnd();

    float markerRX =
        0.26f + 0.05f * scannerPulse;

    float markerRY =
        0.16f + 0.05f * scannerPulse;

    glBegin(
        GL_TRIANGLE_FAN
    );
    glColor4f(
        0.95f,
        0.05f,
        0.05f,
        0.20f * scannerPulse
    );
    glVertex3f(
        scannerGroundHit.x,
        scannerGroundHit.y,
        GROUND_Z + 0.016f
    );

    for (int i = 0; i <= 40; i++) {
        float a =
            static_cast<float>(
                2.0 * PI * i / 40.0
                );

        float cx =
            scannerGroundHit.x +
            markerRX * static_cast<float>(cos(a));

        float cy =
            scannerGroundHit.y +
            markerRY * static_cast<float>(sin(a));

        glColor4f(
            1.0f,
            0.08f,
            0.08f,
            0.01f
        );

        glVertex3f(
            cx,
            cy,
            GROUND_Z + 0.016f
        );
    }
    glEnd();

    glLineWidth(
        2.0f
    );

    glColor4f(
        1.0f,
        0.16f,
        0.16f,
        0.95f * scannerPulse
    );

    glBegin(
        GL_LINE_LOOP
    );
    for (int i = 0; i < 40; i++) {
        float a =
            static_cast<float>(
                2.0 * PI * i / 40.0
                );

        float cx =
            scannerGroundHit.x +
            markerRX * static_cast<float>(cos(a));

        float cy =
            scannerGroundHit.y +
            markerRY * static_cast<float>(sin(a));

        glVertex3f(
            cx,
            cy,
            GROUND_Z + 0.018f
        );
    }
    glEnd();

    float scanOffset =
        markerRY * static_cast<float>(sin(scannerPulsePhase * 1.7f));

    glBegin(
        GL_LINES
    );
    glVertex3f(
        scannerGroundHit.x - markerRX * 0.92f,
        scannerGroundHit.y + scanOffset,
        GROUND_Z + 0.02f
    );
    glVertex3f(
        scannerGroundHit.x + markerRX * 0.92f,
        scannerGroundHit.y + scanOffset,
        GROUND_Z + 0.02f
    );
    glEnd();

    glDepthMask(
        GL_TRUE
    );

    glDisable(
        GL_BLEND
    );

    if (restoreLighting) {
        glEnable(
            GL_LIGHTING
        );
    }
}


void update(int value) {
    if (walking) {
        walkPhase +=
            0.10f;

        if (
            walkPhase >
            2.0f * PI
            ) {
            walkPhase -=
                static_cast<float>(
                    2.0 * PI
                    );
        }

        float phaseA =
            walkPhase;

        float phaseB =
            walkPhase +
            static_cast<float>(
                PI
                );


        setWalkingPose(
            FRONT_LEFT,
            phaseA
        );

        setWalkingPose(
            MIDDLE_RIGHT,
            phaseA
        );

        setWalkingPose(
            REAR_LEFT,
            phaseA
        );


        setWalkingPose(
            FRONT_RIGHT,
            phaseB
        );

        setWalkingPose(
            MIDDLE_LEFT,
            phaseB
        );

        setWalkingPose(
            REAR_RIGHT,
            phaseB
        );


        bodyBob =
            0.02f +
            0.02f *
            static_cast<float>(
                sin(
                    2.0f *
                    walkPhase
                )
                );

        bodyRoll =
            1.15f *
            static_cast<float>(
                sin(
                    walkPhase
                )
                );

        bodyYaw =
            0.85f *
            static_cast<float>(
                sin(
                    walkPhase +
                    PI / 2.0
                )
                );

        walkHeadSway =
            5.0f *
            static_cast<float>(
                sin(
                    walkPhase
                )
                );
    }
    else {
        bodyBob =
            0.0f;

        bodyRoll =
            0.0f;

        bodyYaw =
            0.0f;

        walkHeadSway =
            0.0f;
    }

    scannerAngle +=
        scannerDirection * 1.2f;

    if (scannerAngle >= 32.0f) {
        scannerAngle =
            32.0f;

        scannerDirection =
            -1.0f;
    }
    else if (scannerAngle <= -32.0f) {
        scannerAngle =
            -32.0f;

        scannerDirection =
            1.0f;
    }

    scannerPulsePhase +=
        0.26f;

    float pulseWave =
        static_cast<float>(
            sin(scannerPulsePhase)
            );

    float pulsePeak =
        pulseWave > 0.0f ? pulseWave : 0.0f;

    scannerPulse =
        0.16f +
        0.84f *
        static_cast<float>(
            pow(
                pulsePeak,
                6.0
            )
            );

    computeScannerWorldState();

    glutPostRedisplay();

    glutTimerFunc(
        25,
        update,
        0
    );
}



void handleKeypress(
    unsigned char key,
    int x,
    int y
) {
    switch (key) {

    case 27:
        exit(0);
        break;


    case 'w':
        if (
            viewAngleZ <
            180.0f
            ) {
            viewAngleZ +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case 'z':
        if (
            viewAngleZ >
            0.0f
            ) {
            viewAngleZ -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case 'a':
        viewAngleX -=
            3.0f;

        if (
            viewAngleX <
            0.0f
            ) {
            viewAngleX +=
                360.0f;
        }

        glutPostRedisplay();
        break;


    case 's':
        viewAngleX +=
            3.0f;

        if (
            viewAngleX >=
            360.0f
            ) {
            viewAngleX -=
                360.0f;
        }

        glutPostRedisplay();
        break;


    case 'g':
        robotYaw +=
            5.0f;

        if (
            robotYaw >=
            360.0f
            ) {
            robotYaw -=
                360.0f;
        }

        glutPostRedisplay();
        break;


    case 'd':
        robotYaw -=
            5.0f;

        if (
            robotYaw <
            0.0f
            ) {
            robotYaw +=
                360.0f;
        }

        glutPostRedisplay();
        break;


    case '+':
    case '=':
        if (
            cameraFov >
            25.0f
            ) {
            cameraFov -=
                3.0f;
        }

        updateProjection();
        glutPostRedisplay();
        break;


    case '-':
        if (
            cameraFov <
            90.0f
            ) {
            cameraFov +=
                3.0f;
        }

        updateProjection();
        glutPostRedisplay();
        break;


    case 'x':
        resetCamera();
        break;


    case 't':
        textureOn =
            !textureOn;

        glutPostRedisplay();
        break;


    case 'l':
        lightingOn =
            !lightingOn;

        if (lightingOn) {
            glEnable(
                GL_LIGHTING
            );

            glEnable(
                GL_LIGHT0
            );

            glEnable(
                GL_LIGHT1
            );
        }
        else {
            glDisable(
                GL_LIGHT1
            );

            glDisable(
                GL_LIGHTING
            );
        }

        glutPostRedisplay();
        break;


    case 'q':
        if (
            headAngle <
            35.0f
            ) {
            headAngle +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case 'e':
        if (
            headAngle >
            -35.0f
            ) {
            headAngle -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '1':
        if (
            antennaLeftBaseAngle <
            60.0f
            ) {
            antennaLeftBaseAngle +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '2':
        if (
            antennaLeftBaseAngle >
            -20.0f
            ) {
            antennaLeftBaseAngle -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '3':
        if (
            antennaLeftTipAngle <
            60.0f
            ) {
            antennaLeftTipAngle +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '4':
        if (
            antennaLeftTipAngle >
            -60.0f
            ) {
            antennaLeftTipAngle -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '5':
        if (
            antennaRightBaseAngle <
            60.0f
            ) {
            antennaRightBaseAngle +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '6':
        if (
            antennaRightBaseAngle >
            -20.0f
            ) {
            antennaRightBaseAngle -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '7':
        if (
            antennaRightTipAngle <
            60.0f
            ) {
            antennaRightTipAngle +=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case '8':
        if (
            antennaRightTipAngle >
            -60.0f
            ) {
            antennaRightTipAngle -=
                3.0f;
        }

        glutPostRedisplay();
        break;


    case 'p':
        walking =
            !walking;

        if (!walking) {
            resetLegs();
            resetWalkBody();
        }

        cout
            << "Caminhada: "
            << (
                walking
                ? "LIGADA"
                : "PAUSADA"
                )
            << endl;

        glutPostRedisplay();
        break;


    case '0':
        if (!walking) {
            resetLegs();
            resetWalkBody();

            glutPostRedisplay();
        }

        break;


    case 'c':
        selectedLeg++;

        if (
            selectedLeg >=
            LEG_COUNT
            ) {
            selectedLeg =
                0;
        }

        cout
            << "Perna selecionada: "
            << selectedLeg + 1
            << endl;

        break;


    case 'r':
        if (
            !walking &&
            legHipAngle[selectedLeg]
            < 35.0f
            ) {
            legHipAngle[selectedLeg]
                += 3.0f;
        }

        glutPostRedisplay();
        break;


    case 'f':
        if (
            !walking &&
            legHipAngle[selectedLeg]
            > -35.0f
            ) {
            legHipAngle[selectedLeg]
                -= 3.0f;
        }

        glutPostRedisplay();
        break;


    case 'y':
        if (
            !walking &&
            legKneeAngle[selectedLeg]
            < 20.0f
            ) {
            legKneeAngle[selectedLeg]
                += 3.0f;
        }

        glutPostRedisplay();
        break;


    case 'h':
        if (
            !walking &&
            legKneeAngle[selectedLeg]
            > -70.0f
            ) {
            legKneeAngle[selectedLeg]
                -= 3.0f;
        }

        glutPostRedisplay();
        break;


    case 'u':
        if (
            !walking &&
            legAnkleAngle[selectedLeg]
            < 60.0f
            ) {
            legAnkleAngle[selectedLeg]
                += 3.0f;
        }

        glutPostRedisplay();
        break;


    case 'j':
        if (
            !walking &&
            legAnkleAngle[selectedLeg]
            > -40.0f
            ) {
            legAnkleAngle[selectedLeg]
                -= 3.0f;
        }

        glutPostRedisplay();
        break;
    }
}



void handleSpecialKeypress(
    int key,
    int x,
    int y
) {
    const float panStep =
        0.30f;

    float angle =
        static_cast<float>(
            viewAngleX *
            PI /
            180.0
            );

    float rightX =
        -static_cast<float>(
            sin(angle)
            );

    float rightY =
        static_cast<float>(
            cos(angle)
            );


    switch (key) {

    case GLUT_KEY_LEFT:
        cameraTargetX -=
            rightX *
            panStep;

        cameraTargetY -=
            rightY *
            panStep;

        break;


    case GLUT_KEY_RIGHT:
        cameraTargetX +=
            rightX *
            panStep;

        cameraTargetY +=
            rightY *
            panStep;

        break;


    case GLUT_KEY_UP:
        cameraTargetZ +=
            panStep;

        break;


    case GLUT_KEY_DOWN:
        cameraTargetZ -=
            panStep;

        break;
    }


    glutPostRedisplay();
}



void handleResize(
    int w,
    int h
) {
    if (h == 0)
        h = 1;

    windowWidth =
        w;

    windowHeight =
        h;

    glViewport(
        0,
        0,
        w,
        h
    );

    updateProjection();
}



void drawSphere(float radius) {
    if (textureOn) {
        glEnable(
            GL_TEXTURE_2D
        );

        glBindTexture(
            GL_TEXTURE_2D,
            currentTexture
        );

        gluQuadricTexture(
            quadSphere,
            GLU_TRUE
        );
    }
    else {
        glDisable(
            GL_TEXTURE_2D
        );

        gluQuadricTexture(
            quadSphere,
            GLU_FALSE
        );
    }

    gluSphere(
        quadSphere,
        radius,
        40,
        40
    );
}



void drawTaperedCylinder(
    float baseRadius,
    float topRadius,
    float length
) {
    if (textureOn) {
        glEnable(
            GL_TEXTURE_2D
        );

        glBindTexture(
            GL_TEXTURE_2D,
            currentTexture
        );

        gluQuadricTexture(
            quadCylinder,
            GLU_TRUE
        );
    }
    else {
        glDisable(
            GL_TEXTURE_2D
        );

        gluQuadricTexture(
            quadCylinder,
            GLU_FALSE
        );
    }

    gluCylinder(
        quadCylinder,
        baseRadius,
        topRadius,
        length,
        36,
        20
    );
}



void drawCylinder(
    float radius,
    float length
) {
    drawTaperedCylinder(
        radius,
        radius,
        length
    );
}



void drawEllipsoid(
    float radius,
    float scaleX,
    float scaleY,
    float scaleZ
) {
    glPushMatrix();

    glScalef(
        scaleX,
        scaleY,
        scaleZ
    );

    drawSphere(
        radius
    );

    glPopMatrix();
}



void drawServoJoint(float radius) {
    setJointMaterial();

    drawSphere(
        radius
    );


    glPushMatrix();

    glTranslatef(
        0.0f,
        0.0f,
        -radius * 0.28f
    );

    setRedAccentMaterial();

    drawCylinder(
        radius * 0.72f,
        radius * 0.24f
    );

    glPopMatrix();
}



void drawLinearActuator(
    float length,
    float offset
) {
    glPushMatrix();

    glTranslatef(
        offset,
        0.0f,
        length * 0.13f
    );

    setRedAccentMaterial();

    drawTaperedCylinder(
        0.036f,
        0.026f,
        length * 0.66f
    );

    glPopMatrix();
}


void drawShockAbsorber(
    float length,
    float offset
) {
    glPushMatrix();

    glTranslatef(
        offset,
        0.0f,
        length * 0.18f
    );

    setDarkMetalMaterial();

    drawCylinder(
        0.046f,
        length * 0.34f
    );

    glTranslatef(
        0.0f,
        0.0f,
        length * 0.27f
    );

    setJointMaterial();

    drawCylinder(
        0.029f,
        length * 0.28f
    );

    glTranslatef(
        0.0f,
        0.0f,
        length * 0.24f
    );

    setRedAccentMaterial();

    drawSphere(
        0.042f
    );

    glPopMatrix();
}


void drawAntennaSensor(void) {
    setJointMaterial();

    drawSphere(
        0.064f
    );

    glPushMatrix();

    glTranslatef(
        0.0f,
        0.0f,
        0.045f
    );

    setSensorMaterial();

    drawEllipsoid(
        1.0f,
        0.050f,
        0.050f,
        0.075f
    );

    glPopMatrix();
}


void drawGround(void) {
    glDisable(
        GL_TEXTURE_2D
    );

    setGroundMaterial();


    glBegin(
        GL_QUADS
    );

    glNormal3f(
        0.0f,
        0.0f,
        1.0f
    );

    glVertex3f(
        -15.0f,
        -15.0f,
        GROUND_Z
    );

    glVertex3f(
        15.0f,
        -15.0f,
        GROUND_Z
    );

    glVertex3f(
        15.0f,
        15.0f,
        GROUND_Z
    );

    glVertex3f(
        -15.0f,
        15.0f,
        GROUND_Z
    );

    glEnd();


    bool restoreLighting =
        lightingOn;

    if (restoreLighting) {
        glDisable(
            GL_LIGHTING
        );
    }


    glColor3f(
        0.20f,
        0.22f,
        0.27f
    );

    glBegin(
        GL_LINES
    );

    for (
        int i = -10;
        i <= 10;
        i++
        ) {
        glVertex3f(
            static_cast<float>(i),
            -10.0f,
            GROUND_Z + 0.02f
        );

        glVertex3f(
            static_cast<float>(i),
            10.0f,
            GROUND_Z + 0.02f
        );

        glVertex3f(
            -10.0f,
            static_cast<float>(i),
            GROUND_Z + 0.02f
        );

        glVertex3f(
            10.0f,
            static_cast<float>(i),
            GROUND_Z + 0.02f
        );
    }

    glEnd();


    if (restoreLighting) {
        glEnable(
            GL_LIGHTING
        );
    }
}



void drawCoolingVents(void) {
    for (
        int i = 0;
        i < 4;
        i++
        ) {
        glPushMatrix();

        glTranslatef(
            -0.48f +
            0.25f * i,

            -0.24f,

            0.79f
        );

        glRotatef(
            -90.0f,
            1.0f,
            0.0f,
            0.0f
        );

        setDarkMetalMaterial();

        drawCylinder(
            0.032f,
            0.48f
        );

        glPopMatrix();
    }


    glPushMatrix();

    glTranslatef(
        0.48f,
        0.0f,
        0.84f
    );

    setSensorMaterial();

    drawSphere(
        0.065f
    );

    glPopMatrix();
}



void drawEnergyModule(void) {
    glPushMatrix();

    // Modulo dorsal central, recuado e separado da tampa de manutencao
    glTranslatef(
        -0.30f,
        0.0f,
        0.93f
    );

    setDarkMetalMaterial();

    drawEllipsoid(
        1.0f,
        0.30f,
        0.16f,
        0.050f
    );

    // Nucleo energetico vermelho
    glPushMatrix();

    glTranslatef(
        0.02f,
        0.0f,
        0.050f
    );

    setRedAccentMaterial();

    drawEllipsoid(
        1.0f,
        0.16f,
        0.075f,
        0.030f
    );

    glPopMatrix();

    // Indicadores de estado
    for (
        int i = 0;
        i < 3;
        i++
        ) {
        glPushMatrix();

        glTranslatef(
            -0.12f + 0.12f * i,
            0.0f,
            0.065f
        );

        setSensorMaterial();

        drawSphere(
            0.020f
        );

        glPopMatrix();
    }

    glPopMatrix();
}


void drawThorax(void) {
    // Pronoto

    glPushMatrix();

    glTranslatef(
        0.48f,
        0.0f,
        0.05f
    );

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        1.02f,
        0.98f,
        0.84f
    );

    glPopMatrix();


    // Mesotorax

    glPushMatrix();

    glTranslatef(
        -0.45f,
        0.0f,
        0.0f
    );

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        1.12f,
        0.92f,
        0.78f
    );

    glPopMatrix();


    // Tampa de manutencao mais baixa e integrada ao dorso

    glPushMatrix();

    glTranslatef(
        -0.05f,
        0.0f,
        0.66f
    );

    setDarkMetalMaterial();

    drawEllipsoid(
        1.0f,
        0.70f,
        0.48f,
        0.055f
    );

    glPopMatrix();


    drawCoolingVents();

    drawEnergyModule();
}



void drawEyeLens(
    float side,
    float x,
    float z
) {
    glPushMatrix();

    glTranslatef(
        x,
        1.055f * side,
        z
    );

    setSensorMaterial();

    drawSphere(
        0.052f
    );

    glPopMatrix();
}



void drawEye(float side) {
    glPushMatrix();

    glTranslatef(
        0.16f,
        0.93f * side,
        0.18f
    );

    setEyeMaterial();

    drawEllipsoid(
        1.0f,
        0.40f,
        0.12f,
        0.36f
    );

    glPopMatrix();


    drawEyeLens(
        side,
        -0.06f,
        0.30f
    );

    drawEyeLens(
        side,
        0.18f,
        0.34f
    );

    drawEyeLens(
        side,
        0.40f,
        0.20f
    );

    drawEyeLens(
        side,
        0.18f,
        0.05f
    );

    drawEyeLens(
        side,
        -0.06f,
        0.10f
    );
}



void drawMandible(float side) {
    glPushMatrix();

    glTranslatef(
        0.78f,
        0.34f * side,
        -0.22f
    );


    drawServoJoint(
        0.12f
    );


    glRotatef(
        17.0f * side,
        0.0f,
        0.0f,
        1.0f
    );


    glRotatef(
        102.0f,
        0.0f,
        1.0f,
        0.0f
    );


    setDarkMetalMaterial();

    drawTaperedCylinder(
        0.12f,
        0.075f,
        0.43f
    );


    glTranslatef(
        0.0f,
        0.0f,
        0.43f
    );


    glRotatef(
        18.0f * side,
        0.0f,
        1.0f,
        0.0f
    );


    setCarapaceMaterial();

    drawTaperedCylinder(
        0.075f,
        0.018f,
        0.32f
    );


    glPopMatrix();
}



void drawAntenna(
    float side,
    float baseAngle,
    float tipAngle
) {
    const float scapeLength =
        0.95f;

    const float tipSegmentLength =
        0.28f;


    glPushMatrix();


    glTranslatef(
        0.35f,
        0.46f * side,
        0.75f
    );


    drawServoJoint(
        0.13f
    );


    glRotatef(
        -18.0f * side,
        1.0f,
        0.0f,
        0.0f
    );


    glRotatef(
        baseAngle,
        0.0f,
        1.0f,
        0.0f
    );


    setCarapaceMaterial();

    drawTaperedCylinder(
        0.085f,
        0.065f,
        scapeLength
    );


    glTranslatef(
        0.0f,
        0.0f,
        scapeLength
    );


    drawServoJoint(
        0.095f
    );


    glRotatef(
        tipAngle,
        0.0f,
        1.0f,
        0.0f
    );


    float radius =
        0.060f;


    for (
        int i = 0;
        i < 3;
        i++
        ) {
        setDarkMetalMaterial();

        drawTaperedCylinder(
            radius,
            radius * 0.82f,
            tipSegmentLength
        );


        glTranslatef(
            0.0f,
            0.0f,
            tipSegmentLength
        );


        if (i < 2) {
            setJointMaterial();

            drawSphere(
                radius * 1.05f
            );


            glRotatef(
                5.0f,
                0.0f,
                1.0f,
                0.0f
            );
        }


        radius *=
            0.82f;
    }


    drawAntennaSensor();


    glPopMatrix();
}



void drawScannerCamera(void) {
    glPushMatrix();

    glTranslatef(
        0.44f,
        0.0f,
        0.58f
    );

    drawServoJoint(
        0.11f
    );

    glPushMatrix();
    glTranslatef(
        -0.04f,
        0.0f,
        0.10f
    );
    glRotatef(
        90.0f,
        0.0f,
        1.0f,
        0.0f
    );
    setJointMaterial();
    drawCylinder(
        0.045f,
        0.16f
    );
    glPopMatrix();

    glTranslatef(
        0.06f,
        0.0f,
        0.11f
    );

    glRotatef(
        scannerAngle,
        0.0f,
        0.0f,
        1.0f
    );

    drawServoJoint(
        0.09f
    );

    glRotatef(
        -30.0f,
        0.0f,
        1.0f,
        0.0f
    );

    glPushMatrix();
    glTranslatef(
        0.14f,
        0.0f,
        0.0f
    );
    setScannerHousingMaterial();
    drawScaledCube(
        0.44f,
        0.26f,
        0.24f
    );
    glPopMatrix();

    glPushMatrix();
    glTranslatef(
        0.00f,
        0.0f,
        0.0f
    );
    setScannerHousingMaterial();
    drawScaledCube(
        0.10f,
        0.14f,
        0.14f
    );
    glPopMatrix();

    glPushMatrix();
    glTranslatef(
        0.34f,
        0.0f,
        0.0f
    );
    setScannerFaceMaterial();
    drawScaledCube(
        0.08f,
        0.28f,
        0.28f
    );
    glPopMatrix();

    glPushMatrix();
    glTranslatef(
        0.38f,
        0.0f,
        0.0f
    );
    setScannerFaceMaterial();
    drawEllipsoid(
        1.0f,
        0.045f,
        0.125f,
        0.125f
    );
    glPopMatrix();

    glPushMatrix();
    glTranslatef(
        0.42f,
        0.0f,
        0.0f
    );
    setScannerLensMaterial();
    drawEllipsoid(
        1.0f,
        0.030f,
        0.085f,
        0.085f
    );
    glPopMatrix();

    for (int i = 0; i < 10; i++) {
        float a =
            static_cast<float>(
                2.0 * PI * i / 10.0
                );

        glPushMatrix();
        glTranslatef(
            0.385f,
            0.0f + 0.095f * static_cast<float>(cos(a)),
            0.0f + 0.095f * static_cast<float>(sin(a))
        );
        setJointMaterial();
        drawSphere(
            0.014f
        );
        glPopMatrix();
    }

    glPopMatrix();
}


void drawHead(void) {
    glPushMatrix();


    // Ponto real da articulacao cervical

    glTranslatef(
        1.42f,
        0.0f,
        0.12f
    );


    drawServoJoint(
        0.30f
    );


    glRotatef(
        headAngle +
        walkHeadSway,
        0.0f,
        0.0f,
        1.0f
    );


    glTranslatef(
        0.78f,
        0.0f,
        0.0f
    );


    // Capsula principal da cabeca

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        1.05f,
        1.00f,
        0.88f
    );


    // Regiao frontal / clipeo

    glPushMatrix();

    glTranslatef(
        0.72f,
        0.0f,
        -0.05f
    );

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        0.48f,
        0.72f,
        0.46f
    );

    glPopMatrix();


    drawEye(
        1.0f
    );

    drawEye(
        -1.0f
    );


    drawMandible(
        1.0f
    );

    drawMandible(
        -1.0f
    );


    drawAntenna(
        1.0f,
        antennaLeftBaseAngle,
        antennaLeftTipAngle
    );

    drawAntenna(
        -1.0f,
        antennaRightBaseAngle,
        antennaRightTipAngle
    );


    drawScannerCamera();


    glPopMatrix();
}



void drawPetiole(void) {
    // Pequeno eixo mecanico saindo do torax

    glPushMatrix();

    glTranslatef(
        -1.42f,
        0.0f,
        -0.02f
    );

    glRotatef(
        -90.0f,
        0.0f,
        1.0f,
        0.0f
    );

    setDarkMetalMaterial();

    drawCylinder(
        0.18f,
        0.34f
    );

    glPopMatrix();


    // No do petiolo

    glPushMatrix();

    glTranslatef(
        -1.66f,
        0.0f,
        -0.02f
    );

    drawServoJoint(
        0.27f
    );

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        0.43f,
        0.46f,
        0.54f
    );

    glPopMatrix();
}



void drawGasterBand(
    float x,
    float yRadius,
    float zRadius
) {
    const float width =
        0.075f;


    glPushMatrix();


    glTranslatef(
        x - width * 0.5f,
        0.0f,
        0.0f
    );


    glRotatef(
        90.0f,
        0.0f,
        1.0f,
        0.0f
    );


    glScalef(
        zRadius,
        yRadius,
        1.0f
    );


    setDarkMetalMaterial();


    drawCylinder(
        1.0f,
        width
    );


    glPopMatrix();
}



void drawAbdomen(void) {
    glPushMatrix();


    glTranslatef(
        -3.15f,
        0.0f,
        0.02f
    );


    // Gaster principal

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        1.75f,
        1.18f,
        1.08f
    );


    // Segmentacao inspirada nos tergitos reais

    drawGasterBand(
        0.42f,
        1.16f,
        1.06f
    );


    drawGasterBand(
        -0.55f,
        1.10f,
        1.00f
    );


    // Extremidade posterior mais afilada

    glPushMatrix();

    glTranslatef(
        -1.43f,
        0.0f,
        -0.03f
    );

    setCarapaceMaterial();

    drawEllipsoid(
        1.0f,
        0.62f,
        0.82f,
        0.72f
    );

    glPopMatrix();


    glPopMatrix();
}



void drawClaws(void) {
    for (
        int side = -1;
        side <= 1;
        side += 2
        ) {
        glPushMatrix();


        glRotatef(
            14.0f * side,
            0.0f,
            0.0f,
            1.0f
        );


        glRotatef(
            88.0f,
            0.0f,
            1.0f,
            0.0f
        );


        setDarkMetalMaterial();


        drawTaperedCylinder(
            0.045f,
            0.008f,
            0.22f
        );


        glPopMatrix();
    }
}



void drawLeg(
    float attachmentX,
    float side,
    float baseDirection,
    float hipAngle,
    float kneeAngle,
    float ankleAngle
) {
    const float coxaLength =
        0.35f;

    const float femurLength =
        1.15f;

    const float tibiaLength =
        1.42f;

    const float tarsusLength =
        1.05f;


    glPushMatrix();


    // A perna nasce no torax

    glTranslatef(
        attachmentX,
        0.82f * side,
        -0.08f
    );


    glRotatef(
        baseDirection +
        hipAngle,
        0.0f,
        0.0f,
        1.0f
    );


    glRotatef(
        -125.0f * side,
        1.0f,
        0.0f,
        0.0f
    );


    // Coxa

    drawServoJoint(
        0.19f
    );


    setJointMaterial();

    drawTaperedCylinder(
        0.18f,
        0.14f,
        coxaLength
    );


    glTranslatef(
        0.0f,
        0.0f,
        coxaLength
    );


    drawServoJoint(
        0.15f
    );


    // Abertura anatomica entre coxa e femur

    glRotatef(
        -10.0f * side,
        1.0f,
        0.0f,
        0.0f
    );


    // Femur

    setCarapaceMaterial();

    drawTaperedCylinder(
        0.145f,
        0.105f,
        femurLength
    );


    drawLinearActuator(
        femurLength,
        0.105f
    );


    drawShockAbsorber(
        femurLength,
        -0.105f
    );


    glTranslatef(
        0.0f,
        0.0f,
        femurLength
    );


    drawServoJoint(
        0.145f
    );


    // Articulacao do joelho

    glRotatef(
        kneeAngle * side,
        1.0f,
        0.0f,
        0.0f
    );


    // Tibia

    setCarapaceMaterial();

    drawTaperedCylinder(
        0.115f,
        0.078f,
        tibiaLength
    );


    drawLinearActuator(
        tibiaLength,
        0.082f
    );


    drawShockAbsorber(
        tibiaLength,
        -0.082f
    );


    glTranslatef(
        0.0f,
        0.0f,
        tibiaLength
    );


    drawServoJoint(
        0.115f
    );


    // Articulacao do tarso

    glRotatef(
        ankleAngle * side,
        1.0f,
        0.0f,
        0.0f
    );


    // Tarso

    setDarkMetalMaterial();

    drawTaperedCylinder(
        0.070f,
        0.040f,
        tarsusLength
    );


    glTranslatef(
        0.0f,
        0.0f,
        tarsusLength
    );


    setJointMaterial();

    drawSphere(
        0.075f
    );


    // Garras de contato

    drawClaws();


    glPopMatrix();
}



void drawAnt(void) {
    glPushMatrix();


    glRotatef(
        robotYaw,
        0.0f,
        0.0f,
        1.0f
    );


    glTranslatef(
        0.0f,
        0.0f,
        bodyBob
    );


    glRotatef(
        bodyRoll,
        1.0f,
        0.0f,
        0.0f
    );


    glRotatef(
        bodyYaw,
        0.0f,
        0.0f,
        1.0f
    );


    drawThorax();

    drawHead();

    drawPetiole();

    drawAbdomen();


    // Lado esquerdo

    drawLeg(
        0.62f,
        1.0f,
        -28.0f,

        legHipAngle[FRONT_LEFT],
        legKneeAngle[FRONT_LEFT],
        legAnkleAngle[FRONT_LEFT]
    );


    drawLeg(
        0.0f,
        1.0f,
        0.0f,

        legHipAngle[MIDDLE_LEFT],
        legKneeAngle[MIDDLE_LEFT],
        legAnkleAngle[MIDDLE_LEFT]
    );


    drawLeg(
        -0.62f,
        1.0f,
        28.0f,

        legHipAngle[REAR_LEFT],
        legKneeAngle[REAR_LEFT],
        legAnkleAngle[REAR_LEFT]
    );


    // Lado direito

    drawLeg(
        0.62f,
        -1.0f,
        28.0f,

        legHipAngle[FRONT_RIGHT],
        legKneeAngle[FRONT_RIGHT],
        legAnkleAngle[FRONT_RIGHT]
    );


    drawLeg(
        0.0f,
        -1.0f,
        0.0f,

        legHipAngle[MIDDLE_RIGHT],
        legKneeAngle[MIDDLE_RIGHT],
        legAnkleAngle[MIDDLE_RIGHT]
    );


    drawLeg(
        -0.62f,
        -1.0f,
        -28.0f,

        legHipAngle[REAR_RIGHT],
        legKneeAngle[REAR_RIGHT],
        legAnkleAngle[REAR_RIGHT]
    );


    glPopMatrix();
}



void drawScene(void) {
    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );


    glMatrixMode(
        GL_MODELVIEW
    );

    glLoadIdentity();


    float orbitX =
        static_cast<float>(
            eyeDistance *
            cos(
                viewAngleZ *
                PI /
                180.0
            ) *
            cos(
                viewAngleX *
                PI /
                180.0
            )
            );


    float orbitY =
        static_cast<float>(
            eyeDistance *
            cos(
                viewAngleZ *
                PI /
                180.0
            ) *
            sin(
                viewAngleX *
                PI /
                180.0
            )
            );


    float orbitZ =
        static_cast<float>(
            eyeDistance *
            sin(
                viewAngleZ *
                PI /
                180.0
            )
            );


    eyeX =
        cameraTargetX +
        orbitX;

    eyeY =
        cameraTargetY +
        orbitY;

    eyeZ =
        cameraTargetZ +
        orbitZ;


    if (
        viewAngleZ <
        90.0f
        ) {
        gluLookAt(
            eyeX,
            eyeY,
            eyeZ,

            cameraTargetX,
            cameraTargetY,
            cameraTargetZ,

            0.0,
            0.0,
            1.0
        );
    }
    else {
        gluLookAt(
            eyeX,
            eyeY,
            eyeZ,

            cameraTargetX,
            cameraTargetY,
            cameraTargetZ,

            0.0,
            0.0,
            -1.0
        );
    }


    computeScannerWorldState();


    // Luz fixa no mundo

    if (lightingOn) {
        GLfloat lightDirection[] = {
            -6.0f,
            -4.0f,
            8.0f,
            0.0f
        };

        glLightfv(
            GL_LIGHT0,
            GL_POSITION,
            lightDirection
        );
    }

    updateScannerLight();


    drawGround();

    drawAnt();

    drawScannerBeamAndMarker();


    glutSwapBuffers();
}



int main(
    int argc,
    char** argv
) {
    glutInit(
        &argc,
        argv
    );


    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_DEPTH
    );


    glutInitWindowSize(
        windowWidth,
        windowHeight
    );


    glutCreateWindow(
        "Formiga Robo"
    );


    initRendering();

    updateProjection();


    glutTimerFunc(
        25,
        update,
        0
    );


    glutDisplayFunc(
        drawScene
    );


    glutKeyboardFunc(
        handleKeypress
    );


    glutSpecialFunc(
        handleSpecialKeypress
    );


    glutReshapeFunc(
        handleResize
    );


    cout << "CONTROLES" << endl;

    cout
        << "W/Z/A/S - orbita da camera"
        << endl;

    cout
        << "+/-     - zoom"
        << endl;

    cout
        << "Setas   - pan"
        << endl;

    cout
        << "X       - reset da camera"
        << endl;

    cout
        << "G/D     - girar a formiga"
        << endl;

    cout
        << "P       - caminhada"
        << endl;

    cout
        << "T       - textura"
        << endl;

    cout
        << "L       - iluminacao"
        << endl;

    cout
        << "Q/E     - cabeca"
        << endl;

    cout
        << "1/2 3/4 - antena esquerda"
        << endl;

    cout
        << "5/6 7/8 - antena direita"
        << endl;

    cout
        << "C       - selecionar perna"
        << endl;

    cout
        << "R/F     - quadril"
        << endl;

    cout
        << "Y/H     - joelho"
        << endl;

    cout
        << "U/J     - tarso"
        << endl;


    glutMainLoop();


    return 0;
}