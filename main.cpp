#include <array>
#include <cmath>
#include <ctime>
#include <sys/time.h>
#include <fstream>
#include <GL/glut.h>
#include <iostream>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#define TRACE(x) std::cout << (#x) << " = " << (x) << std::endl
#define TRACE_L(x,y) std::cout << (x) << " = " << (y) << std::endl
#define TRACE_IT(x) \
    {\
        unsigned counter = 0; \
        for (auto& elem : (x)) { \
            std::cout << (#x) << "[" << std::to_string(counter++) << "] = " << elem << std::endl; \
        }\
    }
#define ECHO(x) std::cout << (x) << std::endl

// Types
using Point = std::array<double, 3>;
using AngleGroup = std::array<double, 3>;

struct Size {
    double width;
    double height;
    double thickness;
};

std::ostream& operator<<(std::ostream& stream, const std::array<double, 3>& array) {
    return stream << "(" << array[0] << "," << array[1] << "," << array[2] << ")";
}

// std::ostream& operator<<(std::ostream& stream, const Triangle& triangle) {
//     return stream << "(" << triangle[0] << "," << triangle[1] << "," << triangle[2] << ")";
// }

// std::ostream& operator<<(std::ostream& stream, const Texture& texture) {
//     return stream << "(" << texture[0] << "," << texture[1] << ")";
// }

// Globals
std::string title = "YAHMA";
std::unordered_map<std::string, int> keyMap = {
    {"F1", 1},
    {"F2", 2},
    {"F3", 3},
    {"F4", 4},
    {"LEFT", 100},
    {"UP", 101},
    {"RIGHT", 102},
    {"DOWN", 103},
    {"LSHIFT", 112},
    {"RSHIFT", 113}
};
unsigned winWidth = 800;
unsigned winHeight = 600;
GLuint texture;
unsigned long long globalTime = 0;

float lightPosition[] = {0, 20, 1, 1};
float lightCoefs[] = {0.5, 0.5, 0.5, 1.0};
float ambientCoefs[] = {1, 1, 1, 0.7};
float diffuseCoefs[] = {1, 1, 1, 1};
float specularCoefs[] = {1, 1, 1, 0.2};
int specularExponent = 90;

enum class Mode {
    WALKING,
    JUMPING_JACKS
};
auto mode = Mode::JUMPING_JACKS;

const double rotationSpeed = 5;
const double moveSpeed = 0.003;
double neckHeight = 0.05;
double jointRadius = 0.04;
double shoulderOffset = 0.35;
const int bendingAngle = 20;

// Camera related stuff
double zoom = 1;
const double zoomSpeed = 0.1, zNear = 2, zFar = 100;
Point camera = {0, 1, -3}, lookAt = {0, 0, 0};

// Nice moving around related stuff
double moveRadius = 3;
double theta = 0, phi = 0;
int oldX, oldY;
bool rotating = false;

Size headSize = {0.3, 0.2, 0.2};
Size bodySize = {0.4, 0.5, 0.2};
Size armSize = {0.125, 0.05, 0.05};
Size forearmSize = {0.125, 0.05, 0.05};
Size legSize = {0.05, 0.15, 0.05};
Size thighSize = {0.05, 0.25, 0.05};

Point velocity = {0, 0, 0};
Point robotCenter = {0, 0, 0};
AngleGroup robotAngles = {0, 60, 0};
AngleGroup headAngles;
AngleGroup bodyAngles;
AngleGroup leftArmAngles;
AngleGroup leftForearmAngles;
AngleGroup rightArmAngles;
AngleGroup rightForearmAngles;
AngleGroup leftThighAngles;
AngleGroup leftLegAngles;
AngleGroup rightThighAngles;
AngleGroup rightLegAngles;

bool move;
// -----------------------------------------

void rotate(const AngleGroup& angles) {
    glRotated(angles[0], 1, 0, 0);
    glRotated(angles[1], 0, 1, 0);
    glRotated(angles[2], 0, 0, 1);
}

void box(const Size& size) {
    glPushMatrix();
    glScaled(size.width, size.height, size.thickness);
    glutSolidCube(1);
    glPopMatrix();
}

void drawHead() {
    glPushMatrix();
    glTranslated(0,
                 0 + bodySize.height/2 + headSize.height/2 + neckHeight,
                 0);
    rotate(headAngles);
    box(headSize);
    glPopMatrix();
}

void drawLeftArm() {
    glPushMatrix();
    glTranslated(bodySize.width/2,
                 bodySize.height * shoulderOffset,
                 0);

    glutSolidSphere(jointRadius, 100, 100);

    rotate(leftArmAngles);
    glTranslated(armSize.width/2, 0, 0);
    box(armSize);

    glTranslated(armSize.width/2 + jointRadius/2, 0, 0);
    glutSolidSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(leftForearmAngles);
    glTranslated(forearmSize.width/2, 0, 0);
    box(forearmSize);
    glPopMatrix();

    glPopMatrix();
}

void drawRightArm() {
    glPushMatrix();
    glTranslated(-bodySize.width/2,
                 bodySize.height * shoulderOffset,
                 0);

    glutSolidSphere(jointRadius, 100, 100);

    rotate(rightArmAngles);
    glTranslated(-armSize.width/2, 0, 0);
    box(armSize);

    glTranslated(-armSize.width/2 - jointRadius/2, 0, 0);
    glutSolidSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(rightForearmAngles);
    glTranslated(-forearmSize.width/2, 0, 0);
    box(forearmSize);
    glPopMatrix();

    glPopMatrix();
}

void drawLeftLeg() {
    glPushMatrix();
    glTranslated(bodySize.width/4,
                 -bodySize.height/2,
                 0);

    glutSolidSphere(jointRadius, 100, 100);

    rotate(leftLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    box(legSize);

    glTranslated(0, -legSize.height/2 - jointRadius/2, 0);
    glutSolidSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(leftThighAngles);
    glTranslated(0, -thighSize.height/2, 0);
    box(thighSize);
    glPopMatrix();

    glPopMatrix();
}

void drawRightLeg() {
    glPushMatrix();
    glTranslated(-bodySize.width/4,
                 -bodySize.height/2,
                 0);

    glutSolidSphere(jointRadius, 100, 100);

    rotate(rightLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    box(legSize);

    glTranslated(0, -legSize.height/2 - jointRadius/2, 0);
    glutSolidSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(rightThighAngles);
    glTranslated(0, -thighSize.height/2, 0);
    box(thighSize);
    glPopMatrix();

    glPopMatrix();
}

void drawBody() {
    glPushMatrix();
    box(bodySize);

    drawLeftArm();
    drawRightArm();
    drawLeftLeg();
    drawRightLeg();
    glPopMatrix();
}

void drawRobot() {
    glPushMatrix();
    glTranslated(robotCenter[0], robotCenter[1], robotCenter[2]);
    rotate(robotAngles);

    drawHead();
    drawBody();

    glPopMatrix();
}

void drawGround() {
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
        //tl
        glVertex3f(-10000.0,
                   -(bodySize.height/2
                     + legSize.height
                     + thighSize.height
                     + 2 * jointRadius),
                   10000000.0);
        //tr
        glVertex3f(10000000.0,
                   -(bodySize.height/2
                    + legSize.height
                    + thighSize.height
                    + 2 * jointRadius),
                   10000000.0);
        //br
        glVertex3f(10000000.0,
                   -(bodySize.height/2
                    + legSize.height
                    + thighSize.height
                    + 2 * jointRadius),
                   -10000000.0);
        //bl
        glVertex3f(-10000000.0,
                   -(bodySize.height/2
                    + legSize.height
                    + thighSize.height
                    + 2 * jointRadius),
                   -10000000.0);
    glEnd();
}

void updateProjectionMatrix() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45 * zoom, // field of view angle in y (degrees)
                   (float)winWidth / (float)winHeight, // aspect ratio
                   zNear, // zNear
                   zFar); // zFar
}

void setupModelViewMatrix() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera[0], camera[1], camera[2], // camera position
              lookAt[0], lookAt[1], lookAt[2], // look at point
              0, 1, 0); // up-vector
}

