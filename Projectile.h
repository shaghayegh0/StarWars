#ifndef PROJECTILE_H
#define PROJECTILE_H

enum class ProjectileSource {
    ENEMY,
    DEFENSIVE
};

struct Projectile {
    float posX;
    float posY;
    float posZ;
    float velocityX;
    float velocityY;
    float velocityZ;
    bool active;
    ProjectileSource source;  // New field to track the source
};

#endif // PROJECTILE_H

