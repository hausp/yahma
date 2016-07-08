#include <array>
#include <fstream>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
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

using Point = std::array<double, 3>;
using Triangle = std::array<Point, 3>;
using Texture = std::array<double, 2>;

// Globals
unsigned winWidth = 800;
unsigned winHeight = 600;
std::vector<Point> vertices;
std::vector<Point> normals;
std::vector<Triangle> faces;
std::vector<Texture> textures;
GLuint texture;
// -----------------------------------------

void loadObj(const std::string& filename) {
    vertices.clear();
    normals.clear();
    faces.clear();
    textures.clear();
    std::ifstream input(filename);
    std::string line;
    while (std::getline(input, line)) {
        if (line.size() == 0 || line[0] == '#') {
            continue;
        }
        std::istringstream stream(line);
        std::string command;
        stream >> command;
        if (command == "v") {
            Point vertex;
            stream >> vertex[0];
            stream >> vertex[1];
            stream >> vertex[2];
            vertices.push_back(vertex);
        } else if (command == "vn") {
            Point vertex;
            stream >> vertex[0];
            stream >> vertex[1];
            stream >> vertex[2];
            normals.push_back(vertex);
        } else if (command == "vt") {
            Texture newTexture;
            stream >> newTexture[0];
            stream >> newTexture[1];
            textures.push_back(newTexture);
        } else if (command == "f") {
            Triangle face;
            for (unsigned i = 0; i < 3; i++) {
                for (unsigned j = 0; j < 3; j++) {
                    stream >> face[j][i];
                }
            }
            faces.push_back(face);
        }
    }
}

void drawCharacter() {
    ECHO("Drawing character...");
    glPushMatrix();
    glScaled(3.0, 3.0, 3.0);

    glBindTexture(GL_TEXTURE_2D, texture);
    for (auto face : faces) {
        glBegin(GL_TRIANGLES);
        for (unsigned i = 0; i < 3; i++) {
            auto vertex = vertices[face[0][i]];
            auto currTexture = textures[face[1][i]];
            auto normal = normals[face[2][i]];
            glTexCoord2d(currTexture[0], currTexture[1]);
            glVertex3d(vertex[0], vertex[1], vertex[2]);
            glNormal3d(normal[0], normal[1], normal[2]);
        }
        glEnd();
    }
    glPopMatrix();
}

void reshape(int newWidth, int newHeight) {
    winWidth = newWidth;
    winHeight = newHeight;

    glViewport(0, 0, winWidth, winHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLoadIdentity();

    //drawCharacter();
    glBegin(GL_TRIANGLES); // início triângulo
    glVertex3f( 0.0f, 1.0f, 0.0f); // Topo
    glVertex3f(-1.0f,-1.0f, 0.0f); // Esquerda embaixo
    glVertex3f( 1.0f,-1.0f, 0.0f); // Direita embaixo
    glEnd(); // fim triângulo

    glutSwapBuffers();
}

void idle() {
    display();
}

bool init() {
    ECHO("init");
    glShadeModel(GL_SMOOTH);
    // Color for cleaning the window
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    loadObj("subzero.obj");
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
    auto window = glutCreateWindow("YAHMA");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutSpecialFunc(onSpecialKeyPress);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
