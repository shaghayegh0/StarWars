#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <thread>
#include <chrono>
#include <cstdlib>


#include "QuadMesh.h"
#include "VECTOR3D.h"
#include "enemy.h"



Enemy* Enemy::activeEnemy = nullptr;


// Initial angles for joint rotations
float leftArmAngle = 0.0f;
float rightArmAngle = 0.0f;
float cannonRotation = 0.0f;
float headRotation = 0.0f;   // New variable for head rotation

float hipAngle = 0.0f;  // Angle for the hip joint movement
float hipVerticalShift = 0.0f;  // Vertical shift for hip during walking

float hipShift = 0.0f; // Controls hip vertical movement
float leftLegAngle = 0.0f, rightLegAngle = 0.0f;
float leftKneeAngle = 180.0f, rightKneeAngle = 180.0f;
float hipIncrement = 0.5f; // Hip vertical shift increment
float legIncrement = 5.0f; // Leg rotation increment

float botPositionX = 0.0f;
float botPositionY = 0.0f;
float botPositionZ = 0.0f;
float stepSize = 0.1f;

bool cannonSpinning = false;  // Flag to track if the cannon is spinning
bool leftStep = true;  // Track which leg is stepping (true = left leg, false = right leg)
bool animating = true; // Toggle animation
bool stepWithRightLeg = true;  // Flag to alternate between legs



extern bool cannonBroken;
extern GLuint texture;




// Constructor to initialize the bot's attributes
Enemy::Enemy(float posX, float posY, float posZ)
    : positionX(posX), positionY(posY), positionZ(posZ),
      headRotation(0.0f), cannonRotation(0.0f), 
      leftArmAngle(0.0f), rightArmAngle(0.0f), 
      leftLegAngle(0.0f), rightLegAngle(0.0f), 
      hipAngle(0.0f), hipVerticalShift(0.0f), 
      cannonSpinning(false), animating(false),
    stepWithRightLeg(true), active(true) {}



void Enemy::fireProjectile(float cameraX, float cameraY, float cameraZ) {
    // Calculate direction vector from the projectile's position to the camera
    float directionX = cameraX - positionX;
    float directionY = cameraY - (positionY + 2.5f); // Adjust for cannon height
    float directionZ = cameraZ - positionZ;

    // Normalize the direction vector
    float magnitude = std::sqrt(directionX * directionX + directionY * directionY + directionZ * directionZ);
    directionX /= magnitude;
    directionY /= magnitude;
    directionZ /= magnitude;

    // Add random accuracy offset
    float accuracyX = ((std::rand() % 100) / 100.0f) * 0.2f - 0.05f; // Small random offset for X
    float accuracyY = ((std::rand() % 100) / 100.0f) * 0.2f - 0.05f; // Small random offset for Y
    float accuracyZ = ((std::rand() % 100) / 100.0f) * 0.2f - 0.05f; // Small random offset for Z

    // Add the projectile to the list with randomized accuracy
    projectiles.push_back({
        positionX,
        positionY + 2.5f, // Launch from the cannon height
        positionZ,
        directionX + accuracyX, // Adjust velocity by random offset
        directionY + accuracyY,
        directionZ + accuracyZ,
        true,
        ProjectileSource::ENEMY  // Mark as an enemy projectile

    });
}


void Enemy::updateProjectiles() {
    for (auto& proj : projectiles) {
        if (proj.active) {
            proj.posX += proj.velocityX; // Move along X-axis
            proj.posY += proj.velocityY; // Move along Y-axis
            proj.posZ += proj.velocityZ; // Move along Z-axis

            // Deactivate the projectile if it moves too far
            if (std::abs(proj.posX) > 50.0f || std::abs(proj.posY) > 50.0f || std::abs(proj.posZ) > 50.0f) {
                proj.active = false;
            }
            
            // Check for collision with defensive cannon
            if (!cannonBroken && proj.source == ProjectileSource::ENEMY &&
                fabs(proj.posX) < 0.5f &&           // Close to x = 0.0f
                fabs(proj.posY - 4.0f) < 0.5f &&    // Close to y = 4.0f
                fabs(proj.posZ - 15.0f) < 0.5f) {   // Close to z = 15.0f

                cannonBroken = true;                // Set cannon to broken state
                proj.active = false;                // Deactivate the projectile
                printf("Cannon is broken!\n");
                printf("Projectile Location: (X: %.2f, Y: %.2f, Z: %.2f)\n", proj.posX, proj.posY, proj.posZ);
                
                glutTimerFunc(0, updateExplosion, 0);


            }
        }
    }
    
    

    // Remove inactive projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
                       [](const Projectile& p) { return !p.active; }),
        projectiles.end());
}