// Called when the window is resized.
void reshape(int newWidth, int newHeight) {
    winWidth = newWidth;
    winHeight = newHeight;

    glViewport(0, 0, winWidth, winHeight);

    updateProjectionMatrix();
    setupModelViewMatrix();
}

// Clears and displays the screen.
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    setupModelViewMatrix();

    drawGround();

    drawRobot();

    glutSwapBuffers();
}

void reset() {
    headAngles = {0, 0, 0};
    bodyAngles = {0, 0, 0};
    leftArmAngles = {0, 0, -70};
    leftForearmAngles = {0, 0, 0};
    rightArmAngles = {0, 0, 70};
    rightForearmAngles = {0, 0, 0};
    leftThighAngles = {0, 0, 0};
    leftLegAngles = {0, 0, 0};
    rightThighAngles = {0, 0, 0};
    rightLegAngles = {0, 0, 0};
}

static double ftime() {
    struct timeval t;
    gettimeofday(&t, NULL);

    return 1.0*t.tv_sec + 1e-6*t.tv_usec;
}

double riseCoef(unsigned period) {
    return (static_cast<int>(globalTime) % period) / (period + 0.0);
}

double oscillationCoef(unsigned period) {
    return std::abs(2 * std::abs(riseCoef(period) - 0.5) - 1);
}

