#define GL_SILENCE_DEPRECATION

#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <unistd.h>
#include <stdio.h>






void freeSubdivisionCurve(SubdivisionCurve *curve);
void exportMeshToFile(const char* filename);


// The profile curve is a subdivision curve
SubdivisionCurve subcurve;



// Initialize values
void initialize() {
    subcurve.numControlPoints = 0;
    subcurve.subdivisionSteps = 3;
    subcurve.numCurvePoints = 100;
    subcurve.curvePoints = NULL;
}




void cleanup3D();
void cleanup() {
    cleanup3D();
    freeSubdivisionCurve(&subcurve);
    
}

void freeSubdivisionCurve(SubdivisionCurve *curve) {
    if (curve->curvePoints != NULL) {
        free(curve->curvePoints);
        curve->curvePoints = NULL;
    }
    curve->numCurvePoints = 0;
}


GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;


// 2. camera contorl
GLdouble cameraAzimuth = 0.0; // Horizontal rotation around y-axis
GLdouble cameraElevation = 15.0; // Vertical elevation angle (0–60 degrees)
GLdouble cameraRadius = 10.0; // Distance from the surface
const GLdouble minElevation = 0.0, maxElevation = 60.0; // Elevation limits
const GLdouble minRadius = 5.0, maxRadius = 20.0; // Zoom limits


GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// Global declaration
GLuint* indices = NULL;


// screen window identifiers
int window2D, window3D;

int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;



void importMeshFromFile(const char* filename);
void setupBuffers();


int main(int argc, char* argv[])
{
    // 4. export & import
    char cwd[1024];
    
    initialize();
    
    
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(glutWindowWidth,glutWindowHeight);
    glutInitWindowPosition(50,100);
    
    // 4. import
    importMeshFromFile("exported_mesh.txt");
    
    // The 2D Window
    window2D = glutCreateWindow("Profile Curve");
    glutDisplayFunc(display2D);
    glutReshapeFunc(reshape2D);
    // Initialize the 2D profile curve system
    init2DCurveWindow();
    // A few input handlers
    glutMouseFunc(mouseButtonHandler2D);
    glutMotionFunc(mouseMotionHandler2D);
    glutPassiveMotionFunc(mouseHoverHandler2D);
    glutMouseWheelFunc(mouseScrollWheelHandler2D);
    glutSpecialFunc(specialKeyHandler2D);
    glutKeyboardFunc(keyboardHandler2D);
    
    // The 3D Window
    window3D = glutCreateWindow("Surface of Revolution");
    glutPositionWindow(900,100);
    glutDisplayFunc(display3D);
    glutReshapeFunc(reshape3D);
    glutMouseFunc(mouseButtonHandler3D);
    glutMouseWheelFunc(mouseScrollWheelHandler3D);
    glutMotionFunc(mouseMotionHandler3D);
    glutKeyboardFunc(keyboardHandler3D);
    
    // Initialize the 3D system
    init3DSurfaceWindow();

    // 3. mesh rendering
    setupBuffers();
    
    // Annnd... ACTION!!
    glutMainLoop();

    return 0;
}

