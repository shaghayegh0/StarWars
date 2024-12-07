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

// mesh
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
std::vector<GLfloat> enemyVertices;
std::vector<GLuint> enemyIndices;




bool explosionTriggered = false;
float explosionTime = 0.0f;
bool explosionComplete = false;  // Tracks if the explosion has finished
float explosionDuration = 2.0f;  // Explosion lasts for 2 seconds
std::vector<std::tuple<float, float, float>> explosionParticles; // Stores positions of particles



bool cannonBroken = false;    // Tracks if the cannon is broken
bool gameRunning = true;      // Tracks if the game is running

int currentPhase = 1;  // Track the current phase

// Calculate the camera position
float eyeX = 0.0;
float eyeY = 4.5f;
float eyeZ = 15.0f;

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

// Function prototype for keyboard handler
void keyboard(unsigned char key, int x, int y);

void updateDefensiveProjectiles();

// for camera rotation
void specialKeys(int key, int x, int y);

// for new phase
bool allBotsDeactivated();
void startNewPhase();



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
            enemyVertices.push_back(x);
            enemyVertices.push_back(y);
            enemyVertices.push_back(z);
        } else if (prefix == "f") {
            GLuint vIdx[4];
            int count = 0;
            while (iss >> vIdx[count]) {
                vIdx[count++]--;
            }
            if (count == 4) {
                enemyIndices.push_back(vIdx[0]);
                enemyIndices.push_back(vIdx[1]);
                enemyIndices.push_back(vIdx[2]);
                enemyIndices.push_back(vIdx[0]);
                enemyIndices.push_back(vIdx[2]);
                enemyIndices.push_back(vIdx[3]);
            }
        }
    }

    file.close();
    return true;
}

void drawCustomMesh() {
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_QUADS);
    for (size_t i = 0; i < enemyIndices.size(); i++) {
        int index = enemyIndices[i];
        glVertex3f(enemyVertices[index * 3], enemyVertices[index * 3 + 1], enemyVertices[index * 3 + 2]);
    }
    glEnd();

    glPopMatrix();
}




void handleMouseMotion(int x, int y) {
    static int lastX = x, lastY = y;
    int deltaX = x - lastX;
    int deltaY = y - lastY;
    lastX = x;
    lastY = y;

    // Invert the direction of rotation
    cannonYaw -= deltaX * 0.2f;  // Negate to reverse the yaw direction
    cannonPitch += deltaY * 0.2f;

    // Clamp pitch to avoid flipping
    if (cannonPitch > 45.0f) cannonPitch = 45.0f;
    if (cannonPitch < -45.0f) cannonPitch = -45.0f;

    glutPostRedisplay();
}






// Function to draw the ground
void drawGround() {
    glPushMatrix();
    glColor3f(0.6f, 0.8f, 0.6f);  // Light green for the ground
    glTranslatef(0.0f, -4.0f, 0.0f);  // Position the ground
    glScalef(35.0f, 0.0f, 35.0f);  // Scale the ground to a large plane
    glutSolidCube(1.0);  // Use a scaled cube as the ground
    glPopMatrix();
}

void drawDefensiveCannon() {
    
    // If the explosion is complete, do nothing
    if (explosionComplete) {
        return;
    }
    
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
    
    // Broken state animation: tilt and color change
    if (cannonBroken) {
            // Trigger explosion once
            if (!explosionTriggered) {
                explosionTriggered = true;
                explosionTime = 0.0f;
                
                // Generate random explosion particles
                for (int i = 0; i < 50; ++i) {
                    float offsetX = (rand() % 100 - 50) / 50.0f;
                    float offsetY = (rand() % 100 - 50) / 50.0f;
                    float offsetZ = (rand() % 100 - 50) / 50.0f;
                    explosionParticles.push_back({offsetX, offsetY, offsetZ});
                }
            }

            // Draw explosion particles
            glColor3f(0.0f, 0.0f, 0.0f);
            for (const auto& particle : explosionParticles) {
                glPushMatrix();
                glTranslatef(std::get<0>(particle) * explosionTime,
                             std::get<1>(particle) * explosionTime,
                             std::get<2>(particle) * explosionTime);
                glutSolidSphere(0.1, 10, 10);
                glPopMatrix();
            }
        
            // Increment explosion time
            explosionTime += 0.05f;
            if (explosionTime > explosionDuration) {
                explosionComplete = true;  // Mark explosion as complete
            }
        } else {
            // Draw the normal cannon if not broken
            glPushMatrix();
            drawCustomMesh();
            glPopMatrix();
        }
    


        glPopMatrix();
}

