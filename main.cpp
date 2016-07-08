#include <array>
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
using Triangle = std::array<Point, 3>;
using Texture = std::array<double, 2>;
struct Material {
    double ns, ni, d;
    float ka[4], kd[4], ks[4];
    unsigned illum;
};

std::ostream& operator<<(std::ostream& stream, const Point& point) {
    return stream << "(" << point[0] << "," << point[1] << "," << point[2] << ")";
}

std::ostream& operator<<(std::ostream& stream, const Triangle& triangle) {
    return stream << "(" << triangle[0] << "," << triangle[1] << "," << triangle[2] << ")";
}

std::ostream& operator<<(std::ostream& stream, const Texture& texture) {
    return stream << "(" << texture[0] << "," << texture[1] << ")";
}

// Globals
std::string title = "YAHMA";
std::string model = "subzero.obj";
unsigned winWidth = 800;
unsigned winHeight = 600;
std::vector<Point> vertices;
std::vector<Point> normals;
std::vector<Triangle> faces;
std::vector<Texture> textures;
std::unordered_map<std::string, Material> materials;
std::queue<std::pair<unsigned, std::string>> faceMaterials;
GLuint texture;
float lightPosition[] = {0, 20, 1, 1};
// -----------------------------------------

std::unordered_map<std::string, Material> parseMtl(const std::string& filename) {
    std::unordered_map<std::string, Material> result;
    std::string name;
    Material material;
    bool valid = false;
    std::ifstream input(filename);
    std::string line;
    while (std::getline(input, line)) {
        if (line.size() == 0 || line[0] == '#') {
            continue;
        }
        std::istringstream stream(line);
        std::string command;
        stream >> command;
        if (command == "newmtl") {
            if (valid) {
                result.insert(std::make_pair(name, material));
                material = Material();
            }
            stream >> name;
            valid = true;
        } else if (command == "Ns") {
            stream >> material.ns;
        } else if (command == "Ka") {
            stream >> material.ka[0];
            stream >> material.ka[1];
            stream >> material.ka[2];
            material.ka[3] = 1;
        } else if (command == "Kd") {
            stream >> material.kd[0];
            stream >> material.kd[1];
            stream >> material.kd[2];
            material.kd[3] = 1;
        } else if (command == "Ks") {
            stream >> material.ks[0];
            stream >> material.ks[1];
            stream >> material.ks[2];
            material.ks[3] = 1;
        } else if (command == "Ni") {
            stream >> material.ni;
        } else if (command == "d") {
            stream >> material.d;
        } else if (command == "illum") {
            stream >> material.illum;
        }
    }

    if (valid) {
        result.insert(std::make_pair(name, material));
    }

    return result;
}

void loadObj(const std::string& filename) {
    vertices.clear();
    normals.clear();
    faces.clear();
    textures.clear();
    materials.clear();
    while (!faceMaterials.empty()) {
        faceMaterials.pop();    
    }
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
        } else if (command == "mtllib") {
            std::string filename;
            stream >> filename;
            materials = parseMtl(filename);
        } else if (command == "usemtl") {
            std::string name;
            stream >> name;
            faceMaterials.push(std::make_pair(faces.size(), name));
        }
    }
}

void drawCharacter() {
    glPushMatrix();
    // Subzero
    glTranslated(0, 0.3, 0);
    glScaled(0.3, 0.3, 0.3);
    // Dinomech
    // glRotated(90, 0, 1, 0);
    // glTranslated(0, -0.9, 0);
    // glScaled(0.04, 0.04, 0.04);

    bool hasTextures = (textures.size() > 0);
    bool hasNormals = (normals.size() > 0);
    Material material;
    bool validMaterial = false;
    unsigned faceNumber = 0;
    auto queueCopy = faceMaterials;

    glBindTexture(GL_TEXTURE_2D, texture);
    for (auto face : faces) {
        if (faceMaterials.size() > 0 && faceNumber == faceMaterials.front().first) {
            material = materials[faceMaterials.front().second];
            faceMaterials.pop();
            validMaterial = true;
        }

        if (validMaterial) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material.ka);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material.kd);
            glMaterialfv(GL_FRONT, GL_SPECULAR, material.ks);
            glMateriali(GL_FRONT, GL_SHININESS, material.ns / 1000.0 * 128);
        }
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
        faceNumber++;
    }
    glPopMatrix();
    faceMaterials = queueCopy;
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

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

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
