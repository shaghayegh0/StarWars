#define GL_SILENCE_DEPRECATION

#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
std::vector<GLfloat> enemyNormals;



// texture
GLuint shaderProgram;
GLuint VAO, VBO, EBO, texture;
// Declare textures globally
GLuint bodyTexture;
GLuint eyeTexture;
GLuint handTexture;
GLuint wheelTexture;
GLuint groundTexture;
GLuint skyTexture;


// Vertex data for a quad (positions and texture coordinates)
float vertices[] = {
    // Positions       // Texture Coords
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};



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
float eyeY = -2.5f;
float eyeZ = 18.0f;

float cannonYaw = 0.0f;   // Left/Right rotation
float cannonPitch = 0.0f; // Up/Down rotation
float mouseX = 0.0f;      // Track last mouse X position
float mouseY = 0.0f;      // Track last mouse Y position
std::vector<Projectile> defensiveProjectiles; // Store projectiles fired from the cannon


// List to store all Enemy objects
std::vector<Enemy> bots = {
    Enemy(5.0f, -0.5f, -10.0f),
    Enemy(0.0f, -0.5f, -15.0f),
    Enemy(-5.0f, -0.5f, -5.0f)

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


void loadTextures();


GLuint compileShader(const char* path, GLenum shaderType) {
    // Read shader code from file
    std::ifstream shaderFile(path);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return 0;
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    std::string shaderCode = buffer.str();
    const char* shaderSource = shaderCode.c_str();

    // Compile shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Error: " << infoLog << std::endl;
        return 0;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindAttribLocation(shaderProgram, 0, "aPos");
    glBindAttribLocation(shaderProgram, 1, "aTexCoord");

    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Linking Error: " << infoLog << std::endl;
    }

    // Clean up shaders (they are linked now, so we can delete them)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}




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
        
        
        if (prefix == "vn") {
            GLfloat nx, ny, nz;
            iss >> nx >> ny >> nz;
            enemyNormals.push_back(nx);
            enemyNormals.push_back(ny);
            enemyNormals.push_back(nz);
        }
        
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
            if (count == 3) {  // Triangle face
                enemyIndices.push_back(vIdx[0]);
                enemyIndices.push_back(vIdx[1]);
                enemyIndices.push_back(vIdx[2]);
            } else if (count == 4) {  // Quad face
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

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < enemyIndices.size(); i++) {
        int index = enemyIndices[i];
        glNormal3f(enemyNormals[index * 3], enemyNormals[index * 3 + 1], enemyNormals[index * 3 + 2]);
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


void drawSky() {
    // Disable lighting for the sky to keep it uniformly lit
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);

    // Set the color to white to avoid tinting the texture
    glColor3f(1.0f, 1.0f, 1.0f);

    // Move the sky dome down on the Y-axis
    glPushMatrix();
    glTranslatef(0.0f, -10.0f, 0.0f);  // Adjust the Y-axis translation as needed
    glRotatef(270,0,1,0);

    // Draw the sky dome as a hemisphere
    int stacks = 20;      // Number of horizontal divisions
    int slices = 40;      // Number of vertical divisions
    float radius = 50.0f; // Radius of the sky dome

    for (int i = 0; i < stacks; ++i) {
        float theta1 = i * M_PI / (2 * stacks);
        float theta2 = (i + 1) * M_PI / (2 * stacks);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float phi = j * 2 * M_PI / slices;

            // First vertex
            float x1 = radius * sin(theta1) * cos(phi);
            float y1 = radius * cos(theta1);
            float z1 = radius * sin(theta1) * sin(phi);
            glTexCoord2f((float)j / slices, (float)i / stacks);
            glVertex3f(x1, y1, z1);

            // Second vertex
            float x2 = radius * sin(theta2) * cos(phi);
            float y2 = radius * cos(theta2);
            float z2 = radius * sin(theta2) * sin(phi);
            glTexCoord2f((float)j / slices, (float)(i + 1) / stacks);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }

    glPopMatrix(); // Restore the previous matrix state

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);  // Re-enable lighting
}



// Function to draw the ground with a repeating texture
void drawGround() {
    // Enable texture mapping if using a ground texture
    if (groundTexture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
    }

    glPushMatrix();
    glColor3f(0.6f, 0.8f, 0.6f);  // Grass-like color
    glTranslatef(0.0f, -3.8f, 0.0f);  // Position the ground at y = -3.8

    // Define the size of the ground plane
    float groundSize = 1000.0f;  // Large size to simulate infinity
    float textureRepeat = 100.0f; // Number of times to repeat the texture

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  // Normal pointing up

    // Define vertices with texture coordinates for a large plane
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-groundSize, 0.0f, -groundSize);
    glTexCoord2f(textureRepeat, 0.0f); glVertex3f(groundSize, 0.0f, -groundSize);
    glTexCoord2f(textureRepeat, textureRepeat); glVertex3f(groundSize, 0.0f, groundSize);
    glTexCoord2f(0.0f, textureRepeat); glVertex3f(-groundSize, 0.0f, groundSize);

    glEnd();

    glPopMatrix();

    // Disable texture mapping if it was enabled
    if (groundTexture != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}


