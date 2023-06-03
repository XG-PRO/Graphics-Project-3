// Implicit conversion from data type A to 
// data type B is now forbidden >:D (sanest)
#pragma warning(error: 4305)

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glut.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

#define EAST_WALL   0b001
#define SOUTH_WALL  0b010
#define EAST_ROOF   0b011
#define SOUTH_ROOF  0b100
#define GROUND      0b101

#define HEIGHT_CAMERA 40.0
#define RADIUS_CAMERA 70.0

#define POLY_COUNT 100

#define GRASS   0.00000000f, 0.6039216f, 0.09019608f
#define BROWN   0.36078432f, 0.2509804f, 0.20000000f
#define GRAY    0.30000000f, 0.3000000f, 0.30000000f

#define SQRT_75_PLUS_10 18.66025403784439


volatile bool polygon_high = true;
volatile bool spotlight_on = true;
volatile bool smooth_shade = true;

static int main_menu_id;
static int polygon_submenu_id;
static int spotlight_submenu_id;
static int shade_submenu_id;

extern void polygon_submenu(int);
extern void spotlight_submenu(int);
extern void shade_submenu(int);
extern void main_menu(int);

static GLint subdivision_count = 4;

static GLuint ground_base_list;
static GLubyte ground_polygons_ids[POLY_COUNT];

static volatile GLdouble cam_pos[] = {0.0, HEIGHT_CAMERA, RADIUS_CAMERA};
static volatile GLdouble cam_angle = 0.0;

typedef float point3[3];

//LIGHT EXPLANATION
//position(x,y,z,w), x,y,z indicate starting poisiton of the light relative to the modelview matrix, 
// w indicates whether the light is a near light source (1) or a far away light source (0)
//direction(x,y,z,c), x,y,z indicate the position the light source points at in homogeneous object coordinates
//diffusion(r,g,b,a), ambient(r,g,b,a), specular(r,g,b,a) all indicate their respective light attribute
//	with r,g,b,a being the normalized RGBA values for the source from -1.0 to 1.0
//
//kateuthintiki = specular
//diaxiti = diffuse
//periballontos fotos = ambient

