#ifndef ENEMY_H
#define ENEMY_H

#include <vector>

extern void updateExplosion(int value);


// Enum to differentiate projectile sources
enum class ProjectileSource {
    ENEMY,
    DEFENSIVE
};

// Updated Projectile struct with the source field
struct Projectile {
    float posX, posY, posZ;
    float velocityX, velocityY, velocityZ;
    bool active; // Is the projectile still in motion?
    ProjectileSource source; // New field to track the source of the projectile
};

class Enemy {
public:
    // Constructor
    Enemy(float posX, float posY, float posZ);
    
    // Getter functions for position
    float getX() const { return positionX; }
    float getY() const { return positionY; }
    float getZ() const { return positionZ; }
    
    bool isActive() const { return active; }
    void deactivate() { active = false; }

    // Draw the bot
    void draw();

    // Animation functions
    void updateCannonRotation();
    void startWalking();
    
    
    void fireProjectile(float cameraX, float cameraY, float cameraZ); // Fire a projectile
    void updateProjectiles(); // Update projectile positions

    
    static void timerCallback(int value); // Static function for GLUT timer
    static void setActiveEnemy(Enemy* enemy); // Declaration for setActiveEnemy
    static Enemy* activeEnemy; // Ensure this is correctly declared as static
    
    void setAnimating(bool value) { animating = value; }
    bool isAnimating() const { return animating; }



private:
    
    bool active; // Track if the robot is still active

    
    // Position
    float positionX, positionY, positionZ;

    // Joint angles
    float headRotation, cannonRotation;
    float leftArmAngle, rightArmAngle;
    float leftLegAngle, rightLegAngle;

    // Movement parameters
    float hipAngle, hipVerticalShift;

    // Animation flags
    bool cannonSpinning;
    bool animating;
    bool stepWithRightLeg;
    
    // List of active projectiles
    std::vector<Projectile> projectiles;

    
    // Draw active projectiles
    void drawProjectiles();


};

#endif