void Enemy::drawProjectiles() {
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for projectiles
    for (const auto& proj : projectiles) {
        if (proj.active) {
            glPushMatrix();
            glTranslatef(proj.posX, proj.posY, proj.posZ);
            glScalef(1.0f, 0.5f, 1.5f); // Adjust scaling factors for the oval shape
            glutSolidSphere(0.1, 10, 10); // Small spheres as projectiles
            glPopMatrix();
        }
    }
}


// Function to draw the bot
void Enemy::draw() {
    
    if (!active) return; // Do not draw if the robot is inactive
    
    // Bind the texture if it is valid
    if (bodyTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, bodyTexture);
    }


    // Set body color
    glColor3f(1.0, 1.0, 1.0);

    // Draw the body (an uneven trapezoidal body)
    glPushMatrix();
        glTranslatef(positionX, positionY, positionZ);
        // Use instance variables for position

    
    // Use GL_QUADS to draw the trapezoid-like body
    glBegin(GL_QUADS);

    // Front face (trapezoidal)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8, -0.5, 0.5);  // Bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8, -0.5, 0.5);   // Bottom-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, 0.5);    // Top-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5, 0.5, 0.5);   // Top-left

    // Back face (trapezoidal)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8, -0.5, -0.5); // Bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8, -0.5, -0.5);  // Bottom-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);   // Top-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);  // Top-left

    // Left face (rectangular)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8, -0.5, 0.5);  // Front-bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.8, -0.5, -0.5); // Back-bottom-left
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);  // Back-top-left
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5, 0.5, 0.5);   // Front-top-left

    // Right face (rectangular)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.8, -0.5, 0.5);   // Front-bottom-right
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8, -0.5, -0.5);  // Back-bottom-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);   // Back-top-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5, 0.5, 0.5);    // Front-top-right

    // Top face (smaller rectangle)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5, 0.5, 0.5);   // Front-top-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5, 0.5, 0.5);    // Front-top-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, 0.5, -0.5);   // Back-top-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5, 0.5, -0.5);  // Back-top-left

    // Bottom face (larger rectangle)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8, -0.5, 0.5);  // Front-bottom-left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8, -0.5, 0.5);   // Front-bottom-right
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8, -0.5, -0.5);  // Back-bottom-right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8, -0.5, -0.5); // Back-bottom-left

    glEnd(); // End of body
    glDisable(GL_TEXTURE_2D);


        // Draw the neck (a small cylinder)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, bodyTexture);

        glPushMatrix();
            glTranslatef(0.0, 1.0, 0.0);  // Position the neck above the body
            glRotatef(90, 1.0, 0.0, 0.0);
            GLUquadric* neck = gluNewQuadric();
            gluCylinder(neck, 0.05, 0.1, 0.25, 20, 20);  // Small neck cylinder
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

        // Draw the head (a larger sphere) with rotation
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bodyTexture);

    // Create a quadric object for the sphere
    GLUquadric* quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE); // Enable texture coordinates for the quadric

        glPushMatrix();
            glTranslatef(0.0, 1.75, 0.0);  // Move the head above the neck
            glRotatef(headRotation, 0, 1, 0);  // Rotate the head around the neck
            glScalef(1.0, 1.1, 1.0);
            gluSphere(quadric, 0.75, 20, 20); // Radius 0.75, slices 20, stacks 20

            // Draw the equator-like line (cut sphere in half)
            glColor3f(0.0, 0.0, 0.0);  // Set color for the equator line (black)
            int num_segments = 100;  // Number of segments to draw a smooth circle
            float radius = 0.75;  // Radius of the head sphere (same as the glutSolidSphere)
        
            glBegin(GL_LINE_LOOP);  // Start drawing the equator line
            for (int i = 0; i < num_segments; i++) {
                float theta = 1.0f * 3.1415926f * float(i) / float(num_segments);  // Angle calculation
                float x = radius * cosf(theta);  // X coordinate (cosine of the angle)
                float z = radius * sinf(theta);  // Z coordinate (sine of the angle)
                glVertex3f(x, -0.1, z);  // Set the vertex along the equator
            }
            glEnd();  // End drawing the line
    // Clean up the quadric object
    gluDeleteQuadric(quadric);

    glDisable(GL_TEXTURE_2D);