//Sun Light and Materials
static GLfloat sunlight = 0.3f;
static GLfloat sunEmissionMaterial[] = { 0.5f, 0.5f, 0.0f, 1.0f };
static GLfloat diffuse_sun[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat ambient_sun[] = { 1.0f, 1.0f, 0.0f, 1.0f };
static GLfloat spec_sun[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat position_sun[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat direction_sun[] = { 0.0f, 0.0f, 0.0f };
static GLfloat sun_angle = 0.0;

//House Materials

static GLfloat diffuse_house[] = { 0.2f, 0.1f, 0.1f, 1.0f };
static GLfloat ambient_house[] = { 0.2f, 0.0f, 0.0f, 1.0f };
static GLfloat spec_house[] = { 0.0f, 0.0f, 0.0f, 1.0f };	//Matte surface

static GLfloat diffuse_roof[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat ambient_roof[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat spec_roof[] = { 1.0f, 1.0f, 1.0f, 1.0f };	//Metallic surface

//Grass Materials

static GLfloat diffuse_grass[] = { 0.3f, 1.0f, 0.3f, 0.3f };
static GLfloat ambient_grass[] = { 0.3f, 1.0f, 0.3f, 0.0f };
static GLfloat spec_grass[] = { 0.0f, 0.0f, 0.0f, 0.0f };	//Matte surface

//Spotlight Light

static GLfloat diffuse_spotlight[] = { 1.0f , 1.0f, 1.0f, 1.0f };
static GLfloat ambient_spotlight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat spec_spotlight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat position_spotlight[] = { 0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f, 1.0f };
static GLfloat direction_spotlight[] = { 0.0f, -(GLfloat)SQRT_75_PLUS_10, -10.0f, 1.0f };



// ---------------------- SUN IMPLEMENTATION (START) --------------------- //

//Normalization of a given point preserving signage
void normal(point3 p) {
	float d = 0.0f;
	int i;
	for (i = 0; i < 3; i++) 
	{
		d += p[i] * p[i];
	}
	d = (float)sqrt((double)d);
	for (i = 0; i < 3; i++) 
	{
		p[i] /= d;
	}
}

//Recursive sibdivision of a triangle into 4 equilateral triangles
void divide_triangle(point3 a, point3 b, point3 c, int m)
{
	point3 v1, v2, v3;
	int j;

	//Subdivide current triangle
	if (m > 0)
	{
		// Subdivide the triangle into 2 equal parts
		// v1,v2,v3 are the new points created by this subdivision which will form 4 new triangles
		// Normalize these new points so that they move onto the unit sphere
		for (j = 0; j < 3; j++)
		{
			v1[j] = a[j] + b[j];
		}
		normal(v1);
		for (j = 0; j < 3; j++)
		{
			v2[j] = a[j] + c[j];
		}
		normal(v2);
		for (j = 0; j < 3; j++) {
			v3[j] = b[j] + c[j];
		}
		normal(v3);

		//Form said 4 new triangles with the new points and continue recursive subdivision
		divide_triangle(a, v1, v2, m - 1);
		divide_triangle(c, v2, v3, m - 1);
		divide_triangle(b, v3, v1, m - 1);
		divide_triangle(v1, v3, v2, m - 1);
	}

	//Draw final points as polygons onto the unit sphere
	else 
	{
		glBegin(GL_POLYGON);
			glNormal3fv(a);
			glVertex3fv(a);
			glNormal3fv(b);
			glVertex3fv(b);
			glNormal3fv(c);
			glVertex3fv(c);
		glEnd();
	}
}

//Tetrahedron creation and subdivision initiation
void tetrahedron(int m) {
	//Starting values for the tetrahedron
	//These values are inverted so the sun moves opposite to its light source
	point3 v[] = { 
		{0.0f, 0.0f, 1.0f}, 
		{0.0f, 0.942809f, -0.33333f}, 
		{-0.816497f, -0.471405f, -0.333333f}, 
		{0.816497f, -0.471405f, -0.333333f} 
	};
	int i;
	for (i = 0; i < 3; i++)
		position_sun[i] = (v[0][i] + v[1][i] + v[2][i] + v[3][i]) / 4;

	//Take the points of the tetrahedron and divide it into 4 triangles
	//Initiate subdivision on each one of them
	divide_triangle(v[0], v[1], v[2], m);	//First Triangle
	divide_triangle(v[3], v[2], v[1], m);	//Second Triangle
	divide_triangle(v[0], v[3], v[1], m);	//Third Triangle
	divide_triangle(v[0], v[2], v[3], m);	//Fourth Triangle
}

//Update the sun light's color and intensity
void update_sunlight(void) {

	//Sun rotates backwards
	//Start increasing the intensity until the sun reaches the top of the plane
	//Start decreasing the intensity after the sun reaches the top of the plane
	if (sun_angle > -90)
		sunlight += (GLfloat)(0.7 / 90);
	else
		sunlight -= (GLfloat)(0.7 / 90);

	//Update light respectively
	for (int i = 0; i < 3; i++)
	{
		diffuse_sun[i] = sunlight;
		spec_sun[i] = sunlight;
	}

}

void build_sun(void) {

	//Update light attributes
	update_sunlight();


	glPushMatrix();
		//Create sun's materials for color and light
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, sunEmissionMaterial);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
		//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_sun);
		//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_sun);
		//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_sun);


		//Create sun's light as a directional spotlight
		glLightfv(GL_LIGHT0, GL_POSITION, position_sun);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_sun);
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec_sun);


		//Rotate sun on the plane so it resemples dawn and dusk
		glRotated(sun_angle, 0.0, 0.0, 1.0);
		glTranslatef(-50.0, 0.0, 0.0);

		//Create sun's polygons
		tetrahedron(subdivision_count);

	glPopMatrix();

}

// ----------------------- SUN IMPLEMENTATION (END) ---------------------- //

// --------------------- HOUSE IMPLEMENTATION (START) -------------------- //

void build_house(void)
{
	glPushMatrix();

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_house);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_house);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_house);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);

		glPushMatrix();


			//glColor3f(BROWN);
			// EAST wall
			glCallList(EAST_WALL);

			// WEST wall constructed by rotating the East wall by 180 deg @y axis
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glCallList(EAST_WALL);

		glPopMatrix();
		glPushMatrix();

			// South wall
			glCallList(SOUTH_WALL);

			// NORTH wall constructed by rotating the South wall by 180 deg @y axis
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glCallList(SOUTH_WALL);

		glPopMatrix();
	glPopMatrix();

	glPushMatrix();

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_roof);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_roof);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_roof);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0);;

		glPushMatrix();

			//glColor3f(GRAY);
			// EAST roof
			glCallList(EAST_ROOF);

			// WEST roof constructed by rotation the East roof by 180 deg @y axis
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glCallList(EAST_ROOF);

		glPopMatrix();
		glPushMatrix();

			// SOUTH roof
			glCallList(SOUTH_ROOF);

			// NORTH roof constructed by rotation the South roof by 180 deg @y axis
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glCallList(SOUTH_ROOF);

		glPopMatrix();

	glPopMatrix();
}

