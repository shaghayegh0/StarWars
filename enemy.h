#ifndef ENEMY_H
#define ENEMY_H




class Enemy {
public:
    // Constructor
    Enemy(float posX, float posY, float posZ);

    // Draw the bot
    void draw();

    // Animation functions
    void updateCannonRotation();
    void startWalking();
    
    
    static void timerCallback(int value); // Static function for GLUT timer
    static void setActiveEnemy(Enemy* enemy); // Declaration for setActiveEnemy
    static Enemy* activeEnemy; // Ensure this is correctly declared as static
    
    void setAnimating(bool value) { animating = value; }
    bool isAnimating() const { return animating; }



private:
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

    // Internal rendering functions
    void drawBody();
    void drawHead();
    void drawArms();
    void drawLegs();
    
    

};

#endif