/************************************************************************************
 *
 *
 * 2D Window and Profile Curve Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/



int hoveredCtlPt = -1;
int currentCurvePoint = 0;

// Use little circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];
int numCirclePoints = 30;
double circleRadius = 0.2;


void init2DCurveWindow()
{
    glLineWidth(3.0);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
    initSubdivisionCurve();
    initControlPointCircles();
}

void display2D()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    draw2DScene();
    glutSwapBuffers();
}


void draw2DScene()
{
    drawAxes();
    drawSubdivisionCurve();
    drawControlPoints();
}

void drawAxes()
{
    glPushMatrix();
    glColor3f(1.0, 0.0, 0);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, 8.0, 0);
    glVertex3f(0, -8.0, 0);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3f(-8, 0.0, 0);
    glVertex3f(8, 0.0, 0);
    glEnd();
    glPopMatrix();
}

void drawSubdivisionCurve()
{
    // Subdivide the given curve
    computeSubdivisionCurve(&subcurve);
    glColor3f(0.0, 1.0, 0.0);
    glPushMatrix();
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<subcurve.numCurvePoints; i++)
    {
        glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
    }
    glEnd();
    glPopMatrix();
}

void drawControlPoints()
{
    int i, j;
    for (i=0; i<subcurve.numControlPoints; i++){
        glPushMatrix();
        glColor3f(1.0f,0.0f,0.0f);
        glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
        // for the hoveredCtlPt, draw an outline and change its color
        if (i == hoveredCtlPt)
        {
            // outline
            glColor3f(0.0, 1.0, 0.0);
            glBegin(GL_LINE_LOOP);
            for(j=0; j < numCirclePoints; j++) {
                glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
            }
            glEnd();
            // color change
            glColor3f(0.5,0.0,1.0);
        }
        glBegin(GL_LINE_LOOP);
        for(j=0; j < numCirclePoints; j++) {
            glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
        }
        glEnd();
        glPopMatrix();
    }
}

void initSubdivisionCurve() {
    // Initialize 5 control points of the subdivision curve

    GLdouble x, y;

    x = 2 * cos(M_PI*0.5);
    y = 2 * sin(M_PI*0.5);
    subcurve.controlPoints[0].x = x;
    subcurve.controlPoints[0].y = y;

    x = 2 * cos(M_PI*0.25);
    y = 2 * sin(M_PI*0.25);
    subcurve.controlPoints[1].x = x;
    subcurve.controlPoints[1].y = y;

    x = 2 * cos(M_PI*0.0);
    y = 2 * sin(M_PI*0.0);
    subcurve.controlPoints[2].x = x;
    subcurve.controlPoints[2].y = y;

    x = 2 * cos(-M_PI*0.25);
    y = 2 * sin(-M_PI*0.25);
    subcurve.controlPoints[3].x = x;
    subcurve.controlPoints[3].y = y;

    x = 2 * cos(-M_PI * 0.5);
    y = 2 * sin(-M_PI * 0.5);
    subcurve.controlPoints[4].x = x;
    subcurve.controlPoints[4].y = y;

    subcurve.numControlPoints = 5;
    subcurve.subdivisionSteps = 3;
}

void initControlPointCircles()
{
    int num = subcurve.numControlPoints;
    for (int i=0; i < num; i++){
        constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
        circles[i].circleCenter = subcurve.controlPoints[i];
    }
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw)
{
    GLdouble xView, yView;
    screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
    cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera)
{
    *xCamera = ((wvRight-wvLeft)/glutWindowWidth)  * xScreen;
    *yCamera = ((wvTop-wvBottom)/glutWindowHeight) * (glutWindowHeight-yScreen);
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw)
{
    *xw = xcam + wvLeft;
    *yw = ycam + wvBottom;
}

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
    double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
    double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
    *xcam = worldCenterX - wvCenterX + xWorld;
    *ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler2D(int button, int state, int xMouse, int yMouse)
{
    int i;
    
    currentButton = button;
    if (button == GLUT_LEFT_BUTTON)
    {
        switch (state) {
        case GLUT_DOWN:
            if (hoveredCtlPt > -1)
            {
              screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
              screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
            }
            break;
        case GLUT_UP:
            glutSetWindow(window3D);
            glutPostRedisplay();
            break;
        }
    }
    else if (button == GLUT_MIDDLE_BUTTON)
    {
        switch (state) {
        case GLUT_DOWN:
            break;
        case GLUT_UP:
            if (hoveredCtlPt == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS)
            {
                GLdouble newPointX;
                GLdouble newPointY;
                screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
                subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
                subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;
                constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
                circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
                subcurve.numControlPoints++;
            }
            else if (hoveredCtlPt > -1 && subcurve.numControlPoints > MINCONTROLPOINTS)
            {
                subcurve.numControlPoints--;
                for (i=hoveredCtlPt; i<subcurve.numControlPoints; i++)
                {
                    subcurve.controlPoints[i].x = subcurve.controlPoints[i+1].x;
                    subcurve.controlPoints[i].y = subcurve.controlPoints[i+1].y;
                    circles[i].circleCenter = circles[i+1].circleCenter;
                }
            }
            
            glutSetWindow(window3D);
            glutPostRedisplay();
            break;
        }
    }

    glutSetWindow(window2D);
    glutPostRedisplay();
}

void mouseMotionHandler2D(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON) {
        if (hoveredCtlPt > -1)
        {
            screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
            screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
            
        }
    }
    glutPostRedisplay();
    glutSetWindow(window3D);
    glutPostRedisplay();
    glutSetWindow(window2D);
}

void mouseHoverHandler2D(int xMouse, int yMouse)
{
    hoveredCtlPt = -1;
    GLdouble worldMouseX, worldMouseY;
    screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
    // See if we're hovering over a circle
    for (int i=0; i<subcurve.numControlPoints; i++){
        GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
        GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
        GLdouble euclideanDist = sqrt(distToX*distToX + distToY*distToY);
        if (euclideanDist < 0.5)
          hoveredCtlPt = i;
    }
    glutPostRedisplay();
}

void mouseScrollWheelHandler2D(int button, int dir, int xMouse, int yMouse)
{
    GLdouble worldViewableWidth;
    GLdouble worldViewableHeight;
    GLdouble cameraOnCenterX;
    GLdouble cameraOnCenterY;
    GLdouble anchorPointX, anchorPointY;
    double clipWindowWidth;
    double clipWindowHeight;
    double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
    double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
    double wvWidth   = wvRight - wvLeft;
    double wvHeight  = wvTop   - wvBottom;
    
    worldToCameraCoordiantes(worldCenterX, worldCenterY, &cameraOnCenterX, &cameraOnCenterY);
    if (wvWidth >= (worldRight-worldLeft)*1.2)
    {
        anchorPointX = cameraOnCenterX;
        anchorPointY = cameraOnCenterY;
    } else
    {
        // else, anchor the zoom to the mouse
        screenToWorldCoordinates(xMouse, yMouse, &anchorPointX, &anchorPointY);
    }
    GLdouble anchorToCenterX = anchorPointX - wvCenterX;
    GLdouble anchorToCenterY = anchorPointY - wvCenterY;

    // Set up maximum shift
    GLdouble maxPosShift = 50;
    GLdouble maxNegShift = -50;
    anchorToCenterX = (anchorToCenterX > maxPosShift)? maxPosShift : anchorToCenterX;
    anchorToCenterX = (anchorToCenterX < maxNegShift)? maxNegShift : anchorToCenterX;
    anchorToCenterY = (anchorToCenterY > maxPosShift)? maxPosShift : anchorToCenterY;
    anchorToCenterY = (anchorToCenterY < maxNegShift)? maxNegShift : anchorToCenterY;

    // Move the world centre closer to this point.
    wvCenterX += anchorToCenterX/4;
    wvCenterY += anchorToCenterY/4;
    
    if (dir > 0)
    {
        // Zoom in to mouse point
        clipWindowWidth = wvWidth*0.8;
        clipWindowHeight= wvHeight*0.8;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
    }
    else
    {
        // Zoom out
        clipWindowWidth = wvWidth*1.25;
        clipWindowHeight= wvHeight*1.25;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
    }
    glutPostRedisplay();
}

void keyboardHandler2D(unsigned char key, int x, int y)
{
    int i;
    
    double clipWindowWidth;
    double clipWindowHeight;
    double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
    double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
    double wvWidth   = wvRight - wvLeft;
    double wvHeight  = wvTop   - wvBottom;

    switch(key)
    {
    case 'q':
    case 'Q':
    case 27:
        // Esc, q, or Q key = Quit
        exit(0);
        break;
    case 107:
    case '+':
        clipWindowWidth = wvWidth*0.8;
        clipWindowHeight= wvHeight*0.8;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
        break;
    case 109:
    case '-':
        clipWindowWidth = wvWidth*1.25;
        clipWindowHeight= wvHeight*1.25;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
        break;
    
    default:
        break;
    }
    glutPostRedisplay();
}

void specialKeyHandler2D(int key, int x, int y)
{
    double clipWindowWidth;
    double clipWindowHeight;
    double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
    double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
    double wvWidth   = wvRight - wvLeft;
    double wvHeight  = wvTop   - wvBottom;

    switch(key)
    {
    case GLUT_KEY_LEFT:
        wvLeft -= 5.0;
        wvRight-= 5.0;
        break;
    case GLUT_KEY_RIGHT:
        wvLeft += 5.0;
        wvRight+= 5.0;
        break;
    case GLUT_KEY_UP:
        wvTop   += 5.0;
        wvBottom+= 5.0;
        break;
    case GLUT_KEY_DOWN:
        wvTop   -= 5.0;
        wvBottom-= 5.0;
        break;
        // Want to zoom in/out and keep  aspect ratio = 2.0
    case GLUT_KEY_F1:
        clipWindowWidth = wvWidth*0.8;
        clipWindowHeight= wvHeight*0.8;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
        break;
    case GLUT_KEY_F2:
        clipWindowWidth = wvWidth*1.25;
        clipWindowHeight= wvHeight*1.25;
        wvRight =  wvCenterX + clipWindowWidth/2.0;
        wvTop   =  wvCenterY + clipWindowHeight/2.0;
        wvLeft  =  wvCenterX - clipWindowWidth/2.0;
        wvBottom=  wvCenterY - clipWindowHeight/2.0;
        break;
    }
    glutPostRedisplay();
}


void reshape2D(int w, int h)
{
    glutWindowWidth = (GLsizei) w;
    glutWindowHeight = (GLsizei) h;
    glViewport(0, 0, glutWindowWidth, glutWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window and Surface of Revolution Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/
 // Ground Mesh material

GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

//
// Surface of Revolution consists of vertices and quads
//
// Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


// 3. mesh rendering
GLuint vao, vboVertices, vboNormals, vboIndices;



// Quads and Vertices of the surface of revolution
typedef struct Vertex
{
    GLdouble x, y, z;
    Vector3D normal;
    int numQuads;
    int quadIndex[4];
} Vertex;

typedef struct Quad
{
    int vertexIndex[4]; // 4 vertex indices in clockwise order
    Vector3D normal;
} Quad;

// Quads
Quad *qarray;
bool quadArrayAllocated = false;

// Vertices
#define NUMBEROFSIDES 16 // You may want to lower this to 4 or 5 when debugging

// 3. mesh rendering
#define MAX_VERTICES 1000
#define MAX_QUADS 5000

Vertex *varray; //
bool varrayAllocated = false;

GLdouble fov = 60.0;

int lastMouseX;
int lastMouseY;

bool drawAsLines = false;
bool drawAsPoints = false;
bool drawNormals = false;

GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;

void init3DSurfaceWindow()
{
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

    glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


void reshape3D(int w, int h)
{
    glutWindowWidth = (GLsizei) w;
    glutWindowHeight = (GLsizei) h;
    glViewport(0, 0, glutWindowWidth, glutWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov,aspect,zNear,zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


// 3. mesh rendering
void setupBuffers() {
    if (!varray) {
        varray = (Vertex *)malloc(subcurve.numCurvePoints * NUMBEROFSIDES * sizeof(Vertex));
    }

   
    

    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);

    


    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, subcurve.numCurvePoints * NUMBEROFSIDES * sizeof(Vertex), varray, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);


    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, subcurve.numCurvePoints * NUMBEROFSIDES * sizeof(Vector3D),
                 &varray[0].normal, GL_STATIC_DRAW);


    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);


    if (!indices) {
        int totalIndices = (subcurve.numCurvePoints - 1) * NUMBEROFSIDES * 4;
        indices = (GLuint *)malloc(totalIndices * sizeof(GLuint));
        

        int idx = 0;
        for (int row = 0; row < subcurve.numCurvePoints - 1; row++) {
            for (int col = 0; col < NUMBEROFSIDES; col++) {
                int nextCol = (col + 1) % NUMBEROFSIDES;

                indices[idx++] = row * NUMBEROFSIDES + col;
                indices[idx++] = (row + 1) * NUMBEROFSIDES + col;
                indices[idx++] = (row + 1) * NUMBEROFSIDES + nextCol;
                indices[idx++] = row * NUMBEROFSIDES + nextCol;
            }
        }
    }

    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (subcurve.numCurvePoints - 1) * NUMBEROFSIDES * 4 * sizeof(GLuint),
                 indices, GL_STATIC_DRAW);



    glBindVertexArrayAPPLE(0);
}

void renderNormals();


void display3D() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    // Compute camera position based on spherical coordinates
    GLdouble eyeX = cameraRadius * sin(cameraElevation * M_PI / 180.0) * sin(cameraAzimuth * M_PI / 180.0);
    GLdouble eyeY = cameraRadius * cos(cameraElevation * M_PI / 180.0);
    GLdouble eyeZ = cameraRadius * sin(cameraElevation * M_PI / 180.0) * cos(cameraAzimuth * M_PI / 180.0);

    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    drawGround();

    // Build and Draw Surface of Revolution (Quad Mesh)
    buildVertexArray();
    buildQuadArray();
    computeQuadNormals();
    computeVertexNormals();

    // Draw quad mesh
    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);
    
    // 3. mesh rendering
    buildVertexArray();
    buildQuadArray();
    computeQuadNormals();
    computeVertexNormals();
    
    

    if (drawAsLines)
        drawQuadsAsLines();
    else if (drawAsPoints)
        drawQuadsAsPoints();
    else
        drawQuads();
    
    renderNormals();


    glPopMatrix();
    glutSwapBuffers();
}


void drawGround()
{
    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-22.0f, -4.0f, -22.0f);
    glVertex3f(-22.0f, -4.0f, 22.0f);
    glVertex3f(22.0f, -4.0f, 22.0f);
    glVertex3f(22.0f, -4.0f, -22.0f);
    glEnd();
    glPopMatrix();
}


void buildVertexArray()
{
    int row, col;
    double newX, newZ;
    GLdouble cumulativeTheta = 0;
    GLdouble theta = 360/(GLdouble)NUMBEROFSIDES; // = 18

    // Allocate memeory for the vertex array - needs to be done only once
    if (!varrayAllocated)
    {
        varray = (Vertex *)malloc(subcurve.numCurvePoints* NUMBEROFSIDES * sizeof(Vertex));
        varrayAllocated = true;
    }
    /*  -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
    */
    for (row = 0; row < subcurve.numCurvePoints; row++)
    {
        for (col = 0; col < NUMBEROFSIDES; col++)
        {
            Vector3D newVector = rotateAroundY(subcurve.curvePoints[row].x, 0, cumulativeTheta);
            varray[row*NUMBEROFSIDES + col].numQuads = 0;
            varray[row*NUMBEROFSIDES + col].y = subcurve.curvePoints[row].y;
            varray[row*NUMBEROFSIDES + col].x = newVector.x;
            varray[row*NUMBEROFSIDES + col].z = newVector.z;
            cumulativeTheta += theta;
        }
    }
}

