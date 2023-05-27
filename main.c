#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glut.h>

#define WINDOW_WIDTH  700
#define WINDOW_HEIGHT 700

#define EAST_WALL	0b001
#define SOUTH_WALL	0b010
#define EAST_ROOF	0b011
#define SOUTH_ROOF	0b100

#define HEIGHT_CAMERA 4.0
#define RADIUS_CAMERA 6.0

#define ROTATION_STEP 3.0

#define WHITE   1.000f, 1.000f, 1.000f
#define RED     1.000f, 0.000f, 0.000f
#define YELLOW  1.000f, 1.000f, 0.000f
#define GREEN   0.000f, 1.000f, 0.000f
#define BLUE    0.000f, 0.000f, 1.000f
#define CYAN    0.000f, 1.000f, 1.000f
#define PURPLE  0.300f, 0.216f, 0.380f
#define ORANGE  1.000f, 0.400f, 0.000f
#define FOREST  0.272f, 0.860f, 0.672f


typedef GLdouble vector3lf[3];
typedef GLfloat  vector3f[3];

static volatile bool polygon_high = true;
static volatile bool spotlight_on = true;
static volatile bool smooth_shade = true;

static volatile vector3lf cam_pos = { 0.0f, HEIGHT_CAMERA, RADIUS_CAMERA };
static volatile GLdouble cam_angle = 0.0f;


// ---------------------- MENU IMPLEMENTATION (BEGIN) ---------------------- //
// First Sub-menu
void polygon_submenu(int op_id)
{
	switch (op_id)
	{
	case 1:
		polygon_high = false;
		printf("Polygon set to \"Low\"\n");
		break;
	case 2:
		polygon_high = true;
		printf("Polygon set to \"High\"\n");
		break;
	default:
		fprintf(stderr, "Undefined Polygon Option\n");
		break;
	}
}

// Second Sub-menu
void spotlight_submenu(int op_id)
{
	switch (op_id)
	{
	case 3:
		spotlight_on = true;
		printf("Spotlight enabled\n");
		break;
	case 4:
		spotlight_on = false;
		printf("Spotlight disabled\n");
		break;
	default:
		fprintf(stderr, "Undefined Spotlight Option\n");
		break;
	}
}

// Third Sub-menu
void shade_submenu(int op_id)
{
	switch (op_id)
	{
	case 5:
		smooth_shade = true;
		printf("Smooth Shading enabled\n");
		break;
	case 6:
		smooth_shade = false;
		printf("Smooth Shading disabled\n");
		break;
	default:
		fprintf(stderr, "Undefined Shading Option\n");
		break;
	}
}

// Anchor menu
void main_menu(int op_id)
{
	if (op_id == 7) {
		exit(EXIT_SUCCESS);
	}
}
// ---------------------- MENU IMPLEMENTATION (END) ---------------------- //


void display(void)
{
	// Projection cube's half-length of edge
	static const GLdouble b = 10.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-b, b, -b, b, -b, b);
	gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);

	// ------------------- HOUSE CONSTRUCTION (BEGIN) -------------------
	glPushMatrix();

	glColor3f(RED);
	// EAST wall
	glCallList(EAST_WALL);

	// WEST wall constructed by rotating the East wall by 180 deg @y axis
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(ORANGE);
	glCallList(EAST_WALL);

	glPopMatrix();
	glPushMatrix();

	glColor3f(YELLOW);
	// South wall
	glCallList(SOUTH_WALL);

	// NORTH wall constructed by rotating the South wall by 180 deg @y axis
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(CYAN);
	glCallList(SOUTH_WALL);

	glPopMatrix();
	glPushMatrix();

	glColor3f(FOREST);
	// EAST roof
	glCallList(EAST_ROOF);

	// WEST roof constructed by rotation the East roof by 180 deg @y axis
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(GREEN);
	glCallList(EAST_ROOF);
	
	glPopMatrix();
	glPushMatrix();

	glColor3f(BLUE);
	// SOUTH roof
	glCallList(SOUTH_ROOF);

	// NORTH roof constructed by rotation the South roof by 180 deg @y axis
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glColor3f(PURPLE);
	glCallList(SOUTH_ROOF);

	glPopMatrix();
	// ------------------- HOUSE CONSTRUCTION (END) -------------------

	glutSwapBuffers();
}

void special_key_handler(int key, int x, int y)
{
	static GLdouble angle_rad = 0.0f;

	if (key == GLUT_KEY_RIGHT) {
		// Rotate camera towards positive direction by some deg @y axis
		cam_angle += ROTATION_STEP;
		if (cam_angle >= 360.0) {
			cam_angle -= 360.0;
		}
	}
	else if (key == GLUT_KEY_LEFT) {
		// Rotate camera towards negative direction by some deg @y axis
		cam_angle -= ROTATION_STEP;
		if (cam_angle <= 0.0) {
			cam_angle += 360.0;
		}
	}
	else return;

	angle_rad = cam_angle * M_PI / 180.0;

	cam_pos[0] = RADIUS_CAMERA * sin(angle_rad);
	cam_pos[2] = RADIUS_CAMERA * cos(angle_rad);

	glutPostRedisplay();
}

void init_lists(void)
{
	// East wall of the house
	glNewList(EAST_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-1.0f, 0.0f, 2.0f);
			glVertex3f(-1.0f, 2.0f, 2.0f);
			glVertex3f(-1.0f, 2.0f, -2.0f);
			glVertex3f(-1.0f, 0.0f, -2.0f);
		}
		glEnd();
	}
	glEndList();
	
	// South wall of the house
	glNewList(SOUTH_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-1.0f, 0.0f, 2.0f);
			glVertex3f(-1.0f, 2.0f, 2.0f);
			glVertex3f(1.0f, 2.0f, 2.0f);
			glVertex3f(1.0f, 0.0f, 2.0f);
		}
		glEnd();
	}
	glEndList();

	// East roof of the house
	glNewList(EAST_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-1.0f, 2.0f, 2.0f);
			glVertex3f(0.0f, 4.0f, 2.0f);
			glVertex3f(0.0f, 4.0f, -2.0f);
			glVertex3f(-1.0f, 2.0f, -2.0f);
		}
		glEnd();
	}
	glEndList();

	// East roof of the house
	glNewList(SOUTH_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glVertex3f(-1.0f, 2.0f, 2.0f);
			glVertex3f(1.0f, 2.0f, 2.0f);
			glVertex3f(0.0f, 4.0f, 2.0f);
		}
		glEnd();
	}
	glEndList();
}

int main(int argc, char* argv[])
{
	int main_menu_id;
	int polygon_submenu_id;
	int spotlight_submenu_id;
	int shade_submenu_id;

	// Window Creation
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project 3 - Farm House (AEM1: 3713, AEM2: 3938");

	// Attributes
	glEnable(GL_DEPTH_TEST);				// Depth Buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Black Background

	// Pre-compiled lists initialization
	init_lists();

	// ---------------------- MENU CREATION (BEGIN) ---------------------- //
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
	// ---------------------- MENU CREATION (END) ---------------------- //

	glutSpecialFunc(special_key_handler);
	glutDisplayFunc(display);

	// ...

	glutMainLoop();

	return EXIT_SUCCESS;
}
