
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


void main(int argc, char** argv) {


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("House");
	myinit();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(spin);

	create_menus();


	glutSpecialFunc(keyboard_handler);
	glutMainLoop();




}