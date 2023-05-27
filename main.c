#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glut.h>

#define WIDTH  700
#define HEIGHT 700


static volatile bool polygon_high = true;
static volatile bool spotlight_on = true;
static volatile bool smooth_shade = true;


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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
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
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project 3 - Farm House (AEM1: 3713, AEM2: 3938");

	// Attributes
	glEnable(GL_DEPTH_TEST);				// Depth Buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Black Background


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

	glutDisplayFunc(display);
	glutMainLoop();

	return EXIT_SUCCESS;
}
