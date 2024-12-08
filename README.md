# StarWars

<img width="1440" alt="Screenshot 2024-12-07 at 8 50 33 PM" src="https://github.com/user-attachments/assets/7651a204-8e3f-447e-86c3-4a63f1574e3f">


## Defensive Cannon
- Rotation
- Press the Spacebar to fire projectiles
- borken state until 'r' reset game
- explosion animation
- cannon => custom mesh
- wheels => textured

## Enemy Robots
- Walking Animation
- Start Walking: Press 'w'
- Stop Walking: Press 'W'
- Disappeared by being hit with projectiles.
- Random Attacks
- Disappear when reach end of field
- New phase when all robots are killed or disappeared
- different textures for 'gray' , 'black' , 'green'

<img width="1440" alt="Screenshot 2024-12-07 at 8 51 11 PM" src="https://github.com/user-attachments/assets/88645fd1-d5d8-4fbd-b98e-791fadc73399">

## Texturing
- Body Texture (gray.jpg): Applied to enemy robots’ bodies.
- Eye Texture (green.jpg): Applied to enemy robots’ eyes.
- Hand Texture (black.jpg): Applied to enemy robots’ hands.
- Wheel Texture (wheel.jpg): Applied to cannon wheels.
- Ground Texture (ground.jpg): Applied to the infinite ground plane.
- Sky Texture (sky.jpg): Applied to the sky dome.

## Graphic Pipeline
- Lighting: Implemented using a vertex shader and a fragment shader.
- Shading: Real-time shading with ambient, diffuse, and specular components.
- Texture Mapping: Applied to all models for realistic visual effects.

## how to get custom mesh 
1. brew install freeglut
2. brew list freeglut
3. brew install glew
4. brew install --cask xquartz
5. open -a XQuartz
6. export DISPLAY=:0
7. g++ surfaceModeller.cpp -o a2 \
-I/opt/homebrew/Cellar/freeglut/3.6.0/include \
-I/opt/homebrew/Cellar/glew/2.2.0_1/include \
-L/opt/homebrew/Cellar/freeglut/3.6.0/lib \
-L/opt/homebrew/Cellar/glew/2.2.0_1/lib \
-framework OpenGL -lglut -lGLEW
8. ./a2
9. press keyboard 'e' to extract "mesh_data.txt"

## how to compile and run
1. g++ -std=c++11 -g -framework OpenGL -framework GLUT -o bot main.cpp enemy.cpp QuadMesh.cpp
2. ./bot

## Keyboard Control
1. Spacebar: Fire defensive cannon projectiles.
2. w: Start enemy robots walking.
3. W: Stop enemy robots walking.
4. r: Restart the game.
5. (←): Move the camera left.
6. (→): Move the camera right.
7. (↑): Move the camera up.
8. (↓): Move the camera down.
   
## Mouse Controls
1. Rotate the cannon by moving the mouse.