double rise(unsigned period, double from, double to) {
    return riseCoef(period) * (to - from) + from;
}

double oscillate(unsigned period, double from, double to) {
    return oscillationCoef(period) * (to - from) + from;
}

void bend(unsigned period) {
    leftLegAngles[0] = rise(period, 0, bendingAngle);
    leftThighAngles[0] = rise(period, 0, -bendingAngle);
    rightLegAngles[0] = rise(period, 0, bendingAngle);
    rightThighAngles[0] = rise(period, 0, -bendingAngle);
}

void unbend(unsigned period) {
    leftLegAngles[0] = rise(period, bendingAngle, 0);
    leftThighAngles[0] = rise(period, -bendingAngle, 0);
    rightLegAngles[0] = rise(period, bendingAngle, 0);
    rightThighAngles[0] = rise(period, -bendingAngle, 0);
}

void riseArms(unsigned period) {
    leftArmAngles[2] = rise(period, -70, 70);
    leftForearmAngles[2] = rise(period, -60, 60);
    rightArmAngles[2] = rise(period, 70, -70);
    rightForearmAngles[2] = rise(period, 60, -60);
}

void lowerArms(unsigned period) {
    leftArmAngles[2] = rise(period, 70, -70);
    leftForearmAngles[2] = rise(period, 60, -60);
    rightArmAngles[2] = rise(period, -70, 70);
    rightForearmAngles[2] = rise(period, -60, 60);
}

void jump(unsigned period) {
    const unsigned numStates = 4;
    const unsigned FLOOR_CLOSED = 0;
    const unsigned OPENING_LEGS = 1;
    const unsigned FLOOR_OPEN = 2;
    const unsigned CLOSING_LEGS = 3;
    static unsigned state = FLOOR_CLOSED;
    static double lastFrac = 0;
    const unsigned bendingPeriod = 0.2 * period;
    const unsigned legMovePeriod = (period - bendingPeriod) / 2;
    unsigned currPeriod = (state == FLOOR_CLOSED || state == FLOOR_OPEN)
                        ? bendingPeriod
                        : legMovePeriod;
    double frac = riseCoef(currPeriod);
    if (frac < lastFrac) {
        state = (state + 1) % numStates;
    }
    lastFrac = frac;
    switch (state) {
        case FLOOR_CLOSED:
        case FLOOR_OPEN:
            bend(bendingPeriod);
            break;
        case OPENING_LEGS:
            riseArms(legMovePeriod);
            unbend(legMovePeriod);
            leftLegAngles[2] = rise(legMovePeriod, 0, 40);
            rightLegAngles[2] = rise(legMovePeriod, 0, -40);
            robotCenter[1] = oscillate(legMovePeriod, 0, 0.1);
            break;
        case CLOSING_LEGS:
            lowerArms(legMovePeriod);
            unbend(legMovePeriod);
            leftLegAngles[2] = rise(legMovePeriod, 40, 0);
            rightLegAngles[2] = rise(legMovePeriod, -40, 0);
            robotCenter[1] = oscillate(legMovePeriod, 0, 0.1);
            break;
    }
}

