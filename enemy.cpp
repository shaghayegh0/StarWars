#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <thread>
#include <chrono>

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






// Constructor to initialize the bot's attributes
Enemy::Enemy(float posX, float posY, float posZ)
    : positionX(posX), positionY(posY), positionZ(posZ),
      headRotation(0.0f), cannonRotation(0.0f), 
      leftArmAngle(0.0f), rightArmAngle(0.0f), 
      leftLegAngle(0.0f), rightLegAngle(0.0f), 
      hipAngle(0.0f), hipVerticalShift(0.0f), 
      cannonSpinning(false), animating(false), stepWithRightLeg(true) {}

// Function to draw the bot
void Enemy::draw() {
    // Set body color to metal gray
    glColor3f(0.5, 0.5, 0.5);  // Metallic gray

    // Draw the body (an uneven trapezoidal body)
    glPushMatrix();
        glTranslatef(positionX, positionY, positionZ);
        // Use instance variables for position
        // Use GL_QUADS to draw the trapezoid-like body
        glBegin(GL_QUADS);

        // Front face (trapezoidal)
        glVertex3f(-0.8, -0.5, 0.5);  // Bottom-left
        glVertex3f(0.8, -0.5, 0.5);   // Bottom-right
        glVertex3f(0.5, 0.5, 0.5);    // Top-right (smaller top width)
        glVertex3f(-0.5, 0.5, 0.5);   // Top-left (smaller top width)

        // Back face (trapezoidal)
        glVertex3f(-0.8, -0.5, -0.5);  // Bottom-left
        glVertex3f(0.8, -0.5, -0.5);   // Bottom-right
        glVertex3f(0.5, 0.5, -0.5);    // Top-right (smaller top width)
        glVertex3f(-0.5, 0.5, -0.5);   // Top-left (smaller top width)

        // Left face (rectangular)
        glVertex3f(-0.8, -0.5, 0.5);  // Front-bottom-left
        glVertex3f(-0.8, -0.5, -0.5); // Back-bottom-left
        glVertex3f(-0.5, 0.5, -0.5);  // Back-top-left
        glVertex3f(-0.5, 0.5, 0.5);   // Front-top-left

        // Right face (rectangular)
        glVertex3f(0.8, -0.5, 0.5);   // Front-bottom-right
        glVertex3f(0.8, -0.5, -0.5);  // Back-bottom-right
        glVertex3f(0.5, 0.5, -0.5);   // Back-top-right
        glVertex3f(0.5, 0.5, 0.5);    // Front-top-right

        // Top face (smaller rectangle)
        glVertex3f(-0.5, 0.5, 0.5);   // Front-top-left
        glVertex3f(0.5, 0.5, 0.5);    // Front-top-right
        glVertex3f(0.5, 0.5, -0.5);   // Back-top-right
        glVertex3f(-0.5, 0.5, -0.5);  // Back-top-left

        // Bottom face (larger rectangle)
        glVertex3f(-0.8, -0.5, 0.5);  // Front-bottom-left
        glVertex3f(0.8, -0.5, 0.5);   // Front-bottom-right
        glVertex3f(0.8, -0.5, -0.5);  // Back-bottom-right
        glVertex3f(-0.8, -0.5, -0.5); // Back-bottom-left

        glEnd(); //end of body
    

        // Draw the neck (a small cylinder)
        glPushMatrix();
            glTranslatef(0.0, 1.0, 0.0);  // Position the neck above the body
            glRotatef(90, 1.0, 0.0, 0.0);
            GLUquadric* neck = gluNewQuadric();
            gluCylinder(neck, 0.05, 0.1, 0.25, 20, 20);  // Small neck cylinder
        glPopMatrix();

        // Draw the head (a larger sphere) with rotation
        glPushMatrix();
            glTranslatef(0.0, 1.75, 0.0);  // Move the head above the neck
            glRotatef(headRotation, 0, 1, 0);  // Rotate the head around the neck
            glScalef(1.0, 1.1, 1.0);
            glutSolidSphere(0.75, 20, 20); // Draw a bigger sphere for the head
    
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

    
            // Draw the ears (two spheres at the ends of the equator) with surrounding cylinders
            glColor3f(0.5, 0.5, 0.5);  // Set color for the ears gray
            glPushMatrix();
                glTranslatef(-0.75, -0.1, 0.0);  // Left ear position
                glutSolidSphere(0.12, 20, 20);  // Draw left ear

                // Draw a cylinder around the left ear
                glColor3f(0.55, 0.55, 0.7);  // Set color for the cylinder
                glPushMatrix();
                glTranslatef(-0.08, 0.0, 0.0); // Move the cylinder to wrap around the ear
                    glRotatef(90, 0.0, 1.0, 0.0);  // Rotate to align the cylinder with the ear
                    GLUquadric* leftCylinder = gluNewQuadric();
                    gluCylinder(leftCylinder, 0.12, 0.12, 0.15, 20, 20);  // Draw the cylinder (same height as ear)
                glPopMatrix();
            glPopMatrix();

            glColor3f(0.5, 0.5, 0.5);
            glPushMatrix();
                glTranslatef(0.75, -0.1, 0.0);   // Right ear position
                glutSolidSphere(0.12, 20, 20);  // Draw right ear

                // Draw a cylinder around the right ear
                glColor3f(0.55, 0.55, 0.7); // Set color for the cylinder
                glPushMatrix();
                    glTranslatef(-0.08, 0.0, 0.0);  // Move the cylinder to wrap around the ear
                    glRotatef(90, 0.0, 1.0, 0.0);  // Rotate to align the cylinder with the ear
                    GLUquadric* rightCylinder = gluNewQuadric();
                    gluCylinder(rightCylinder, 0.12, 0.12, 0.15, 20, 20);  // Draw the cylinder (same height as ear)
                glPopMatrix();
            glPopMatrix();

    
    
            // Draw the eyes (two green spheres)
            glColor3f(0.0, 1.0, 0.0);  // Set color to green
                glPushMatrix();
                glTranslatef(-0.35, 0.3, 0.7);  // Left eye position in front of head
                glutSolidSphere(0.2, 20, 20);   // Draw left eye
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.35, 0.3, 0.7);   // Right eye position in front of head
                glutSolidSphere(0.2, 20, 20);   // Draw right eye
            glPopMatrix();

        
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

        // Palm
            glPushMatrix();
                glTranslatef(0.0, -1.5, 0.0);  // Move to the palm position
                glColor3f(0.4, 0.4, 0.4);  // Gray palm
                glutSolidSphere(0.25, 20, 20);  // Palm sphere
            glPopMatrix();

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

        // Palm
            glPushMatrix();
                glTranslatef(0.0, -1.5, 0.0);  // Move to the palm position
                glColor3f(0.4, 0.4, 0.4);  // Gray palm
                glutSolidSphere(0.25, 20, 20);  // Palm sphere
            glPopMatrix();

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
    
}

