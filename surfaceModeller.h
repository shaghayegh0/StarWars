typedef struct Vector2D
{
    GLdouble x, y;
    GLdouble nx, ny;
    GLdouble u; // running distance
} Vector2D;

// Struct prototypes
typedef struct SubdivisionCurve SubdivisionCurve;
typedef struct Rect Rect;
typedef struct Button Button;
typedef struct Vertex Vertex;

// Function prototypes
void init2DCurveWindow();
void display2D();
void reshape2D(int w, int h);
void mouseButtonHandler2D(int button, int state, int x, int y);
void mouseMotionHandler2D(int x, int y);
void keyboardHandler2D(unsigned char key, int x, int y);
void specialKeyHandler2D(int key, int x, int y);
void mouseScrollWheelHandler2D(int button, int dir, int xMouse, int yMouse);
void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam);
void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera);
void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw);
void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw);
void draw2DScene();
void drawAxes();
void drawSubdivisionCurve();
Vector2D getPoint(Vector2D *currentPoints, int numCurrentPoints, int j) ;
Vector2D subdivide(Vector2D v1, Vector2D v2, Vector2D v3, Vector2D v4) ;
void computeSubdivisionCurve(SubdivisionCurve *subcurvePointer) ;
void initSubdivisionCurve();
void constructCircle(double radius, int numPoints, Vector2D* circlePts);
void drawControlPoints();
void initControlPointCircles();
void mouseHoverHandler2D(int x, int y);
void computeRunningDistances(SubdivisionCurve *subcurvePointer);
void computeNormalVectors(SubdivisionCurve *subcurvePointer);
void init3DSurfaceWindow();


/*

    3D Functions

*/

typedef struct Vector3D
{
    GLdouble x, y, z;
} Vector3D;


void display3D();
void reshape3D(int w, int h);
void mouseButtonHandler3D(int button, int state, int x, int y);
void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse);
void mouseMotionHandler3D(int xMouse, int yMouse);
void keyboardHandler3D(unsigned char key, int x, int y);

void buildVertexArray();
void buildQuadArray();
void computeQuadNormals();
void computeVertexNormals();
Vector3D rotateAroundY(double x, double z, double theta);
Vector3D crossProduct(Vector3D a, Vector3D b);
float DotProduct(Vector3D lhs, Vector3D rhs);
Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d);
Vector3D normalize(Vector3D a);

void drawGround();
void drawQuads();
void drawQuadsAsLines();
void drawQuadsAsPoints();