Vector3D rotateAroundY(double x, double z, double theta)
{
    Vector3D newVector;
    
    newVector.x =  cos(theta/180*M_PI) * x + sin(theta/180.0*M_PI) * z;
    newVector.z = -sin(theta/180*M_PI) * x + cos(theta/180.0*M_PI) * z;
    return newVector;
}

void buildQuadArray()
{
    int col, row, numQuads;
    
    if (!quadArrayAllocated)
    {
        qarray = (Quad *)malloc(sizeof(Quad)*(subcurve.numCurvePoints-1)*NUMBEROFSIDES);
    }
    /*  -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
        |  |  |  |  |
        -------------
    */
    for (row = 0; row < subcurve.numCurvePoints-1; row++)
    {
        for (col = 0; col < NUMBEROFSIDES; col++)
        {
            int nextCol;

            // if the we are at last column, then the next column must wrap around back to 0th column
            if (col == NUMBEROFSIDES - 1)
                nextCol = 0;
            else
                nextCol = col + 1;

            qarray[row*NUMBEROFSIDES + col].vertexIndex[0] = row * NUMBEROFSIDES + col;
            numQuads = varray[row*NUMBEROFSIDES + col].numQuads;
            varray[row*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
            varray[row*NUMBEROFSIDES + col].numQuads++;

            qarray[row*NUMBEROFSIDES + col].vertexIndex[1] = (row+1) * NUMBEROFSIDES + col;
            numQuads = varray[(row+1)*NUMBEROFSIDES + col].numQuads;
            varray[(row+1)*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
            varray[(row+1)*NUMBEROFSIDES + col].numQuads++;

            qarray[row*NUMBEROFSIDES + col].vertexIndex[2] = (row + 1) * NUMBEROFSIDES + nextCol;
            numQuads = varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads;
            varray[(row + 1)*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
            varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads++;

            qarray[row*NUMBEROFSIDES + col].vertexIndex[3] = row * NUMBEROFSIDES + nextCol;
            numQuads = varray[row*NUMBEROFSIDES + nextCol].numQuads;
            varray[row*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
            varray[row*NUMBEROFSIDES + nextCol].numQuads++;
        }
    }
}

void computeQuadNormals() {
    for (int i = 0; i < (subcurve.numCurvePoints - 1) * NUMBEROFSIDES; i++) {
        Vertex *v0 = &varray[qarray[i].vertexIndex[0]];
        Vertex *v1 = &varray[qarray[i].vertexIndex[1]];
        Vertex *v2 = &varray[qarray[i].vertexIndex[2]];

        Vector3D edge1 = {v1->x - v0->x, v1->y - v0->y, v1->z - v0->z};
        Vector3D edge2 = {v2->x - v0->x, v2->y - v0->y, v2->z - v0->z};

        Vector3D normal = crossProduct(edge1, edge2);
        qarray[i].normal = normalize(normal);

        if (isnan(qarray[i].normal.x) || isnan(qarray[i].normal.y) || isnan(qarray[i].normal.z)) {
            qarray[i].normal = (Vector3D){0.0, 1.0, 0.0};
        }
    }
}

void computeVertexNormals() {
    for (int i = 0; i < subcurve.numCurvePoints * NUMBEROFSIDES; i++) {
        Vector3D vn = {0.0, 0.0, 0.0};
        for (int j = 0; j < varray[i].numQuads; j++) {
            int quadIndex = varray[i].quadIndex[j];
            vn.x += qarray[quadIndex].normal.x;
            vn.y += qarray[quadIndex].normal.y;
            vn.z += qarray[quadIndex].normal.z;
        }

        varray[i].normal = normalize(vn);

        if (isnan(varray[i].normal.x) || isnan(varray[i].normal.y) || isnan(varray[i].normal.z)) {
            varray[i].normal = (Vector3D){0.0, 1.0, 0.0};
        }
    }
}




//void drawQuads()
//{
//    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);
//
//    
//    // 3. mesh rendering
//    // Bind VAO and draw elements
//    glBindVertexArrayAPPLE(vao);
//    glDrawElements(GL_QUADS, (subcurve.numCurvePoints - 1) * NUMBEROFSIDES * 4, GL_UNSIGNED_INT, 0);
//    glBindVertexArrayAPPLE(0);
//}



void drawQuads() {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

    glBegin(GL_QUADS);
    for (int i = 0; i < (subcurve.numCurvePoints - 1) * NUMBEROFSIDES; i++) {
        for (int j = 0; j < 4; j++) {
            int vIndex = qarray[i].vertexIndex[j];
            glNormal3f(varray[vIndex].normal.x, varray[vIndex].normal.y, varray[vIndex].normal.z);
            glVertex3f(varray[vIndex].x, varray[vIndex].y, varray[vIndex].z);
        }
    }
    glEnd();
}

// 1. Drawing Style
void drawQuadsAsPoints()
{
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0); // Set points color (red)
    glPointSize(5.0); // Set point size
    glBegin(GL_POINTS);
    for (int row = 0; row < subcurve.numCurvePoints; row++) {
        for (int col = 0; col < NUMBEROFSIDES; col++) {
            Vertex *vertex = &varray[row * NUMBEROFSIDES + col];
            glVertex3f(vertex->x, vertex->y, vertex->z);
        }
    }
    glEnd();
    glPopMatrix();
}


// 1. Drawing Style
void drawQuadsAsLines()
{
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0); // Set wireframe color (white)
    for (int row = 0; row < subcurve.numCurvePoints - 1; row++) {
        for (int col = 0; col < NUMBEROFSIDES; col++) {
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 4; i++) {
                Vertex *vertex = &varray[qarray[row * NUMBEROFSIDES + col].vertexIndex[i]];
                glVertex3f(vertex->x, vertex->y, vertex->z);
            }
            glEnd();
        }
    }

    if (drawNormals) {
        glColor3f(0.0, 1.0, 0.0); // Normal vectors color (green)
        for (int row = 0; row < subcurve.numCurvePoints; row++) {
            for (int col = 0; col < NUMBEROFSIDES; col++) {
                Vertex *vertex = &varray[row * NUMBEROFSIDES + col];
                Vector3D normal = vertex->normal;
                glBegin(GL_LINES);
                glVertex3f(vertex->x, vertex->y, vertex->z);
                glVertex3f(vertex->x + normal.x, vertex->y + normal.y, vertex->z + normal.z);
                glEnd();
            }
        }
    }
    glPopMatrix();
}




// A few utility functions - use VECTOR3D.h or glm if you prefer
Vector3D crossProduct(Vector3D a, Vector3D b)
{
    Vector3D cross;
    
    cross.x = a.y * b.z - b.y * a.z;
    cross.y = a.x * b.z - b.x * a.z;
    cross.z = a.x * b.y - b.x * a.y;
    return cross;
}

float DotProduct(Vector3D lhs, Vector3D rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d)
{
    Vector3D average;
    average.x = (a.x + b.x + c.x + d.x)/4.0;
    average.y = (a.y + b.y + c.y + d.y)/4.0;
    average.z = (a.z + b.z + c.z + d.z)/4.0;
    return average;
}

Vector3D normalize(Vector3D a)
{
    GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    Vector3D normalized;
    normalized.x = a.x/norm;
    normalized.y = a.y/norm;
    normalized.z = a.z/norm;
    return normalized;
}

// Mouse and Key Handling
//
//
void mouseButtonHandler3D(int button, int state, int x, int y)
{
    currentButton = button;
    lastMouseX = x;
    lastMouseY = y;
    switch (button)
    {
    case GLUT_MIDDLE_BUTTON:
        if (state == GLUT_DOWN)
        {
           // Fill in code for zooming or use scroll wheel
        }
    break;
    default:
        break;
    }
}

// 2. camera control
void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse) {
    if (dir > 0) {
        // Zoom in
        cameraRadius -= 0.5;
        if (cameraRadius < minRadius) cameraRadius = minRadius;
    } else {
        // Zoom out
        cameraRadius += 0.5;
        if (cameraRadius > maxRadius) cameraRadius = maxRadius;
    }
    glutPostRedisplay();
}