// Draw body
void Enemy::drawBody() {
    glColor3f(0.5, 0.5, 0.5);  // Metallic gray
    glPushMatrix();
    glScalef(1.0, 1.5, 1.0);  // Scale for body shape
    glutSolidCube(1.0);
    glPopMatrix();
}

// Draw head
void Enemy::drawHead() {
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);  // Position head above body
    glRotatef(headRotation, 0.0f, 1.0f, 0.0f);
    glutSolidSphere(0.5, 20, 20);
    glPopMatrix();
}

// Draw arms
void Enemy::drawArms() {
    // Left arm
    glPushMatrix();
    glTranslatef(-0.6f, 0.5f, 0.0f);  // Left shoulder position
    glRotatef(leftArmAngle, 1.0f, 0.0f, 0.0f);
    glutSolidCube(0.2);
    glPopMatrix();

    // Right arm
    glPushMatrix();
    glTranslatef(0.6f, 0.5f, 0.0f);  // Right shoulder position
    glRotatef(rightArmAngle, 1.0f, 0.0f, 0.0f);
    glutSolidCube(0.2);
    glPopMatrix();
}

// Draw legs
void Enemy::drawLegs() {
    // Left leg
    glPushMatrix();
    glTranslatef(-0.3f, -0.75f, 0.0f);  // Left hip position
    glRotatef(leftLegAngle, 1.0f, 0.0f, 0.0f);
    glutSolidCube(0.2);
    glPopMatrix();

    // Right leg
    glPushMatrix();
    glTranslatef(0.3f, -0.75f, 0.0f);  // Right hip position
    glRotatef(rightLegAngle, 1.0f, 0.0f, 0.0f);
    glutSolidCube(0.2);
    glPopMatrix();
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

