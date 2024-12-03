#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include "enemy.h"
#include <cstdio> // For printf

#include <thread>
#include <chrono>

#include "QuadMesh.h"
#include "VECTOR3D.h"
#include <vector> // For std::vector
#include <cstdlib> // For srand and rand



float cannonYaw = 0.0f;   // Left/Right rotation
float cannonPitch = 0.0f; // Up/Down rotation
float mouseX = 0.0f;      // Track last mouse X position
float mouseY = 0.0f;      // Track last mouse Y position
std::vector<Projectile> defensiveProjectiles; // Store projectiles fired from the cannon


// List to store all Enemy objects
std::vector<Enemy> bots = {
    Enemy(5.0f, 0.0f, -10.0f),
    Enemy(0.0f, 0.0f, -15.0f),
    Enemy(-5.0f, 0.0f, -5.0f)

};

bool animatingWalk = true; // Global flag for animation - walking

void updateDefensiveProjectiles();


void handleMouseMotion(int x, int y) {
    static int lastX = x, lastY = y;
    int deltaX = x - lastX;
    int deltaY = y - lastY;
    lastX = x;
    lastY = y;

    // Invert the direction of rotation
    cannonYaw -= deltaX * 0.2f;  // Negate to reverse the yaw direction
    cannonPitch += deltaY * 0.2f; // Negate to reverse the pitch direction

    // Clamp pitch to avoid flipping
    if (cannonPitch > 45.0f) cannonPitch = 45.0f;
    if (cannonPitch < -45.0f) cannonPitch = -45.0f;

    glutPostRedisplay();
}




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

void drawDefensiveCannon() {
    glPushMatrix();
    
    // Move to the camera position
    glTranslatef(0.0f, 4.0f, 15.0f);

    // Scale the cannon if needed for appropriate size
    glScalef(1.0f, 1.0f, 1.0f); // Increase size for better visibility
    
    // Rotate the cannon based on mouse movement
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f); // Rotate around the Y-axis
    glRotatef(cannonYaw, 0.0f, 1.0f, 0.0f);  // Yaw (left/right)
    glRotatef(cannonPitch, 1.0f, 0.0f, 0.0f); // Pitch (up/down)

    glEnable(GL_DEPTH_TEST);


    // Draw the main cannon cylinder
    glColor3f(0.5f, 0.5f, 0.5f);  // Gray color
    GLUquadric* cannonBase = gluNewQuadric();
    gluCylinder(cannonBase, 0.2, 0.2, 1.0, 20, 20);

    // Draw a sphere on top of the cannon
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.0f); // Move the sphere to the end of the cylinder
    glColor3f(1.0f, 0.0f, 0.0f);    // Red color for the sphere
    glutSolidSphere(0.3, 20, 20);
    glPopMatrix();

    // Draw a smaller cannon attachment
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.2f); // Move the attachment further
    glColor3f(0.7f, 0.7f, 0.7f);    // Lighter gray
    gluCylinder(cannonBase, 0.1, 0.1, 0.5, 20, 20);
    glPopMatrix();

    glPopMatrix();
}


void fireDefensiveProjectile() {
    
    // all directins are mirrored
    float directionX = -sin(cannonYaw * M_PI / 180.0f);
    float directionY = -sin(cannonPitch * M_PI / 180.0f);
    float directionZ = -cos(cannonYaw * M_PI / 180.0f);

    // Add a projectile with the cannon's direction
    defensiveProjectiles.push_back({
        0.0f, 4.0f, 15.0f,         // Start at the cannon's position
        directionX * 0.5f,         // Velocity in X
        directionY * 0.5f,         // Velocity in Y
        directionZ * 0.5f,         // Velocity in Z
        true                       // Active
    });
    // Immediately update projectiles to make them appear without delay
    updateDefensiveProjectiles();
}


void drawDefensiveProjectiles() {
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for defensive projectiles
    for (auto& proj : defensiveProjectiles) {
        if (proj.active) {
            glPushMatrix();
            glTranslatef(proj.posX, proj.posY, proj.posZ);
            glutSolidSphere(0.1, 10, 10); // Draw projectiles as small spheres
            glPopMatrix();
        }
    }
}



void updateDefensiveProjectiles() {
    for (auto& proj : defensiveProjectiles) {
        if (proj.active) {
            proj.posX += proj.velocityX;
            proj.posY += proj.velocityY;
            proj.posZ += proj.velocityZ;

            // Deactivate projectile if out of bounds
            if (proj.posZ < -50.0f || proj.posX < -50.0f || proj.posY < -50.0f ||
                proj.posZ > 50.0f || proj.posX > 50.0f || proj.posY > 50.0f) {
                proj.active = false;
            }

            // Check for collision with robots
            for (auto& bot : bots) {
                if (bot.isActive() && // Assuming isActive() exists in Enemy
                    fabs(bot.getX() - proj.posX) < 1.0f && // Check proximity
                    fabs(bot.getY() - proj.posY) < 1.0f &&
                    fabs(bot.getZ() - proj.posZ) < 1.0f) {
                    bot.deactivate(); // Deactivate the robot (implement in Enemy)
                    proj.active = false; // Deactivate the projectile
                    break; // Stop checking other robots
                }
            }
        }
    }

    // Remove inactive projectiles
    defensiveProjectiles.erase(
        std::remove_if(defensiveProjectiles.begin(), defensiveProjectiles.end(),
                       [](const Projectile& p) { return !p.active; }),
        defensiveProjectiles.end());
}



void mouseMotion(int x, int y) {
    static int lastX = x, lastY = y;
    int deltaX = x - lastX;
    int deltaY = y - lastY;
    lastX = x;
    lastY = y;

    cannonYaw += deltaX * 0.2f;  // Adjust sensitivity
    cannonPitch -= deltaY * 0.2f;

    // Clamp pitch to avoid flipping
    if (cannonPitch > 45.0f) cannonPitch = 45.0f;
    if (cannonPitch < -45.0f) cannonPitch = -45.0f;

    glutPostRedisplay();
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

    // Draw the defensive cannon
    drawDefensiveCannon();
    
    // Draw all the bots
    for (auto& bot : bots) {
        bot.draw();
    }

    // Draw defensive projectiles
    drawDefensiveProjectiles();

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
    glutCreateWindow("CPS511 - A3");

    initOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard); // Register the keyboard handler
    
    // Register mouse motion functions
    glutMotionFunc(handleMouseMotion);        // Active motion
    glutPassiveMotionFunc(handleMouseMotion); // Passive motion


    glutMainLoop();
    return 0;
}



void timerFunc(int value) {
    if (animatingWalk) {
        for (auto& bot : bots) {
            bot.startWalking();
            if (std::rand() % 20 == 0) { // Random chance to fire a projectile
                float cameraX = 0.0f, cameraY = 4.0f, cameraZ = 15.0f;
                bot.fireProjectile(cameraX, cameraY, cameraZ);
            }
            bot.updateProjectiles(); // Update projectiles for each bot
        }

        updateDefensiveProjectiles(); // Update defensive projectiles

        glutPostRedisplay(); // Redraw the scene
        glutTimerFunc(100, timerFunc, 0); // Schedule the next call
    }
}




void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case ' ': // Fire defensive cannon
            fireDefensiveProjectile();
            break;
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