// Disable texture mapping


// Enable texture mapping for the ears
    if (bodyTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, bodyTexture);
    }

    // Create a quadric object for the ears and cylinders
    GLUquadric* earQuadric = gluNewQuadric();
    gluQuadricTexture(earQuadric, GL_TRUE); // Enable texture coordinates for the ears

    // Draw the left ear
    glPushMatrix();
        glTranslatef(-0.75, -0.1, 0.0);  // Left ear position
        gluSphere(earQuadric, 0.12, 20, 20); // Draw left ear with texture

        // Draw a cylinder around the left ear
        glPushMatrix();
            glTranslatef(-0.08, 0.0, 0.0); // Move the cylinder to wrap around the ear
            glRotatef(90, 0.0, 1.0, 0.0);  // Rotate to align the cylinder with the ear
            gluCylinder(earQuadric, 0.12, 0.12, 0.15, 20, 20); // Draw the cylinder with texture
        glPopMatrix();
    glPopMatrix();

    // Draw the right ear
    glPushMatrix();
        glTranslatef(0.75, -0.1, 0.0);  // Right ear position
        gluSphere(earQuadric, 0.12, 20, 20); // Draw right ear with texture

        // Draw a cylinder around the right ear
        glPushMatrix();
            glTranslatef(-0.08, 0.0, 0.0); // Move the cylinder to wrap around the ear
            glRotatef(90, 0.0, 1.0, 0.0);  // Rotate to align the cylinder with the ear
            gluCylinder(earQuadric, 0.12, 0.12, 0.15, 20, 20); // Draw the cylinder with texture
        glPopMatrix();
    glPopMatrix();

    // Clean up the quadric object
    gluDeleteQuadric(earQuadric);

    glDisable(GL_TEXTURE_2D); // Disable texture mapping

    
// body and head texture done!

// eye texture //
    
    // Bind the eye texture if it is valid
    if (eyeTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, eyeTexture);
    }

    // Create a quadric object for the eyes
    GLUquadric* eyeQuadric = gluNewQuadric();
    gluQuadricTexture(eyeQuadric, GL_TRUE); // Enable texture coordinates for the quadric

    // Draw the left eye with texture
    glPushMatrix();
        glTranslatef(-0.35, 0.3, 0.7);  // Left eye position in front of the head
        glColor3f(1.0, 1.0, 1.0);       // Set color to white (to avoid color affecting the texture)
        gluSphere(eyeQuadric, 0.2, 20, 20); // Draw left eye with texture
    glPopMatrix();

    // Draw the right eye with texture
    glPushMatrix();
        glTranslatef(0.35, 0.3, 0.7);   // Right eye position in front of the head
        glColor3f(1.0, 1.0, 1.0);       // Set color to white (to avoid color affecting the texture)
        gluSphere(eyeQuadric, 0.2, 20, 20); // Draw right eye with texture
    glPopMatrix();

    // Clean up the quadric object
    gluDeleteQuadric(eyeQuadric);

    glDisable(GL_TEXTURE_2D); // Disable texture mapping

    