// Updates all animated properties and the screen.
void idle() {
    globalTime = ftime()*100;
    unsigned period;
    if (mode == Mode::JUMPING_JACKS) {
        period = 85;
        jump(period);
    } else if (mode == Mode::WALKING) {
        if (move) {
            period = 70;
            headAngles[1] = oscillate(period, -5, 5);
            bodyAngles[1] = oscillate(period, -10, 10);
            leftArmAngles[0] = oscillate(period, 25, -25);
            rightArmAngles[0] = oscillate(period, -25, 25);

            leftLegAngles[0] = oscillate(period, -50, 50);
            rightLegAngles[0] = oscillate(period, 50, -50);
            leftThighAngles[0] = oscillate(period, -40, 0);
            rightThighAngles[0] = oscillate(period, 0, -40);

            robotCenter[1] = oscillate(period/2, 0, 0.1);

            robotCenter[0] += velocity[0];
            robotCenter[1] += velocity[1];
            robotCenter[2] += velocity[2];
        }
    }
    glutPostRedisplay();
}

// Initializes all important structures.
bool init() {
    glShadeModel(GL_SMOOTH);
    // Color for cleaning the window
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientCoefs);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseCoefs);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularCoefs);
    glMateriali(GL_FRONT, GL_SHININESS, specularExponent);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    // glLightfv(GL_LIGHT0, GL_AMBIENT, lightCoefs);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCoefs);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glDepthFunc(GL_LESS);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    reset();
    return true;
}

// Checks if a given special key code corresponds to a given key name.
bool is(int key, const std::string& keyName) {
    return key == keyMap[keyName];
}

Point polarToCartesian(double distance, double angle) {
    double a = angle * M_PI / 180;
    double s = sin(a);
    double c = cos(a);
    return {-distance * s, 0, -distance * c};
}

// Called when a normal key is pressed.
void onKeyPress(unsigned char key, int mouseX, int mouseY) {
    switch (key) {
        case ' ':
            mode = (mode == Mode::WALKING) ? Mode::JUMPING_JACKS : Mode::WALKING;
            reset();
            break;
        case 'a':
        case 'A':
            robotAngles[1] += rotationSpeed;
            break;
        case 'd':
        case 'D':
            robotAngles[1] -= rotationSpeed;
            break;
        case 'w':
        case 'W':
            velocity = polarToCartesian(moveSpeed, robotAngles[1]);
            move = true;
            break;
        case 's':
        case 'S':
            velocity = polarToCartesian(-moveSpeed, robotAngles[1]);
            move = true;
            break;
    }
}

void onKeyRelease(unsigned char key, int mouseX, int mouseY) {
    switch (key) {
        case 'w':
        case 'W':
            move = false;
            break;
        case 's':
        case 'S':
            move = false;
            break;
    }
}

// Called when a special key (e.g arrows and shift) is pressed.
void onSpecialKeyPress(int key, int mouseX, int mouseY) {
    if (is(key, "LEFT")) {
        camera[0] -= 0.1;
        if (camera[0] < 0) {
            camera[2] += 0.1;
        } else {
            camera[2] -= 0.1;
        }
    } else if (is(key, "RIGHT")) {
        camera[0] += 0.1;
        if (camera[0] > 0) {
            camera[2] += 0.1;
        } else {
            camera[2] -= 0.1;
        }
    } else if (is(key, "UP")) {
        camera[1] += 0.1;
    } else if (is(key, "DOWN")) {
        camera[1] -= 0.1;
    } else if (is(key, "F1")) {
        zoom += zoomSpeed;
        updateProjectionMatrix();
    } else if (is(key, "F2")) {
        zoom -= zoomSpeed;
        updateProjectionMatrix();
    } else if (is(key, "F3")) {
        ECHO("FULLSCREEN");
    }
}

void onMouseMove(int x, int y) {
    if (rotating) {
        theta -= (x - oldX) * 0.01;
        phi += (y - oldY) * 0.01;
        camera[0] = lookAt[0] + moveRadius * cos(phi) * sin(theta);
        camera[1] = lookAt[1] + moveRadius * sin(phi) * sin(theta);
        camera[2] = lookAt[2] + moveRadius * cos(theta);

        setupModelViewMatrix();

        oldX = x;
        oldY = y;
    }
}

void onMousePress(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            rotating = true;
            oldX = x;
            oldY = y;
        } else {
            rotating = false;
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(0, 0);
    auto window = glutCreateWindow(title.c_str());
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutSpecialFunc(onSpecialKeyPress);
    glutKeyboardUpFunc(onKeyRelease);
    glutMouseFunc(onMousePress);
    glutMotionFunc(onMouseMove);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
