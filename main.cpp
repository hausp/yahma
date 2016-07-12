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
using Color3 = std::array<float, 3>;
using Color4 = std::array<float, 4>;

struct Size {
    double width;
    double height;
    double thickness;
};

enum class Mode {
    WALKING,
    JUMPING_JACKS
};

std::ostream& operator<<(std::ostream& stream, const std::array<double, 3>& array) {
    return stream << "(" << array[0] << "," << array[1] << "," << array[2] << ")";
}

// Globals
std::string title = "YAHMA";
unsigned winWidth = 800;
unsigned winHeight = 600;
const unsigned fixedWidth = 800;
const unsigned fixedHeight = 600;
bool fullscreen = false;

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

float lightPosition[] = {0, 50, 0, 1};
float lightCoefs[] = {0.5, 0.5, 0.5, 1.0};
Color4 stdColor = {0, 0, 0, 1};

unsigned long long globalTime = 0;

auto mode = Mode::JUMPING_JACKS;

const double rotationSpeed = 0.7;
const double moveSpeed = 0.007;
const double neckHeight = 0.05;
const double jointRadius = 0.04;
const double shoulderOffset = 0.35;
const double cameraMoveSpeed = 0.03;
const int bendingAngle = 20;

// Camera related stuff
double zoom = 1;
const double zoomSpeed = 0.1, zNear = 0.1, zFar = 100;
Point camera = {0, 1, -3}, lookAt = {0, 0, 0};

// Nice moving around related stuff
double moveRadius = 3;
double theta = 0, phi = 0;
int oldX, oldY;
bool rotating = false;
short move = 0;
short spin = 0;
std::array<short, 2> cameraMove = {0, 0};

Size headSize = {0.3, 0.2, 0.2};
Size bodySize = {0.4, 0.5, 0.2};
Size armSize = {0.125, 0.05, 0.05};
Size forearmSize = {0.125, 0.05, 0.05};
Size legSize = {0.05, 0.15, 0.05};
Size thighSize = {0.05, 0.25, 0.05};
double legTotalHeight = legSize.height + thighSize.height + jointRadius - 0.02;

Point velocity = {0, 0, 0};
Point robotCenter = {0, 0, 0};
AngleGroup robotAngles = {0, 180, 0};
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
// -----------------------------------------

void onMouseMove(int, int);
void onMousePress(int, int, int, int);

void rotate(const AngleGroup& angles) {
    glRotated(angles[0], 1, 0, 0);
    glRotated(angles[1], 0, 1, 0);
    glRotated(angles[2], 0, 0, 1);
}

void drawBox(const Size& size,
         const Color4& color = {1, 1, 1, 1},
         int specularCoef = 0,
         bool sameAs = false) {
    glPushMatrix();
    glScaled(size.width, size.height, size.thickness);
    glColor4f(color[0], color[1], color[2], color[3]);
    if (sameAs) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, &color[0]);
    } else {
        glMaterialfv(GL_FRONT, GL_SPECULAR, &stdColor[0]);
    }
    glMateriali(GL_FRONT, GL_SHININESS, specularCoef);
    glutSolidCube(1);
    glPopMatrix();
}

void drawSphere(double radius,
                double x, double y,
                const Color4& color = {1, 1, 1, 1},
                int specularCoef = 0,
                bool sameAs = false) {
    glPushMatrix();
    glColor4f(color[0], color[1], color[2], color[3]);
    if (sameAs) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, &color[0]);
    } else {
        glMaterialfv(GL_FRONT, GL_SPECULAR, &stdColor[0]);
    }
    glMateriali(GL_FRONT, GL_SHININESS, specularCoef);
    glutSolidSphere(radius, x, y);
    glPopMatrix();
}

void drawHead() {
    glPushMatrix();
    glTranslated(0,
                 0 + bodySize.height/2 + headSize.height/2 + neckHeight,
                 0);
    rotate(headAngles);
    drawBox(headSize);
    glPopMatrix();
}

void drawLeftArm() {
    glPushMatrix();
    glTranslated(bodySize.width/2,
                 bodySize.height * shoulderOffset,
                 0);

    drawSphere(jointRadius, 100, 100, {0.3, 0.1, 0.3, 1});

    rotate(leftArmAngles);
    glTranslated(armSize.width/2, 0, 0);
    drawBox(armSize, {0.4, 0.4, 0.4, 1});

    glTranslated(armSize.width/2 + jointRadius/2, 0, 0);
    drawSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(leftForearmAngles);
    glTranslated(forearmSize.width/2, 0, 0);
    drawBox(forearmSize);
    glPopMatrix();

    glPopMatrix();
}