// eye texture //


        
            // Draw the antena
            glColor3f(0.0, 0.0, 0.0);  // Set color to black
            glPushMatrix();
                glTranslatef(0.0, 1.0, 0.0);  // Place the cannon directly on top of the head
                glRotatef(90, 1, 0, 0);  // Rotate 90 degrees around the x-axis to make it vertical
                GLUquadric* antena = gluNewQuadric();
                gluCylinder(antena, 0.08, 0.1, 0.3, 20, 20);  // Draw the cannon cylinder

    
    
                // Draw the cannon (cylinder on top of the head with rotation)
                    glColor3f(1.0, 1.0, 0.0);  // Set color to yellow
                    glPushMatrix();
                        glTranslatef(0.0, 0.2, 0.1);  // Place the cannon directly on top of the head
                        glRotatef(90, 1, 0, 0);  // Rotate 90 degrees around the x-axis to make it vertical
                        glRotatef(cannonRotation, 0, 0, 1);  // Apply the rotation around the z-axis for spinning
                        GLUquadric* cannon = gluNewQuadric();
                        gluCylinder(cannon, 0.05, 0.05, 0.2, 20, 20);  // Draw the cannon cylinder

                    // Add small sub-part to the cannon (small sphere)
                    glColor3f(1.0, 0.0, 0.0);  // Set color to red for sub-part
                        glPushMatrix();
                            glTranslatef(0.05, 0.05, 0.0);  // Position a small sphere on the side of the cannon
                            glutSolidSphere(0.05, 10, 10);  // Draw a small sphere
                        glPopMatrix();
    
                    glPopMatrix();  // End cannon
            glPopMatrix(); // End antena
        glPopMatrix();  // End head

    
    
// Enable arm texture
    // Draw left arm
          glPushMatrix();
              glTranslatef(-0.65, 0.6, 0.0); // Position on the left side

          // Shoulder joint
              glPushMatrix();
                  glColor3f(0.0, 0.0, 0.0);  // Set color to black for the shoulder joint
                  glutSolidSphere(0.2, 20, 20); // Shoulder joint sphere
              glPopMatrix();

          // Rotate arm outward for "V" shape
              glRotatef(-30, 0, 0, 1);

              glRotatef(leftArmAngle, 1, 0, 0); // Rotate arm at shoulder
              glTranslatef(0.0, -0.5, 0.0); // Move down for upper arm

          // Upper arm
              glPushMatrix();
                  glColor3f(0.0, 0.0, 0.0);  // Set color to black for the upper arm
                  glScalef(0.2, 0.8, 0.2);  // Thin and long upper arm
                  glutSolidCube(1.0);
              glPopMatrix();

          // Elbow joint
              glPushMatrix();
                  glTranslatef(0.0, -0.5, 0.0); // Move to elbow position
                  glColor3f(0.5, 0.5, 0.5);  // Set color to black for the elbow joint
                  glutSolidSphere(0.15, 20, 20); // Elbow joint sphere
              glPopMatrix();

          // Lower arm
              glPushMatrix();
                  glTranslatef(0.0, -1.0, 0.0);  // Move down to lower arm position
                  glColor3f(0.0, 0.0, 0.0); // Set color to black for the lower arm
                  glScalef(0.2, 0.8, 0.2);  // Thin lower arm
                  glutSolidCube(1.0);
              glPopMatrix();
    
// disable arm texture


       
// palm texture
    if (handTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, handTexture);
    }

    // Create a quadric object for the palm
    GLUquadric* handQuadric = gluNewQuadric();
    gluQuadricTexture(handQuadric, GL_TRUE); // Enable texture coordinates for the quadric

    // Draw the palm with texture
    glPushMatrix();
        glTranslatef(0.0, -1.5, 0.0);  // Move to the palm position
        gluSphere(handQuadric, 0.25, 20, 20);  // Draw the palm sphere with texture
    glPopMatrix();

    // Clean up the quadric object
    gluDeleteQuadric(handQuadric);

    glDisable(GL_TEXTURE_2D); // Disable texture mapping

// palm texture

        glPopMatrix();

        // Draw right arm
        glPushMatrix();
            glTranslatef(0.65, 0.6, 0.0); // Position on the right side

        // Shoulder joint
            glPushMatrix();
                glColor3f(0.0, 0.0, 0.0);  // Set color to black for the shoulder joint
                glutSolidSphere(0.2, 20, 20); // Shoulder joint sphere
            glPopMatrix();

        // Rotate arm outward for "V" shape
            glRotatef(30, 0, 0, 1); // Rotate around z-axis for a "V" angle

            glRotatef(rightArmAngle, 1, 0, 0); // Rotate arm at shoulder
            glTranslatef(0.0, -0.5, 0.0); // Move down for upper arm

        // Upper arm
            glPushMatrix();
                glColor3f(0.0, 0.0, 0.0);  // Set color to black for the upper arm
                glScalef(0.2, 0.8, 0.2);  // Thin and long upper arm
                glutSolidCube(1.0);
            glPopMatrix();

        // Elbow joint
            glPushMatrix();
                glTranslatef(0.0, -0.5, 0.0); // Move to elbow position
                glColor3f(0.5, 0.5, 0.5);  // Set color to gray for the elbow joint
                glutSolidSphere(0.15, 20, 20); // Elbow joint sphere
            glPopMatrix();

        // Lower arm
            glPushMatrix();
                glTranslatef(0.0, -1.0, 0.0);  // Move down to lower arm position
                glColor3f(0.0, 0.0, 0.0); // Set color to black for the lower arm
                glScalef(0.2, 0.8, 0.2);  // Thin lower arm
                glutSolidCube(1.0);
            glPopMatrix();

