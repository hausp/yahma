#include <array>
#include <fstream>
#include <GL/glut.h>
#include <iostream>
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
	std::ifstream input(filename);
	std::string line;
	while (std::getline(input, line)) {
		if (line.size() == 0 || line[0] == '#') {
			continue;
		}
		TRACE(line);
	}
}

void drawCharacter() {
	ECHO("Drawing character...");
}

void reshape(int newWidth, int newHeight) {
	winWidth = newWidth;
	winHeight = newHeight;
	ECHO("reshape");
}

void display() {
	ECHO("display");
}

void idle() {
	static bool onlyOnce = false;
	if (!onlyOnce) {
		ECHO("idle");
		onlyOnce = true;
	}
}

void init() {
	ECHO("init");
	loadObj("subzero.obj");
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