// 2. camera control
void mouseMotionHandler3D(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (currentButton == GLUT_LEFT_BUTTON) {
        // Orbit camera (change azimuth)
        cameraAzimuth += dx * 0.5; // Sensitivity multiplier
        if (cameraAzimuth > 360) cameraAzimuth -= 360;
        if (cameraAzimuth < 0) cameraAzimuth += 360;
    }
    else if (currentButton == GLUT_RIGHT_BUTTON) {
        // Elevate camera
        cameraElevation += dy * 0.5; // Sensitivity multiplier
        if (cameraElevation > maxElevation) cameraElevation = maxElevation;
        if (cameraElevation < minElevation) cameraElevation = minElevation;
    }

    lastMouseX = x;
    lastMouseY = y;
    glutPostRedisplay();
}


// 1. Drawing Style
void keyboardHandler3D(unsigned char key, int x, int y)
{
    
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        // Esc, q, or Q key = Quit
        exit(0);
        break;
    case 'e':
        exportMeshToFile("/Users/sherrysanij/Downloads/Assignment2-SkeletonCode/mesh_data.txt");

        break;
    case 'i':
        importMeshFromFile("/Users/sherrysanij/Downloads/Assignment2-SkeletonCode/mesh_data.txt");

        break;
    case 'l':
        if (drawAsLines)
            drawAsLines = false;
        else
            drawAsLines = true;
        break;
    case 'p':
        if (drawAsPoints)
            drawAsPoints = false;
        else
            drawAsPoints = true;
        break;
    case 'n':
        if (drawNormals)
            drawNormals = false;
        else
            drawNormals = true;
    default:
        break;
    }
    glutPostRedisplay();
}


