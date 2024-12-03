#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>

class VECTOR3D {
public:
    float x, y, z;
    VECTOR3D(float xVal = 0.0, float yVal = 0.0, float zVal = 0.0) : x(xVal), y(yVal), z(zVal) {}

    // Cross product
    VECTOR3D cross(const VECTOR3D& v) const {
        return VECTOR3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    // Normalize vector
    VECTOR3D normalize() const {
        float length = sqrt(x * x + y * y + z * z);
        return VECTOR3D(x / length, y / length, z / length);
    }
};

#endif

