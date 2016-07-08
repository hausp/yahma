#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

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

// ---------------------------- GLOBAL VARIABLES ---------------------------- //
std::string title = "YAHMA";
unsigned winWidth = 800;
unsigned winHeight = 600;
unsigned frameCount = 0;
int WindowHandle = 0;

GLuint vertexShaderID,
       fragmentShaderID,
       programID,
       vaoID,
       vboID,
       colorBufferID;

const GLchar* vertexShader = {
    "#version 400\n"\

    "layout(location=0) in vec4 in_Position;\n"\
    "layout(location=1) in vec4 in_Color;\n"\
    "out vec4 ex_Color;\n"\

    "void main(void)\n"\
    "{\n"\
    "  gl_Position = in_Position;\n"\
    "  ex_Color = in_Color;\n"\
    "}\n"
};

const GLchar* fragmentShader = {
    "#version 400\n"\

    "in vec4 ex_Color;\n"\
    "out vec4 out_Color;\n"\

    "void main(void)\n"\
    "{\n"\
    "  out_Color = ex_Color;\n"\
    "}\n"
};

// ------------------------------- FUNCTIONS ------------------------------- //
void idle() {
    glutPostRedisplay();
}

void timer(int value) {
    if (0 != value) {
        auto temp = title + ": " + std::to_string(frameCount * 4)
                     + " FPS @ " + std::to_string(winWidth)
                     + " x " + std::to_string(winHeight);

        glutSetWindowTitle(temp.c_str());
    }
    frameCount = 0;
    glutTimerFunc(250, timer, 1);
}

void reshape(int width, int height) {
    winWidth = width;
    winHeight = height;
    glViewport(0, 0, winWidth, winHeight);
}

void display() {
    frameCount++;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glutSwapBuffers();
    glutPostRedisplay();
}

void onKeyPress(unsigned char key, int mouseX, int mouseY) {
    ECHO("NORMAL");
    TRACE(key);
}

void onSpecialKeyPress(int key, int mouseX, int mouseY) {
    ECHO("SPECIAL");
    TRACE(key);
}

void createShaders() {
    auto errorCheckValue = glGetError();

    vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
    glCompileShader(vertexShaderID);

    fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
    glCompileShader(fragmentShaderID);

    programID= glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    glUseProgram(programID);

    errorCheckValue = glGetError();
    if (errorCheckValue != GL_NO_ERROR) {
        std::cerr << "ERROR: Could not create the shaders: "
                  << gluErrorString(errorCheckValue) << std::endl;
        exit(-1);
    }
}

void destroyShaders() {
    auto errorCheckValue = glGetError();

    glUseProgram(0);

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(fragmentShaderID);
    glDeleteShader(vertexShaderID);

    glDeleteProgram(programID);

    errorCheckValue = glGetError();
    if (errorCheckValue != GL_NO_ERROR) {
        std::cout << "ERROR: Could not destroy the shaders: "
                  << gluErrorString(errorCheckValue) << std::endl;
        exit(-1);
    }
}

void createVBO() {
    GLfloat vertices[] = {
        -0.8f, -0.8f, 0.0f, 1.0f,
        0.0f,  0.8f, 0.0f, 1.0f,
        0.8f, -0.8f, 0.0f, 1.0f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };

    auto errorCheckValue = glGetError();

    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    errorCheckValue = glGetError();
    if (errorCheckValue != GL_NO_ERROR) {
        std::cerr << "ERROR: Could not create a VBO: "
                  << gluErrorString(errorCheckValue) << std::endl;
        exit(-1);
    }
}

void destroyVBO() {
    auto errorCheckValue = glGetError();

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &vboID);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vaoID);

    errorCheckValue = glGetError();
    if (errorCheckValue != GL_NO_ERROR) {
        std::cerr << "ERROR: Could not destroy the VBO: "
                  << gluErrorString(errorCheckValue) << std::endl;
        exit(-1);
    }
}

void init() {
    std::cout << "INFO: OpenGL Version:" << glGetString(GL_VERSION) << std::endl;
    createShaders();
    createVBO();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    ECHO("end");
}

void close() {
    destroyShaders();
    destroyVBO();
}

void bindGlutFunctions() {
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyPress);
    glutSpecialFunc(onSpecialKeyPress);
    glutIdleFunc(idle);
    glutTimerFunc(0, timer, 0);
    glutCloseFunc(close);
}

// ---------------------------------- MAIN ---------------------------------- //

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitContextVersion(4, 0);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutInitWindowSize(winWidth, winHeight);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

    WindowHandle = glutCreateWindow(title.c_str());
    if(WindowHandle < 1) {
        std::cerr << "ERROR: Could not create a new rendering window." << std::endl;
        exit(EXIT_FAILURE);
    }

    bindGlutFunctions();

    glewExperimental = GL_TRUE;
    auto glewInitResult = glewInit();
    if (glewInitResult != GLEW_OK) {
        std::cerr << "ERROR: "<< glewGetErrorString(glewInitResult) << std::endl;
        exit(EXIT_FAILURE);
    }

    init();

    glutMainLoop();

    exit(EXIT_SUCCESS);
}