void drawRightArm() {
    glPushMatrix();
    glTranslated(-bodySize.width/2,
                 bodySize.height * shoulderOffset,
                 0);

    drawSphere(jointRadius, 100, 100, {0.3, 0.1, 0.3, 1});

    rotate(rightArmAngles);
    glTranslated(-armSize.width/2, 0, 0);
    drawBox(armSize, {0.4, 0.4, 0.4, 1});

    glTranslated(-armSize.width/2 - jointRadius/2, 0, 0);
    drawSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(rightForearmAngles);
    glTranslated(-forearmSize.width/2, 0, 0);
    drawBox(forearmSize);
    glPopMatrix();

    glPopMatrix();
}

void drawLeftLeg() {
    glPushMatrix();
    glTranslated(bodySize.width/4,
                 -bodySize.height/2,
                 0);

    drawSphere(jointRadius, 100, 100, {0.3, 0.1, 0.3, 1});

    rotate(leftLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    drawBox(legSize, {0.4, 0.4, 0.4, 1});

    glTranslated(0, -legSize.height/2 - jointRadius/2, 0);
    drawSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(leftThighAngles);
    glTranslated(0, -thighSize.height/2, 0);
    drawBox(thighSize);
    glPopMatrix();

    glPopMatrix();
}

void drawRightLeg() {
    glPushMatrix();
    glTranslated(-bodySize.width/4,
                 -bodySize.height/2,
                 0);

    drawSphere(jointRadius, 100, 100, {0.3, 0.1, 0.3, 1});

    rotate(rightLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    drawBox(legSize, {0.4, 0.4, 0.4, 1});

    glTranslated(0, -legSize.height/2 - jointRadius/2, 0);
    drawSphere(jointRadius, 100, 100);

    glPushMatrix();
    rotate(rightThighAngles);
    glTranslated(0, -thighSize.height/2, 0);
    drawBox(thighSize);
    glPopMatrix();

    glPopMatrix();
}

void drawBody() {
    glPushMatrix();

    drawLeftArm();
    drawRightArm();
    drawLeftLeg();
    drawRightLeg();

    drawBox(bodySize, {0.3, 0.1, 0.3, 1});
    
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
    glPushMatrix();
    glTranslated(0,
                 -(bodySize.height/2 + legTotalHeight) - 50,
                 0);
    drawBox({100, 100, 100});
    glColor4f(0.5, 0.5, 0.5, 1);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &stdColor[0]);
    glMateriali(GL_FRONT, GL_SHININESS, 5);
    glPopMatrix();
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
    setupModelViewMatrix();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    robotCenter[1] = 0;
}

Point polarToCartesian(double distance, double angle) {
    double a = angle * M_PI / 180;
    double s = sin(a);
    double c = cos(a);
    return {-distance * s, 0, -distance * c};
}

static double ftime() {
    struct timeval t;
    gettimeofday(&t, NULL);

    return 1.0*t.tv_sec + 1e-6*t.tv_usec;
}

double genericCoef(unsigned long long timestamp, unsigned period) {
    return (timestamp % period) / (period + 0.0);
}

double riseCoef(unsigned period) {
    return genericCoef(globalTime, period);
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

double irregularOscillate(unsigned period, double from, double to, double end) {
    static bool state = false;
    static double lastFrac = 0;
    period /= 2;
    double frac = riseCoef(period);
    if (frac < lastFrac) {
        state = !state;
    }
    lastFrac = frac;
    if (!state) {
        return rise(period, from, to);        
    } else {
        return rise(period, to, end);        
    }
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
    static double timeOffset = 0;
    const unsigned bendingPeriod = 0.2 * period;
    const unsigned legMovePeriod = (period - bendingPeriod) / 2;
    unsigned currPeriod = (state == FLOOR_CLOSED || state == FLOOR_OPEN)
                        ? bendingPeriod
                        : legMovePeriod;
    globalTime += timeOffset;
    double frac = riseCoef(currPeriod);
    if (frac < lastFrac) {
        state = (state + 1) % numStates;
        timeOffset = 0;
        if (state == OPENING_LEGS || state == CLOSING_LEGS) {
            auto coef1 = genericCoef(globalTime, bendingPeriod);
            auto coef2 = genericCoef(globalTime, legMovePeriod);
            if (std::abs(coef1 - coef2) >= 0.2) {
                timeOffset = legMovePeriod - bendingPeriod;
            }
        }
    }
    lastFrac = frac;

    double angle = 40;
    double c = cos(angle * M_PI / 180);
    double offset = legTotalHeight * (1 - c);
    switch (state) {
        case FLOOR_CLOSED:
        case FLOOR_OPEN:
            bend(currPeriod);
            break;
        case OPENING_LEGS:
            riseArms(currPeriod);
            unbend(currPeriod);
            leftLegAngles[2] = rise(currPeriod, 0, angle);
            rightLegAngles[2] = rise(currPeriod, 0, -angle);
            robotCenter[1] = irregularOscillate(currPeriod, 0, 0.1, -offset);
            break;
         case CLOSING_LEGS:
            lowerArms(currPeriod);
            unbend(currPeriod);
            leftLegAngles[2] = rise(currPeriod, angle, 0);
            rightLegAngles[2] = rise(currPeriod, -angle, 0);
            robotCenter[1] = irregularOscillate(currPeriod, -offset, 0.1, 0);
            break;
    }
    globalTime -= timeOffset;
}

// Updates all animated properties and the screen.
void idle() {
    auto currentTime = ftime() * 100;
    auto diff = currentTime - globalTime;
    globalTime = currentTime;
    unsigned period;
    robotAngles[1] += diff * rotationSpeed * spin;
    lookAt[0] += diff * cameraMove[0] * cameraMoveSpeed;
    lookAt[1] += diff * cameraMove[1] * cameraMoveSpeed;
    if (mode == Mode::JUMPING_JACKS) {
        period = 120;
        jump(period);
    } else if (mode == Mode::WALKING) {
        if (move != 0) {
            period = 100;
            headAngles[1] = oscillate(period, -5, 5);
            bodyAngles[1] = oscillate(period, -10, 10);
            leftArmAngles[0] = oscillate(period, 25, -25);
            rightArmAngles[0] = oscillate(period, -25, 25);

            leftLegAngles[0] = oscillate(period, -30, 30);
            rightLegAngles[0] = oscillate(period, 30, -30);
            leftThighAngles[0] = oscillate(period, -30, 0);
            rightThighAngles[0] = oscillate(period, 0, -30);

            robotCenter[1] = oscillate(period/2, -0.1, 0.1);

            velocity = polarToCartesian(diff * move * moveSpeed, robotAngles[1]);
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
    glDepthFunc(GL_LESS);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &stdColor[0]);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, lightCoefs);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    reset();
    onMousePress(GLUT_LEFT_BUTTON, GLUT_DOWN, 0, 0);
    onMouseMove(0, 0);
    return true;
}

// Checks if a given special key code corresponds to a given key name.
bool is(int key, const std::string& keyName) {
    return key == keyMap[keyName];
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
            spin = 1;
            break;
        case 'd':
        case 'D':
            spin = -1;
            break;
        case 'w':
        case 'W':
            move = 1;
            break;
        case 's':
        case 'S':
            move = -1;
            break;
    }
}

// Called when a normal key is released.
void onKeyRelease(unsigned char key, int mouseX, int mouseY) {
    switch (key) {
        case 'a':
        case 'A':
        case 'd':
        case 'D':
            spin = 0;
            break;
        case 'w':
        case 'W':
        case 's':
        case 'S':
            move = 0;
            if (mode == Mode::WALKING) {
                reset();
            }
            break;
    }
}

// Called when a special key (e.g arrows and shift) is pressed.
void onSpecialKeyPress(int key, int mouseX, int mouseY) {
    if (is(key, "LEFT")) {
        cameraMove[0] = -1;
    } else if (is(key, "RIGHT")) {
        cameraMove[0] = 1;
    } else if (is(key, "UP")) {
        cameraMove[1] = 1;
    } else if (is(key, "DOWN")) {
        cameraMove[1] = -1;
    } else if (is(key, "F1")) {
        zoom += zoomSpeed;
        updateProjectionMatrix();
    } else if (is(key, "F2")) {
        zoom -= zoomSpeed;
        updateProjectionMatrix();
    } else if (is(key, "F3")) {
        if (!fullscreen) {
            glutFullScreen();
            fullscreen = true;
        } else {
            glutPositionWindow(0,0);
            glutReshapeWindow(fixedWidth, fixedHeight);
            fullscreen = false;
        }
    } else if (is(key, "F9")) {
        camera[0] = 1.18547;
        camera[1] = -0.663793;
        camera[2] = 2.6747;
    } 
}

// Called when a special key (e.g arrows and shift) is released.
void onSpecialKeyRelease(int key, int mouseX, int mouseY) {
    if (is(key, "LEFT")) {
        cameraMove[0] = 0;
    } else if (is(key, "RIGHT")) {
        cameraMove[0] = 0;
    } else if (is(key, "UP")) {
        cameraMove[1] = 0;
    } else if (is(key, "DOWN")) {
        cameraMove[1] = 0;
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
    glutSpecialUpFunc(onSpecialKeyRelease);
    glutMouseFunc(onMousePress);
    glutMotionFunc(onMouseMove);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