void updateExplosion(int value) {
    if (explosionTriggered) {
        explosionTime += 0.1f; // Increment explosion time

        if (explosionTime > explosionDuration) {
            explosionTriggered = false;
            cannonBroken = false;
            explosionParticles.clear();
        }

        glutPostRedisplay(); // Redraw the scene
        glutTimerFunc(100, updateExplosion, 0); // Continue the timer
    }
}




void fireDefensiveProjectile() {
    
    if (cannonBroken) return;  // Prevent firing if the cannon is broken

    
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
        true,                       // Active
        ProjectileSource::DEFENSIVE  // Mark as a defensive projectile

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
    gluLookAt(eyeX, eyeY, eyeZ,  // Camera position
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
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);                    // Enable at least one light source
    glEnable(GL_DEPTH_TEST);                // Enable depth testing
    glShadeModel(GL_SMOOTH);                // Smooth shading
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);


    // Configure light properties
    GLfloat lightPos[] = { 0.0f, 10.0f, 10.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    if (!loadMesh("mesh_data.txt")) {
        printf("Error loading mesh\n");
    }
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
    
    // Register the special keys handler
    glutSpecialFunc(specialKeys);




    glutMainLoop();
    return 0;
}



void timerFunc(int value) {
    if (animatingWalk) {
        for (auto& bot : bots) {
            bot.startWalking();
            
            // **Condition to deactivate the bot when it reaches the cannon's x position**
            if (bot.getZ() >= 12.0f) {
                bot.deactivate(); // Deactivate the bot
            }
            
            
            
            if (std::rand() % 20 == 0) { // Random chance to fire a projectile
                float cameraX = 0.0f, cameraY = 4.0f, cameraZ = 15.0f;
                bot.fireProjectile(cameraX, cameraY, cameraZ);
            }
            bot.updateProjectiles(); // Update projectiles for each bot
        }

        updateDefensiveProjectiles(); // Update defensive projectiles
        
        // Check if all bots are deactivated
        if (allBotsDeactivated()) {
            animatingWalk = false;  // Stop the current animation
            startNewPhase();        // Start a new phase
        }

        glutPostRedisplay(); // Redraw the scene
        glutTimerFunc(100, timerFunc, 0); // Schedule the next call
    }
}


bool allBotsDeactivated() {
    for (const auto& bot : bots) {
        if (bot.isActive()) {
            return false;
        }
    }
    return true;
}


void startNewPhase() {
    currentPhase++;
    printf("Starting Phase %d\n", currentPhase);

    bots.clear();  // Remove old bots

    // Add new bots for the next phase with different positions
    bots = {
        Enemy(8.0f, 0.0f, -8.0f),
        Enemy(2.0f, 0.0f, -14.0f),
        Enemy(-3.0f, 0.0f, 3.0f)
    };

    animatingWalk = true;  // Restart animation
    glutTimerFunc(0, timerFunc, 0);  // Restart the timer
}


void restartGame() {
    printf("Game restarted!\n");
    
    cannonBroken = false;
    explosionTriggered = false; // Reset explosion state if applicable
    explosionComplete = false;    // Reset explosion completion state
    explosionParticles.clear(); // Clear explosion particles if applicable
    
    currentPhase = 1;
    bots.clear();
    bots = {
        Enemy(5.0f, 0.0f, -10.0f),
        Enemy(0.0f, 0.0f, -15.0f),
        Enemy(-5.0f, 0.0f, -5.0f)
    };
    defensiveProjectiles.clear();
    animatingWalk = true;
    
    glutTimerFunc(0, timerFunc, 0);
    glutPostRedisplay();
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
            
            
        case 'r': // Restart the game
              restartGame();
              break;
            
        default:
            break;
    }

    glutPostRedisplay(); // Request a redraw to reflect any changes
}



void specialKeys(int key, int x, int y) {
    const float angleIncrement = 2.0f; // Adjust the degree of rotation

    switch (key) {
        case GLUT_KEY_LEFT:
            eyeX -= angleIncrement;
            break;
        case GLUT_KEY_RIGHT:
            eyeX += angleIncrement;
            break;
        case GLUT_KEY_UP:
            eyeY += angleIncrement;
            
            break;
        case GLUT_KEY_DOWN:
            eyeY -= angleIncrement;
            
            break;
    }

    glutPostRedisplay(); // Redraw the scene with updated camera angles
}

