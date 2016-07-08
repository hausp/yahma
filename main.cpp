#include <GL/glut.h>
#include <iostream>

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

unsigned winWidth = 800;
unsigned winHeight = 600;

void reshape(int newWidth, int newHeight) {
    winWidth = newWidth;
    winHeight = newHeight;
    ECHO("reshape");
}

void display() {
    ECHO("display");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLoadIdentity();
    glutSwapBuffers();
}

void idle() {
    static bool onlyOnce = false;
    if (!onlyOnce) {
        ECHO("idle");
        onlyOnce = true;
    }
}

bool init() {
    ECHO("init");
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    return true;
}

void onKeyPress(unsigned char key, int mouseX, int mouseY) {
    ECHO("NORMAL");
    TRACE(key);
}

void onSpecialKeyPress(int key, int mouseX, int mouseY) {
    ECHO("SPECIAL");
    TRACE(key);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(0, 0);
    auto window = glutCreateWindow("TODO: name");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutSpecialFunc(onSpecialKeyPress);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