void drawDefensiveCannon() {
    
    // If the explosion is complete, do nothing
    if (explosionComplete) {
        return;
    }
    
    glPushMatrix();
    
    // Move to the camera position
    glTranslatef(0.0f, -3.0f, 15.0f);

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

// wheel texture
            
            if (wheelTexture != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, wheelTexture);
            }
            
            glColor3f(0.54f, 0.27f, 0.07f);


            GLUquadric* wheel = gluNewQuadric();
            gluQuadricTexture(wheel, GL_TRUE); // Enable texture generation for the quadric


            glPushMatrix();
                // Position the wheel at the base of the cannon
                glTranslatef(-0.5f, -0.5f, 0.5f);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

                // Draw the wheel rim
                gluCylinder(wheel, 0.4, 0.4, 0.2, 30, 30);

                // Draw the front face of the wheel
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.0f);
                gluDisk(wheel, 0.0, 0.4, 30, 1);
                glPopMatrix();

                // Draw the back face of the wheel
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.2f);
                gluDisk(wheel, 0.0, 0.4, 30, 1);
                glPopMatrix();
            glPopMatrix();
            
            glPushMatrix();
                // Position the wheel at the base of the cannon
                glTranslatef(0.3f, -0.5f, 0.5f);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

                // Draw the wheel rim
                gluCylinder(wheel, 0.4, 0.4, 0.2, 30, 30);

                // Draw the front face of the wheel
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.0f);
                gluDisk(wheel, 0.0, 0.4, 30, 1);
                glPopMatrix();

                // Draw the back face of the wheel
                glPushMatrix();
                glTranslatef(0.0f, 0.0f, 0.2f);
                gluDisk(wheel, 0.0, 0.4, 30, 1);
                glPopMatrix();
            glPopMatrix();
            
            glDisable(GL_TEXTURE_2D);
// wheel texture
            
            // custom mesh
            glColor3f(0.0f, 0.0f, 0.9f);
            glPushMatrix();
                // Apply transformations for the custom mesh
                glTranslatef(0.0f, 0.0f, 1.0f); // Position where the mesh should be drawn
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                glScalef(0.35f, 0.35f, 0.35f);
                // Set color if needed
                
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
        0.0f, -3.0f, 15.0f,         // Start at the cannon's position
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
    glEnable(GL_LIGHTING);

    // Set the camera view
    gluLookAt(eyeX, eyeY, eyeZ,  // Camera position
              0.0, 0.0, 0.0,   // Look-at point
              0.0, 1.0, 0.0);  // Up direction

    
    //draw sky
    drawSky();
    
    // Draw the ground
    drawGround();

    // Draw the defensive cannon
    drawDefensiveCannon();
    
    
    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);
    // bind shader & texture
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    
    // Draw all the bots
    for (auto& bot : bots) {
        bot.draw();
    }
    
    glBindVertexArray(0);
    glUseProgram(0);  // Reset to the fixed-function pipeline if necessary
    
    // Disable texture
    glDisable(GL_TEXTURE_2D);

    // Draw defensive projectiles
    drawDefensiveProjectiles();


    glutSwapBuffers();
}

// Initialization function
void initOpenGL() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Light blue sky background
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

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

    // VAO and VBO setup
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Enable vertex attributes (positions and texture coordinates)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
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
    
    // Create and compile the shader program after OpenGL context is initialized
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");
    
    // Load textures after initializing OpenGL and shaders
    loadTextures();
    
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

void loadTextures() {
    
    // Load body texture
    glGenTextures(1, &bodyTexture);
    glBindTexture(GL_TEXTURE_2D, bodyTexture);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/gray.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("Body texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to load body texture\n");
    }
    
    
    // Load eye texture
    glGenTextures(1, &eyeTexture);
    glBindTexture(GL_TEXTURE_2D, eyeTexture);

    data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/green.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("Eye texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to load eye texture\n");
    }
    
    
    // Load hand texture
    glGenTextures(1, &handTexture);
    glBindTexture(GL_TEXTURE_2D, handTexture);

    data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/black.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("Hand texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to load hand texture\n");
    }
    
    
    // Load cannon-wheel texture
    glGenTextures(1, &wheelTexture);
    glBindTexture(GL_TEXTURE_2D, wheelTexture);

    data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/wheel.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("Wheel texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to load wheel texture\n");
    }
    
    // Load ground texture
    glGenTextures(1, &groundTexture);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/ground.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("Ground texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to ground texture\n");
    }

    
    // Load sky texture
    glGenTextures(1, &skyTexture);


    glBindTexture(GL_TEXTURE_2D, skyTexture);

    data = stbi_load("/Users/sherrysanij/Documents/cps511/a3/sky.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        printf("sky texture loaded successfully: %dx%d, Channels: %d\n", width, height, nrChannels);
    } else {
        printf("Failed to sky texture\n");
    }

    

    for (auto& bot : bots) {
        bot.setBodyTexture(bodyTexture);
        bot.setEyeTexture(eyeTexture);
        bot.setHandTexture(handTexture);
        bot.setWheelTexture(wheelTexture);
    }
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
        Enemy(8.0f, -0.5f, -8.0f),
        Enemy(2.0f, -0.5f, -14.0f),
        Enemy(-3.0f, -0.5f, 3.0f)
    };
    
    // Reassign textures to the new bots
    for (auto& bot : bots) {
        bot.setBodyTexture(bodyTexture);
        bot.setEyeTexture(eyeTexture);
        bot.setHandTexture(handTexture);
        bot.setWheelTexture(wheelTexture);
    }

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
        Enemy(5.0f, -0.5f, -10.0f),
        Enemy(0.0f, -0.5f, -15.0f),
        Enemy(-5.0f, -0.5f, -5.0f)
    };
    defensiveProjectiles.clear();
    animatingWalk = true;
    
    
    // Reassign textures to the new bots
    for (auto& bot : bots) {
        bot.setBodyTexture(bodyTexture);
        bot.setEyeTexture(eyeTexture);
        bot.setHandTexture(handTexture);
        bot.setWheelTexture(wheelTexture);
    }
    
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

