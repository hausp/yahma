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
std::string title = "YAHMA";
std::string model = "dinomech.obj";
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
    vertices.push_back(Point());
    normals.push_back(Point());
    faces.push_back(Triangle());
    textures.push_back(Texture());
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
                    face[i][j] = 0;
                }
            }
            for (unsigned i = 0; i < 3; i++) {
                std::string triple;
                stream >> triple;
                std::istringstream iss(triple);
                std::string container;
                unsigned j = 0;
                while (std::getline(iss, container, '/')) {
                    face[j][i] = stoi(container);
                    j++;
                }
            }
            faces.push_back(face);
        }
    }
}

void drawCharacter() {
    glPushMatrix();
    // Subzero
    // glTranslated(0, 0.3, 0);
    // glScaled(0.3, 0.3, 0.3);
    // Dinomech
    glRotated(90, 0, 1, 0);
    glTranslated(0, -0.9, 0);
    glScaled(0.04, 0.04, 0.04);

    bool hasTextures = (textures.size() > 0);
    bool hasNormals = (normals.size() > 0);
    glBindTexture(GL_TEXTURE_2D, texture);
    for (auto face : faces) {
        glBegin(GL_TRIANGLES);
        for (unsigned i = 0; i < 3; i++) {
            auto vertex = vertices[face[0][i]];
            glVertex3d(vertex[0], vertex[1], vertex[2]);
            if (hasTextures) {
                auto currTexture = textures[face[1][i]];
                glTexCoord2d(currTexture[0], currTexture[1]);                
            }
            if (hasNormals) {
                auto normal = normals[face[2][i]];
                glNormal3d(normal[0], normal[1], normal[2]);                
            }
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

    drawCharacter();

    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

bool init() {
    glShadeModel(GL_SMOOTH);
    // Color for cleaning the window
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    loadObj(model);
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
    auto window = glutCreateWindow(title.c_str());
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutSpecialFunc(onSpecialKeyPress);
    glutIdleFunc(idle);
    init();
    glutMainLoop();
}