// ---------------------- HOUSE IMPLEMENTATION (END) --------------------- //

// --------------------- GRASS IMPLEMENTATION (START) -------------------- //

void build_grass(void)
{
	glColor3f(GRASS);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_grass);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_grass);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_grass);
	
	if (polygon_high) {
		glCallLists(POLY_COUNT, GL_UNSIGNED_BYTE, ground_polygons_ids);
	}
	else {
		glCallList(GROUND);
	}
}
// ---------------------- GRASS IMPLEMENTATION (END) --------------------- //

// ------------------- SPOTLIGHT IMPLEMENTATION (START) ------------------ //

void spotlight(void)
{

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, position_spotlight);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction_spotlight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_spotlight);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_spotlight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spec_spotlight);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	
}
// -------------------- SPOTLIGHT IMPLEMENTATION (END) ------------------- //

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-60.0, 60.0, -60.0, 60.0, -300.0, 300.0);
	gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glColor3f(1.0f, 1.0f, 1.0f);

	//House Creation
	build_house();

	//Sun Creation
	build_sun();

	//Grass Creation
	build_grass();
	
	//Spotlight Creation
	if (spotlight_on)
		spotlight();
	else
		glDisable(GL_LIGHT1);

	glutSwapBuffers();
}

void idle(void)
{
	// Update the sun's rotational angle but backwards to resemple correct day transition
	sun_angle -= 0.5;
	if (sun_angle == -180)
		sun_angle = 0;

	glutPostRedisplay();
}

/*	
*	The camera/observer moves on a circle with its center located 
*	on the Y axis, 40 units above the ground (directly above the 
*	house). The circle has a distance of 70 units, thus:
*		C: x^2 + z^2 = 70^2, y = 40
*	Parametric expression:
*		- C: [x(a)]^2 + [z(a)]^2 = 70^2, y = 40
*		- x(a) = r * sin(a)
*		- z(a) = r * cos(a)
*/
void special_key_handler(int key, int x, int y)
{
	static GLdouble angle_rad = 0.0f;
	static const GLdouble ROTATION_STEP = 2.0;

	// RIGHT and LEFT keys change the value of the angle, a.
	if (key == GLUT_KEY_LEFT) {
		// Rotate camera towards positive direction by some deg @y axis
		cam_angle += ROTATION_STEP;
		if (cam_angle >= 360.0) {
			cam_angle -= 360.0;
		}
	}
	else if (key == GLUT_KEY_RIGHT) {
		// Rotate camera towards negative direction by some deg @y axis
		cam_angle -= ROTATION_STEP;
		if (cam_angle <= 0.0) {
			cam_angle += 360.0;
		}
	}
	else return;

	angle_rad = cam_angle * M_PI / 180.0;

	cam_pos[0] = RADIUS_CAMERA * sin(angle_rad);	// x(a)
	cam_pos[2] = RADIUS_CAMERA * cos(angle_rad);	// z(a)

	glutPostRedisplay();
}

