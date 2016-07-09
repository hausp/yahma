#include <array>
#include <cmath>
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
// using Triangle = std::array<Point, 3>;
// using Texture = std::array<double, 2>;
using AngleGroup = std::array<double, 3>;
struct Material {
    double ns, ni, d;
    float ka[4], kd[4], ks[4];
    unsigned illum;
};

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
std::string model = "subzero.obj";
std::unordered_map<std::string, int> keyMap = {
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
float lightPosition[] = {0, 20, 1, 1};
unsigned long long globalTime = 0;

double robotAngle = 30;
float ambientCoefs[] = {1, 1, 1, 0.7};
float diffuseCoefs[] = {1, 1, 1, 1};
float specularCoefs[] = {1, 1, 1, 0.2};
int specularExponent = 90;
Size headSize = {0.3, 0.2, 0.2};
Size bodySize = {0.4, 0.5, 0.2};
Size armSize = {0.25, 0.05, 0.05};
Size legSize = {0.05, 0.3, 0.05};
double neckHeight = 0.05;
Point robotCenter = {0, 0, 0};
AngleGroup headAngles = {0, 0, 0};
AngleGroup leftArmAngles = {0, 0, -80};
AngleGroup rightArmAngles = {0, 0, 80};
AngleGroup leftLegAngles = {0, 0, 0};
AngleGroup rightLegAngles = {0, 0, 0};
AngleGroup bodyAngles = {0, 0, 0};
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
    // Front
    // glBegin(GL_POLYGON);
    // glVertex3d(-size.width/2, -size.height/2, -size.thickness/2);
    // glVertex3d(size.width/2, -size.height/2, -size.thickness/2);
    // glVertex3d(size.width/2, size.height/2, -size.thickness/2);
    // glVertex3d(-size.width/2, size.height/2, -size.thickness/2);
    // glEnd();

    // // Back
    // glBegin(GL_POLYGON);
    // glVertex3d(-size.width/2, -size.height/2, size.thickness/2);
    // glVertex3d(size.width/2, -size.height/2, size.thickness/2);
    // glVertex3d(size.width/2, size.height/2, size.thickness/2);
    // glVertex3d(-size.width/2, size.height/2, size.thickness/2);
    // glEnd();

    // // Left
    // glBegin(GL_POLYGON);
    // glVertex3d(-size.width/2, -size.height/2, size.thickness/2);
    // glVertex3d(size.width/2, -size.height/2, -size.thickness/2);
    // glVertex3d(size.width/2, size.height/2, size.thickness/2);
    // glVertex3d(-size.width/2, size.height/2, -size.thickness/2);
    // glEnd();
}

void drawHead() {
    glPushMatrix();
    glTranslated(robotCenter[0],
                 robotCenter[1] + bodySize.height/2 + headSize.height/2 + neckHeight,
                 robotCenter[2]);
    rotate(headAngles);
    box(headSize);
    glPopMatrix();
}

void drawLeftArm() {
    glPushMatrix();
    glTranslated(robotCenter[0] + bodySize.width/2,
                 robotCenter[1] + bodySize.height/4,
                 robotCenter[2]);
    rotate(leftArmAngles);
    glTranslated(armSize.width/2, 0, 0);

    // static GLUquadricObj* quadric = gluNewQuadric();
    // gluCylinder(quadric, 0.025, 0.025, armSize.height, 100, 100);
    box(armSize);

    glPopMatrix();
}

void drawRightArm() {
    glPushMatrix();
    glTranslated(robotCenter[0] - bodySize.width/2,
                 robotCenter[1] + bodySize.height/4,
                 robotCenter[2]);
    rotate(rightArmAngles);
    glTranslated(-armSize.width/2, 0, 0);
    box(armSize);
    glPopMatrix();
}

void drawLeftLeg() {
    glPushMatrix();
    glTranslated(robotCenter[0] - bodySize.width/4,
                 robotCenter[1] - bodySize.height/2,
                 robotCenter[2]);
    rotate(leftLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    box(legSize);
    glPopMatrix();
}

void drawRightLeg() {
    glPushMatrix();
    glTranslated(robotCenter[0] + bodySize.width/4,
                 robotCenter[1] - bodySize.height/2,
                 robotCenter[2]);
    rotate(rightLegAngles);
    glTranslated(0, -legSize.height/2, 0);
    box(legSize);
    glPopMatrix();
}

void drawBody() {
    glPushMatrix();
    glPushMatrix();
    glTranslated(robotCenter[0], robotCenter[1], robotCenter[2]);
    rotate(bodyAngles);
    box(bodySize);
    glPopMatrix();

    drawLeftLeg();
    drawRightLeg();
    glPopMatrix();
}

void drawRobot() {
    glPushMatrix();
    glRotated(robotAngle, 0, 1, 0);

    drawHead();
    drawLeftArm();
    drawRightArm();
    drawBody();

    glPopMatrix();
}

// Called when the window is resized.
void reshape(int newWidth, int newHeight) {
    winWidth = newWidth;
    winHeight = newHeight;

    glViewport(0, 0, winWidth, winHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Clears and displays the screen.
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLoadIdentity();

    //drawCharacter();
    drawRobot();

    glutSwapBuffers();
}

double oscillate(unsigned period, int lowerValue, int higherValue) {
    double frac = (static_cast<int>(globalTime) % period) / (period + 0.0);
    double coef = std::abs(2 * std::abs(frac - 0.5) - 1);
    return coef * (higherValue - lowerValue) + lowerValue;
}

// Updates all animated properties and the screen.
void idle() {
    globalTime++;
    // leftArmAngles[2] = oscillate(100, -70, 70);
    // rightArmAngles[2] = oscillate(100, -70, 70);
    // leftLegAngles[0] = oscillate(100, -70, 70);
    // rightLegAngles[0] = oscillate(100, -70, 70);
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
    //glLightfv(GL_LIGHT0, GL_AMBIENT, ambientCoefs);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseCoefs);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // loadObj(model);
    return true;
}

// Checks if a given special key code corresponds to a given key name.
bool is(int key, const std::string& keyName) {
    return key == keyMap[keyName];
}

// Called when a normal key is pressed.
void onKeyPress(unsigned char key, int mouseX, int mouseY) {
    ECHO("NORMAL");
    TRACE(key);
}

// Called when a special key (e.g arrows and shift) is pressed.
void onSpecialKeyPress(int key, int mouseX, int mouseY) {

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
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
