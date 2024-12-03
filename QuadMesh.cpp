#define GL_SILENCE_DEPRECATION
#include "QuadMesh.h"

QuadMesh::QuadMesh() {
    // Constructor: Can initialize anything related to the mesh here
}

// Function to setup lighting
void QuadMesh::setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  // Enable a light source
    
    // Define light properties
    GLfloat light_position[] = { 0.0, 5.0, 5.0, 1.0 }; // Light positioned above and in front
    GLfloat ambient_light[] = { 0.2, 0.2, 0.2, 1.0 };  // Ambient light
    GLfloat diffuse_light[] = { 0.8, 0.8, 0.8, 1.0 };  // Diffuse light
    GLfloat specular_light[] = { 1.0, 1.0, 1.0, 1.0 }; // Specular highlights
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light);
    
    // Enable shading model
    glShadeModel(GL_SMOOTH);

    glEnable(GL_COLOR_MATERIAL); //Enable glColor


}

// Function to draw a basic quad mesh (useful for shadow projections later)
void QuadMesh::drawQuadMesh() {
    // Set color for the ground (e.g., green)
    glColor3f(0.7f, 0.7f, 0.0f);  // Example color: green
        
    
    glBegin(GL_QUADS);
    // Define vertices of a quad (these could represent a ground plane)
    glNormal3f(0.0, 1.0, 0.0);  // Normal pointing up
    glVertex3f(-5.0, -5.0, -5.0);
    glVertex3f(5.0, -5.0, -5.0);
    glVertex3f(5.0, -5.0, 5.0);
    glVertex3f(-5.0, -5.0, 5.0);
    glEnd();
}

