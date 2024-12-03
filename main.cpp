#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include "enemy.h"
#include <cstdio> // For printf

#include <thread>
#include <chrono>

#include "QuadMesh.h"
#include "VECTOR3D.h"
#include <vector> // For std::vector




// List to store all Enemy objects
std::vector<Enemy> bots = {
    Enemy(5.0f, 0.0f, -10.0f),
    Enemy(0.0f, 0.0f, -10.0f),
    Enemy(-5.0f, 0.0f, -10.0f)
};

bool animatingWalk = true; // Global flag for animation - walking



// Function prototype for keyboard handler
void keyboard(unsigned char key, int x, int y);

// Function to draw the ground
void drawGround() {
    glPushMatrix();
    glColor3f(0.6f, 0.8f, 0.6f);  // Light green for the ground
    glTranslatef(0.0f, -2.5f, 0.0f);  // Position the ground
    glScalef(20.0f, 0.1f, 20.0f);  // Scale the ground to a large plane
    glutSolidCube(1.0);  // Use a scaled cube as the ground
    glPopMatrix();
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set the camera view
    gluLookAt(0.0, 5.0, 15.0,  // Camera position
              0.0, 0.0, 0.0,   // Look-at point
              0.0, 1.0, 0.0);  // Up direction

    // Draw the ground
    drawGround();

    // Draw all the bots
        for (auto& bot : bots) {
            bot.draw();
        }

    glutSwapBuffers();
}

// Initialization function
void initOpenGL() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Light blue sky background
    glEnable(GL_DEPTH_TEST);               // Enable depth testing
    glShadeModel(GL_SMOOTH);               // Smooth shading
}

// Reshape function
void reshape(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Bots with Ground");

    initOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard); // Register the keyboard handler


    glutMainLoop();
    return 0;
}



void timerFunc(int value) {
    if (animatingWalk) {
        for (auto& bot : bots) {
            bot.startWalking();
        }
        glutPostRedisplay(); // Redraw the scene
        glutTimerFunc(100, timerFunc, 0); // Schedule the next call
    }
}



void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // Start walking
            animatingWalk = true;
            glutTimerFunc(0, timerFunc, 0); // Start the timer
            break;

        case 'W': // Stop walking
            animatingWalk = false; // Stop the timer
            break;

        default:
            break;
    }

    glutPostRedisplay(); // Request a redraw to reflect any changes
}