void cleanup3D() {
    if (varray) {
        free(varray);
        varray = NULL;
    }
    if (qarray) {
        free(qarray);
        qarray = NULL;
    }
    glDeleteBuffers(1, &vboVertices);
    glDeleteBuffers(1, &vboNormals);

}



void exportMeshToFile(const char* filename) {
    if (!varray || !qarray) {
        printf("Error: Mesh data is not initialized. Export aborted.\n");
        return;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create file %s.\n", filename);
        return;
    }

    for (int i = 0; i < subcurve.numCurvePoints * NUMBEROFSIDES; i++) {
        fprintf(file, "v %f %f %f\n", varray[i].x, varray[i].y, varray[i].z);
    }

    for (int i = 0; i < subcurve.numCurvePoints * NUMBEROFSIDES; i++) {
        fprintf(file, "vn %f %f %f\n", varray[i].normal.x, varray[i].normal.y, varray[i].normal.z);
    }

    for (int i = 0; i < (subcurve.numCurvePoints - 1) * NUMBEROFSIDES; i++) {
        fprintf(file, "f %d %d %d %d\n",
                qarray[i].vertexIndex[0] + 1,
                qarray[i].vertexIndex[1] + 1,
                qarray[i].vertexIndex[2] + 1,
                qarray[i].vertexIndex[3] + 1);
    }

    fclose(file);
    printf("Mesh exported successfully to %s.\n", filename);
}


void importMeshFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s.\n", filename);
        return;
    }

    
    

    char line[128];
    int vertexCount = 0, normalCount = 0, faceCount = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            if (vertexCount >= MAX_VERTICES) {
                printf("Error: Vertex limit exceeded.\n");
                break;
            }
            sscanf(line, "v %lf %lf %lf",
                   &varray[vertexCount].x,
                   &varray[vertexCount].y,
                   &varray[vertexCount].z);
            vertexCount++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (normalCount >= MAX_VERTICES) {
                printf("Error: Normal limit exceeded.\n");
                break;
            }
            sscanf(line, "vn %lf %lf %lf",
                   &varray[normalCount].normal.x,
                   &varray[normalCount].normal.y,
                   &varray[normalCount].normal.z);
            normalCount++;
        } else if (line[0] == 'f') {
            if (faceCount >= MAX_QUADS) {
                printf("Error: Face limit exceeded.\n");
                break;
            }
            sscanf(line, "f %d %d %d %d",
                   &qarray[faceCount].vertexIndex[0],
                   &qarray[faceCount].vertexIndex[1],
                   &qarray[faceCount].vertexIndex[2],
                   &qarray[faceCount].vertexIndex[3]);


            for (int i = 0; i < 4; i++) {
                qarray[faceCount].vertexIndex[i]--;
            }
            faceCount++;
        }
    }

    fclose(file);

    printf("Imported mesh with %d vertices, %d normals, and %d faces.\n",
           vertexCount, normalCount, faceCount);

    glutPostRedisplay();
}


void renderNormals() {
    if (!drawNormals) return;

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < subcurve.numCurvePoints * NUMBEROFSIDES; i++) {
        Vertex *v = &varray[i];

        glVertex3f(v->x, v->y, v->z);
        glVertex3f(
            v->x + v->normal.x * 0.5,
            v->y + v->normal.y * 0.5,
            v->z + v->normal.z * 0.5
        );
    }
    glEnd();
}

void cleanupBuffers() {
    if (varray) {
        free(varray);
        varray = NULL;
    }
    if (indices) {
        free(indices);
        indices = NULL;
    }
    glDeleteBuffers(1, &vboVertices);
    glDeleteBuffers(1, &vboNormals);
    glDeleteBuffers(1, &vboIndices);
    glDeleteVertexArraysAPPLE(1, &vao);

}