// palm texture
    if (handTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, handTexture);
    }

    // Create a quadric object for the palm
    handQuadric = gluNewQuadric();
    gluQuadricTexture(handQuadric, GL_TRUE); // Enable texture coordinates for the quadric

    // Draw the palm with texture
    glPushMatrix();
        glTranslatef(0.0, -1.5, 0.0);  // Move to the palm position
        gluSphere(handQuadric, 0.25, 20, 20); // Draw the palm sphere with texture
    glPopMatrix();

    // Clean up the quadric object
    gluDeleteQuadric(handQuadric);

    glDisable(GL_TEXTURE_2D); // Disable texture mapping

// palm texture

        glPopMatrix();

        
        
        // Draw the hip joint (rectangle)
        glPushMatrix();
            // Apply vertical shift and hip rotation
            glTranslatef(0.0, -1 + hipVerticalShift, 0.0);  // Move hip under body, apply vertical shift
            glRotatef(hipAngle, 0, 1, 0);  // Rotate hip joint
            

            // ** Draw the legs before scaling the hip **
            // Left leg
            glPushMatrix();
                // Translate relative to the hip position
                glTranslatef(-0.3, -0.6, 0.0);  // Adjust leg translation
                glRotatef(leftLegAngle, 1, 0, 0); // Rotate leg at the hip joint
                glColor3f(0.0, 0.0, 0.0); // Black thigh
                glScalef(0.2, 1.0, 0.2);  // Scale for leg
                glutSolidCube(1.0);  // Thigh

                // Draw the knee and lower leg
                glPushMatrix();
                    glTranslatef(0.0, -0.6, 0.0); // Move to knee position
                    glColor3f(0.4, 0.4, 0.4); // Gray knee joint
                    glutSolidSphere(0.2, 20, 20); // Knee joint

                    // Lower leg
                    glPushMatrix();
                        glTranslatef(0.0, -0.6, 0.0); // Move down from knee
                        glRotatef(leftKneeAngle, 1, 0, 0); // Rotate at knee
                        glColor3f(0.0, 0.0, 0.0); // Black lower leg
                        glutSolidCube(1.0); // Lower leg
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();

            // Right leg
            glPushMatrix();
                glTranslatef(0.3, -0.6, 0.0);  // Adjust right leg translation
                glRotatef(rightLegAngle, 1, 0, 0); // Rotate leg at hip joint
                glColor3f(0.0, 0.0, 0.0); // Black thigh
                glScalef(0.2, 1.0, 0.2);  // Scale for leg
                glutSolidCube(1.0);  // Thigh

                // Knee and lower leg
                glPushMatrix();
                    glTranslatef(0.0, -0.6, 0.0); // Move to knee position
                    glColor3f(0.4, 0.4, 0.4); // Gray knee joint
                    glutSolidSphere(0.2, 20, 20); // Knee joint

                    // Lower leg
                    glPushMatrix();
                        glTranslatef(0.0, -0.6, 0.0); // Move down from knee
                        glRotatef(rightKneeAngle, 1, 0, 0); // Rotate at knee
                        glColor3f(0.0, 0.0, 0.0); // Black lower leg
                        glutSolidCube(1.0); // Lower leg
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();

            // Scale the hip at the end
            glColor3f(0.0, 0.0, 0.0); // Color for hip (reddish)
            glScalef(1.0, 0.2, 0.5);  // Scale to make hip rectangle
            glutSolidCube(1.0);       // Draw hip cube

        glPopMatrix();  // End of hip joint and leg assembly
    glPopMatrix(); // End of body
    
    
    drawProjectiles();

    
}