void init_lists(void)
{
	// East wall of the house
	glNewList(EAST_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-5.0f, 0.0f, 10.0f);
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(-5.0f, 10.0f, -10.0f);
			glVertex3f(-5.0f, 0.0f, -10.0f);
		}
		glEnd();
	}
	glEndList();
	
	// South wall of the house
	glNewList(SOUTH_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-5.0f, 0.0f, 10.0f);
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 0.0f, 10.0f);
		}
		glEnd();
	}
	glEndList();

	// East roof of the house
	glNewList(EAST_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(-5.0f, 10.0f, -10.0f);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f);
		}
		glEnd();
	}
	glEndList();

	// South roof of the house
	glNewList(SOUTH_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 10.0f, 10.0f);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f);
		}
		glEnd();
	}
	glEndList();

	// Grass polygon
	glNewList(GROUND, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-40.0f, 0.0f, 40.0f);
			glVertex3f(-40.0f, 0.0f, -40.0f);
			glVertex3f(40.0f, 0.0f, -40.0f);
			glVertex3f(40.0f, 0.0f, 40.0f);
		}
		glEnd();
	}
	glEndList();

	// Ground consists of POLY_COUNT = 100 polygons.
	// That means the ground consists of 10 x 10 polygons
	const GLuint EDGE_LENGTH = (GLuint)sqrt((double)POLY_COUNT);

	ground_base_list = glGenLists(POLY_COUNT);
	GLuint i, j;
	GLfloat x_left, x_right;
	GLfloat z_back, z_front;
	
	for (i = 0U; i < EDGE_LENGTH; i++)
	{
		x_left = -40.0f + (GLfloat)(i * 8);
		x_right = -32.0f + (GLfloat)(i * 8);

		for (j = 0U; j < EDGE_LENGTH; j++)
		{
			z_back = -40.0f + (GLfloat)(j * 8);
			z_front = -32.0f + (GLfloat)(j * 8);

			glNewList(ground_base_list + EDGE_LENGTH * i + j, GL_COMPILE);
			{
				glBegin(GL_POLYGON);
				{
					glVertex3f(x_left, 0.0f, z_back);
					glVertex3f(x_right, 0.0f, z_back);
					glVertex3f(x_right, 0.0f, z_front);
					glVertex3f(x_left, 0.0f, z_front);
				}
				glEnd();
			}
			glEndList();
		}
	}
	for (GLuint k = 0; k < POLY_COUNT; k++) {
		ground_polygons_ids[k] = (GLubyte)(ground_base_list + k);
	}
}

void create_menu(void)
{
	polygon_submenu_id = glutCreateMenu(polygon_submenu);
	{
		glutAddMenuEntry("Low", 1);
		glutAddMenuEntry("High", 2);
	}
	spotlight_submenu_id = glutCreateMenu(spotlight_submenu);
	{
		glutAddMenuEntry("On", 3);
		glutAddMenuEntry("Off", 4);
	}
	shade_submenu_id = glutCreateMenu(shade_submenu);
	{
		glutAddMenuEntry("Smooth", 5);
		glutAddMenuEntry("Flat", 6);
	}

	main_menu_id = glutCreateMenu(main_menu);
	{
		glutAddSubMenu("Polygon", polygon_submenu_id);
		glutAddSubMenu("Spotlight", spotlight_submenu_id);
		glutAddSubMenu("Shade", shade_submenu_id);
		glutAddMenuEntry("Exit", 7);
	}
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


int main(int argc, char* argv[])
{
	// Window Creation
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project 3 - Farm House (AEM1: 3713, AEM2: 3938");

	// Attributes
	glEnable(GL_DEPTH_TEST);				// Depth Buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Black Background
	glEnable(GL_LIGHTING);					// Lighting
	glEnable(GL_LIGHT0);					// Light Source
	//glEnable(GL_NORMALIZE);					// Normals Preservation for units
	//glEnable(GL_COLOR_MATERIAL);			// Make glColorf() as Material
	glShadeModel(GL_SMOOTH);				// Smooth Shading Model

	// Pre-compiled lists initialization
	init_lists();

	// Window menu (4 options)
	create_menu();

	// ----------- CALLBACK FUNCTIONS (BEGIN) ----------- //

	glutSpecialFunc(special_key_handler);
	glutDisplayFunc(display);
	glutIdleFunc(idle);

	// ----------- CALLBACK FUNCTIONS (END) ----------- //

	glutMainLoop();

	return EXIT_SUCCESS;
}
