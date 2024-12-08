//MESHVIEWER FINAL VERSION
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <windows.h>
#include <gl/glut.h>1
#endif
#include <math.h>
#include <stdio.h>

// Global variables
std::vector<GLfloat> vertices;
std::vector<GLfloat> normals;
std::vector<GLuint> indices;

GLuint vao, vboVertices, vboNormals, ebo;

GLfloat rotationX = 0.0f, rotationY = 0.0f;
int lastMouseX = 0, lastMouseY = 0;
bool isDragging = false;

// Lighting and material properties
GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light_diffuse[] = {0.8, 0.8, 0.8, 1.0};
GLfloat light_position[] = {0.0, 8.0, 0.0, 1.0};

GLfloat mat_ambient[] = {0.3, 0.3, 0.3, 1.0};
GLfloat mat_diffuse[] = {0.6, 0.6, 0.8, 1.0};
GLfloat mat_specular[] = {0.9, 0.9, 0.9, 1.0};
GLfloat mat_shininess[] = {32.0};

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}

void mouseMotion(int x, int y) {
    if (isDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        rotationX += dy * 0.5f;
        rotationY += dx * 0.5f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

// Function to load a mesh from the TXT file
bool loadMesh(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            GLfloat x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        } else if (prefix == "vn") {
            GLfloat nx, ny, nz;
            iss >> nx >> ny >> nz;
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);
        } else if (prefix == "f") {
            GLuint vIdx[4];
            int count = 0;
            while (iss >> vIdx[count]) {
                vIdx[count++]--;
            }
            if (count == 3) {
                indices.push_back(vIdx[0]);
                indices.push_back(vIdx[1]);
                indices.push_back(vIdx[2]);
            } else if (count == 4) {
                indices.push_back(vIdx[0]);
                indices.push_back(vIdx[1]);
                indices.push_back(vIdx[2]);
                indices.push_back(vIdx[0]);
                indices.push_back(vIdx[2]);
                indices.push_back(vIdx[3]);
            }
        }
    }

    file.close();
    return true;
}

void initOpenGL() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Light 0
    glEnable(GL_LIGHT1); // Light 1
    glEnable(GL_LIGHT2); // Light 2
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // Material properties
    GLfloat mat_ambient[] = {0.2, 0.1, 0.1, 1.0}; // Warm reddish tones
    GLfloat mat_diffuse[] = {0.3, 0.5, 0.9, 1.0}; // Bright blue tone
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0}; // Sharp white highlights
    GLfloat mat_shininess[] = {64.0}; // Increased shininess for a sharper reflection

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // Light 0 (above)
    GLfloat light0_position[] = {0.0, 8.0, 0.0, 1.0};
    GLfloat light0_diffuse[] = {0.6, 0.2, 0.8, 1.0}; // Slightly bluish light
    GLfloat light0_specular[] = {1.0, 1.0, 1.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Light 1 (diagonal from front-left)
    GLfloat light1_position[] = {-5.0, 5.0, 5.0, 1.0};
    GLfloat light1_diffuse[] = {0.4, 0.4, 0.6, 1.0}; // Cool, soft lighting
    GLfloat light1_specular[] = {0.6, 0.6, 0.6, 1.0};

    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    // Light 2 (diagonal from back-right)
    GLfloat light2_position[] = {5.0, 5.0, -5.0, 1.0};
    GLfloat light2_diffuse[] = {0.8, 0.5, 0.4, 1.0}; // Warm light for contrast
    GLfloat light2_specular[] = {0.8, 0.8, 0.8, 1.0};

    glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);

    // Background color
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Cool blue-gray background


    glGenVertexArraysAPPLE(1, &vao);
    glGenBuffers(1, &vboVertices);
    glGenBuffers(1, &vboNormals);
    glGenBuffers(1, &ebo);

    glBindVertexArrayAPPLE(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArrayAPPLE(0);

}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 0.0, -20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    glBindVertexArrayAPPLE(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArrayAPPLE(0);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)width / (GLdouble)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_txt_file>" << std::endl;
        return -1;
    }

    if (!loadMesh(argv[1])) {
        return -1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Mesh Viewer");

    initOpenGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();

    return 0;
}