// Update cannon rotation
void Enemy::updateCannonRotation() {
    if (cannonSpinning) {
        cannonRotation += 5.0f;
        if (cannonRotation > 360.0f) cannonRotation -= 360.0f;
    }
}


// Timer callback
void Enemy::timerCallback(int value) {
    if (activeEnemy) {
        activeEnemy->startWalking(); // Delegate to the active enemy's startWalking
    }
}

// Define setActiveEnemy
void Enemy::setActiveEnemy(Enemy* enemy) {
    activeEnemy = enemy;
}



// Trigger walking animation
void Enemy::startWalking() {
    animating = true;
    if (animating) {
        if (stepWithRightLeg) {
            // Right leg moves
            if (rightLegAngle > -30.0f) {
                rightLegAngle -= legIncrement;
                rightKneeAngle -= legIncrement;
                if (rightKneeAngle < 120.0f) rightKneeAngle = 120.0f;
                hipVerticalShift += hipIncrement;
                if (hipVerticalShift > 0.3f) hipVerticalShift = 0.3f;
                // Update the bot's position (moving in the direction of hip rotation)
                float angleRadians = hipAngle * (M_PI / 180.0f);  // Convert hipAngle to radians
                botPositionX += stepSize * sin(angleRadians);  // Move along X based on hip rotation
                botPositionZ += stepSize * cos(angleRadians);  // Move along Z based on hip rotation
            } else if (rightLegAngle <= -30.0f && rightKneeAngle < 180.0f) {
                rightKneeAngle += legIncrement;
                if (rightKneeAngle > 180.0f) rightKneeAngle = 180.0f;
                hipVerticalShift -= hipIncrement;
                if (hipVerticalShift < 0.0f) hipVerticalShift = 0.0f;
            }

            if (rightLegAngle <= -30.0f && rightKneeAngle == 180.0f && hipVerticalShift == 0.0f) {
                stepWithRightLeg = false;  // Switch to left leg after right leg completes
                // Reset angles for the next cycle
                rightLegAngle = 0.0f;
                rightKneeAngle = 180.0f;
            }
        } else {
            // Left leg moves
            if (leftLegAngle > -30.0f) {
                leftLegAngle -= legIncrement;
                leftKneeAngle -= legIncrement;
                if (leftKneeAngle < 120.0f) leftKneeAngle = 120.0f;
                hipVerticalShift += hipIncrement;
                if (hipVerticalShift > 0.3f) hipVerticalShift = 0.3f;
                // Update the bot's position (moving in the direction of hip rotation)
                float angleRadians = hipAngle * (M_PI / 180.0f);  // Convert hipAngle to radians
                positionX += stepSize * sin(angleRadians);  // Move along X based on hip rotation
                positionZ += stepSize * cos(angleRadians);  // Move along Z based on hip rotation
            } else if (leftLegAngle <= -30.0f && leftKneeAngle < 180.0f) {
                leftKneeAngle += legIncrement;
                if (leftKneeAngle > 180.0f) leftKneeAngle = 180.0f;
                hipVerticalShift -= hipIncrement;
                if (hipVerticalShift < 0.0f) hipVerticalShift = 0.0f;
            }
            
            if (leftLegAngle <= -30.0f && leftKneeAngle == 180.0f && hipVerticalShift == 0.0f) {
                stepWithRightLeg = true;  // Switch to right leg after left leg completes
                // Reset angles for the next cycle
                leftLegAngle = 0.0f;
                leftKneeAngle = 180.0f;
            }
        }
        



        // Redraw the scene with updated leg and hip positions
        glutPostRedisplay();
        // Continue the walking animation every 100ms
        glutTimerFunc(100, Enemy::timerCallback, 0);
        // Continue walking, not stepForward
    }
}

void Enemy::setBodyTexture(GLuint tex) {
    bodyTexture = tex;
}

void Enemy::setEyeTexture(GLuint tex) {
    eyeTexture = tex;
}

void Enemy::setHandTexture(GLuint tex) {
    handTexture = tex;
}

void Enemy::setWheelTexture(GLuint tex) {
    wheelTexture = tex;
}
