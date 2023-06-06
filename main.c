#pragma warning(error: 4305)

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glut.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

#define EAST_WALL   0b0001
#define WEST_WALL	0b0010
#define SOUTH_WALL  0b0011
#define NORTH_WALL	0b0100
#define EAST_ROOF   0b0101
#define WEST_ROOF	0b0110
#define SOUTH_ROOF  0b0111
#define NORTH_ROOF	0b1000
#define GROUND      0b1001

#define RADIUS_CAMERA 80.62257748298549

// Must be perfect square (+bonus if its root divides 80)
#define POLY_COUNT 100

#define GRASS   0.00000000f, 0.6039216f, 0.09019608f
#define BROWN   0.36078432f, 0.2509804f, 0.20000000f
#define GRAY    0.30000000f, 0.3000000f, 0.30000000f

#define SQRT_75_PLUS_10 18.66025403784439


// Menu options
volatile bool polygon_high = true;
volatile bool spotlight_on = true;
volatile bool smooth_shade = true;

// Keyboard optiions
volatile bool shadows_on = false;
volatile bool freeze_sun = false;
volatile bool mystery_var = false;


// Menu ids
static int main_menu_id;
static int polygon_submenu_id;
static int spotlight_submenu_id;
static int shade_submenu_id;

// Menu externals
extern void polygon_submenu(int);
extern void spotlight_submenu(int);
extern void shade_submenu(int);
extern void main_menu(int);

// Depth of recursive subdivision for the sun
static GLint subdivision_count = 4;

// List ids for the many polygons that the ground consists of
static GLushort ground_polygons_ids[POLY_COUNT];

static volatile GLdouble cam_pos[] = { 0.0, 40.0, 70.0 };
static volatile GLdouble cam_angle_horizontal = 0.0;
static volatile GLdouble cam_angle_vertical = 29.74488129694223;

// vector3f and point3f only have scemantic differences
typedef GLfloat point3f[3];
typedef GLfloat vector3f[3];


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
static GLfloat sunEmissionMaterial[] = { 0.2f, 0.2f, 0.0f, 1.0f };
static GLfloat diffuse_sun[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat spec_sun[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat position_sun[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat sun_angle = 0.0;

//House Materials
static GLfloat diffuse_house[] = { 0.3f, 0.0f, 0.0f, 1.0f };
static GLfloat ambient_house[] = { 0.3f, 0.0f, 0.0f, 1.0f };
static GLfloat spec_house[] = { 0.0f, 0.0f, 0.0f, 1.0f };	//Matte surface

static GLfloat diffuse_roof[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat ambient_roof[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static GLfloat spec_roof[] = { 1.0f, 1.0f, 1.0f, 1.0f };	//Metallic surface

//Grass Materials
static GLfloat diffuse_grass[] = { 0.3f, 1.0f, 0.3f, 0.3f }; // 0.3 last param?
static GLfloat ambient_grass[] = { 0.3f, 1.0f, 0.3f, 0.0f };
static GLfloat spec_grass[] = { 0.0f, 0.0f, 0.0f, 0.0f };	//Matte surface

//Spotlight Light
static GLfloat diffuse_spotlight[] = { 1.0f , 1.0f, 1.0f, 1.0f };
static GLfloat ambient_spotlight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat spec_spotlight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat position_spotlight[] = { 0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f, 1.0f };
static GLfloat direction_spotlight[] = { 0.0f, -(GLfloat)SQRT_75_PLUS_10, 10.0f, 1.0f };

/*
*	Suppose three points p0, p1, p2.
*	The first parameter is assigned the cross product:
*		(p2 - p1) x (p0 - p1)
*	The next parameters are the coordinates of the three points.
*	To get the outer product of a triangle's sides, they must be 
*	inserted clockwise in order for it to point out of the object.
*/
void cross_product(vector3f out, 
	GLfloat p00, GLfloat p01, GLfloat p02,
	GLfloat p10, GLfloat p11, GLfloat p12,
	GLfloat p20, GLfloat p21, GLfloat p22)
{
	vector3f v2 = { p20 - p10, p21 - p11, p22 - p12 };
	vector3f v1 = { p00 - p10, p01 - p11, p02 - p12 };

	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

GLfloat shadow_check(GLfloat var, GLfloat limit1, GLfloat limit2) 
{
	if (var < limit1)
		return limit1;
	if (var > limit2)
		return limit2;
	return var;
}

void getShadow4f(point3f polygon_shadow[4],
	GLfloat ax, GLfloat ay, GLfloat az,
	GLfloat bx, GLfloat by, GLfloat bz,
	GLfloat cx, GLfloat cy, GLfloat cz,
	GLfloat dx, GLfloat dy, GLfloat dz)
{
	GLdouble sun_angle_rad = (GLdouble)(-sun_angle) * M_PI / 180.0;
	GLfloat X;
	GLfloat Z;
	GLfloat t;

	GLfloat Lx = -50.0f * (GLfloat)cos(sun_angle_rad);
	GLfloat Ly = 50.0f * (GLfloat)sin(sun_angle_rad);;

	t = -Ly / (ay - Ly);
	X = Lx + t * (ax - Lx);
	Z = -t * az;
	polygon_shadow[0][0] = shadow_check(X, -40.0f, 40.0f);
	polygon_shadow[0][1] = 0.1f;
	polygon_shadow[0][2] = shadow_check(Z, -40.0f, 40.0f);

	t = -Ly / (by - Ly);
	X = Lx + t * (bx - Lx);
	Z = -t * bz;
	polygon_shadow[1][0] = shadow_check(X, -40.0f, 40.0f);
	polygon_shadow[1][1] = 0.1f;
	polygon_shadow[1][2] = shadow_check(Z, -40.0f, 40.0f);

	t = -Ly / (cy - Ly);
	X = Lx + t * (cx - Lx);
	Z = -t * cz;
	polygon_shadow[2][0] = shadow_check(X, -40.0f, 40.0f);
	polygon_shadow[2][1] = 0.1f;
	polygon_shadow[2][2] = shadow_check(Z, -40.0f, 40.0f);

	t = -Ly / (dy - Ly);
	X = Lx + t * (dx - Lx);
	Z = -t * dz;
	polygon_shadow[3][0] = shadow_check(X, -40.0f, 40.0f);
	polygon_shadow[3][1] = 0.1f;
	polygon_shadow[3][2] = shadow_check(Z, -40.0f, 40.0f);
}

// ---------------------- SUN IMPLEMENTATION (START) --------------------- //

//Normalization of a given point/vector preserving signage
void normal(GLfloat p[3]) {
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
void divide_triangle(point3f a, point3f b, point3f c, int m)
{
	vector3f cross;
	point3f v1, v2, v3;
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
		for (j = 0; j < 3; j++)
		{
			v2[j] = a[j] + c[j];
		}
		for (j = 0; j < 3; j++) {
			v3[j] = b[j] + c[j];
		}
		if (!mystery_var) {
			normal(v1);
			normal(v2);
			normal(v3);
		}

		//Form said 4 new triangles with the new points and continue recursive subdivision
		divide_triangle(a, v1, v2, m - 1);
		divide_triangle(c, v2, v3, m - 1);
		divide_triangle(b, v3, v1, m - 1);
		divide_triangle(v1, v3, v2, m - 1);
	}

	//Draw final points as polygons onto the unit sphere
	else
	{
		cross_product(cross,
			c[0], c[1], c[2],
			a[0], a[1], a[2],
			b[0], b[1], b[2]
		);

		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3fv(a);
			glVertex3fv(b);
			glVertex3fv(c);
		}
		glEnd();
	}
}

//Tetrahedron creation and subdivision initiation
void tetrahedron(int m) {
	//Starting values for the tetrahedron
	//These values are inverted so the sun moves opposite to its light source
	point3f v[] = {
		{0.0000000f, 0.0000000f, 1.0000000f},
		{0.0000000f, 0.9428090f, -0.333333f},
		{-0.816497f, -0.471405f, -0.333333f},
		{0.8164970f, -0.471405f, -0.333333f}
	};

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
		sunlight += 0.7f / 90;
	else
		sunlight -= 0.7f / 90;

	//Update light respectively
	for (int i = 0; i < 3; i++)
	{
		diffuse_sun[i] = sunlight;
		spec_sun[i] = sunlight;
	}
}

// Physical manifestation of the Sun
void build_sun(void) {

	//Update light attributes
	if (!freeze_sun)
		update_sunlight();

	glPushMatrix();
	{
		//Create sun's materials for color and light
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, sunEmissionMaterial);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);

		//Rotate sun on the plane so it resemples dawn and dusk
		glRotated(sun_angle, 0.0, 0.0, 1.0);
		glTranslatef(-50.0, 0.0, 0.0);

		//Create sun's polygons
		tetrahedron(subdivision_count);

		//Create sun's light as a directional spotlight
		glLightfv(GL_LIGHT0, GL_POSITION, position_sun);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_sun);
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec_sun);
	}
	glPopMatrix();
}
// ---------------------- SUN IMPLEMENTATION (END) --------------------- //

// Physical manifestation of the House
void build_house(void)
{
	glPushMatrix();
	{
		glMaterialfv(GL_FRONT, GL_SPECULAR, spec_house);
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_house);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_house);
		glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);

		glCallList(EAST_WALL);
		glCallList(WEST_WALL);
		glCallList(SOUTH_WALL);
		glCallList(NORTH_WALL);

		glMaterialfv(GL_FRONT, GL_SPECULAR, spec_roof);
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_roof);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_roof);
		glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

		glCallList(EAST_ROOF);
		glCallList(WEST_ROOF);
		glCallList(SOUTH_ROOF);
		glCallList(NORTH_ROOF);
	}
	glPopMatrix();
}

// Physical manifestation of the Grass
void build_grass(void)
{
	glColor3f(GRASS);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_grass);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec_grass);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_grass);

	if (polygon_high) {
		// 100 polygons
		glCallLists(POLY_COUNT, GL_UNSIGNED_SHORT, ground_polygons_ids);
	}
	else {
		// 1 big polygon
		glCallList(GROUND);
	}
}

// Physical manifestation of the Shadows
void cast_shadows(void)
{
	point3f east_wall_shadow[4];
	point3f south_wall_shadow[4];
	point3f north_wall_shadow[4];
	point3f west_wall_shadow[4];
	point3f east_roof_shadow[4];
	point3f west_roof_shadow[4];
	point3f south_roof_shadow[4];
	point3f north_roof_shadow[4];

	if (sun_angle < -170.0f || sun_angle > -10.0f) {
		return;
	}
	
	getShadow4f(east_wall_shadow,
		-5.0f, 10.0f, -10.0f,
		-5.0f, 0.0f, -10.0f,
		-5.0f, 0.0f, 10.0f,
		-5.0f, 10.0f, 10.0f
	);
	getShadow4f(west_wall_shadow,
		5.0f, 10.0f, 10.0f,
		5.0f, 0.0f, 10.0f,
		5.0f, 0.0f, -10.0f,
		5.0f, 10.0f, -10.0f
	);
	getShadow4f(south_wall_shadow,
		-5.0f, 10.0f, 10.0f,
		-5.0f, 0.0f, 10.0f,
		5.0f, 0.0f, 10.0f,
		5.0f, 10.0f, 10.0f
	);
	getShadow4f(north_wall_shadow,
		5.0f, 10.0f, -10.0f,
		5.0f, 0.0f, -10.0f,
		-5.0f, 0.0f, -10.0f,
		-5.0f, 10.0f, -10.0f
	);
	getShadow4f(east_roof_shadow,
		0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f,
		-5.0f, 10.0f, -10.0f,
		-5.0f, 10.0f, 10.0f,
		0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f
	);
	getShadow4f(west_roof_shadow,
		0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f,
		5.0f, 10.0f, 10.0f,
		5.0f, 10.0f, -10.0f,
		0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f
	);
	getShadow4f(south_roof_shadow,
		0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f,
		-5.0f, 10.0f, 10.0f,
		5.0f, 10.0f, 10.0f,
		5.0f, 10.0f, 10.0f
	);
	getShadow4f(north_roof_shadow,
		0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f,
		5.0f, 10.0f, -10.0f,
		-5.0f, 10.0f, -10.0f,
		-5.0f, 10.0f, -10.0f
	);
	
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);

	glBegin(GL_POLYGON);
	{
		glVertex3fv(east_wall_shadow[0]);
		glVertex3fv(east_wall_shadow[1]);
		glVertex3fv(east_wall_shadow[2]);
		glVertex3fv(east_wall_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(west_wall_shadow[0]);
		glVertex3fv(west_wall_shadow[1]);
		glVertex3fv(west_wall_shadow[2]);
		glVertex3fv(west_wall_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(south_wall_shadow[0]);
		glVertex3fv(south_wall_shadow[1]);
		glVertex3fv(south_wall_shadow[2]);
		glVertex3fv(south_wall_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(north_wall_shadow[0]);
		glVertex3fv(north_wall_shadow[1]);
		glVertex3fv(north_wall_shadow[2]);
		glVertex3fv(north_wall_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(east_roof_shadow[0]);
		glVertex3fv(east_roof_shadow[1]);
		glVertex3fv(east_roof_shadow[2]);
		glVertex3fv(east_roof_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(west_roof_shadow[0]);
		glVertex3fv(west_roof_shadow[1]);
		glVertex3fv(west_roof_shadow[2]);
		glVertex3fv(west_roof_shadow[3]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(south_roof_shadow[0]);
		glVertex3fv(south_roof_shadow[1]);
		glVertex3fv(south_roof_shadow[2]);
	}
	glEnd();

	glBegin(GL_POLYGON);
	{
		glVertex3fv(north_roof_shadow[0]);
		glVertex3fv(north_roof_shadow[1]);
		glVertex3fv(north_roof_shadow[2]);
	}
	glEnd();
}

// Spotlight implementation
void spotlight(void)
{
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, position_spotlight);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction_spotlight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse_spotlight);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient_spotlight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spec_spotlight);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-60.0, 60.0, -60.0, 60.0, -300.0, 300.0);
	//	Camera is positioned on the sphere:
	//		x^2 + y^2 + z^2 = 6500
	gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (smooth_shade)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	//House Creation
	build_house();

	//Sun Creation
	build_sun();

	// Projection shadows
	if (shadows_on)
		cast_shadows();

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
	if (!freeze_sun)
		sun_angle -= 0.5f;
	if (sun_angle <= -180.0f)
		sun_angle = 0;

	glutPostRedisplay();
}

/*
*	The camera/observer moves on a sphere with its center being 
*	the origin of the axes. The sphere has a distance of 10 * sqrt(65)
*	units, thus:
*		C: x^2 + y^2 z^2 = 6500
*	Parametric expression:
*		- C: [x(u,v)]^2 + [y(u,v)]^2 [z(u,v)]^2 = 6500
*		- x(u,v) = r * cos(u) * sin(v)
*		- y(u,v) = r * sin(u)
*		- z(u,v) = r * cos(u) * cos(v)
* 
*	The camera's initial location is at (0, 40, 70).
*	The circle x^2 + z^2 = 70^2, y = 40 is entirely
*	contained into the aforementioned sphere.
*/
void special_key_handler(int key, int x, int y)
{
	static GLdouble angle_rad_horizontal = 0.0f;
	static GLdouble angle_rad_vertical = 0.0f;
	static const GLdouble ROTATION_STEP = 2.0;

	// RIGHT and LEFT keys change the value of the angle v.
	if (key == GLUT_KEY_LEFT) {
		// Rotate camera towards positive direction by some deg @y axis
		cam_angle_horizontal += ROTATION_STEP;
		if (cam_angle_horizontal >= 360.0) {
			cam_angle_horizontal -= 360.0;
		}
	}
	else if (key == GLUT_KEY_RIGHT) {
		// Rotate camera towards negative direction by some deg @y axis
		cam_angle_horizontal -= ROTATION_STEP;
		if (cam_angle_horizontal <= 0.0) {
			cam_angle_horizontal += 360.0;
		}
	}
	// UP and DOWN keys change the value of the angle u.
	else if (key == GLUT_KEY_UP) {
		// Rotate camera towards negative direction by some deg @y axis
		cam_angle_vertical += ROTATION_STEP;
		if (cam_angle_vertical >= 360.0) {
			cam_angle_vertical -= 360.0;
		}
	}
	else if (key == GLUT_KEY_DOWN) {
		// Rotate camera towards negative direction by some deg @y axis
		cam_angle_vertical -= ROTATION_STEP;
		if (cam_angle_vertical <= 0.0) {
			cam_angle_vertical += 360.0;
		}
	}
	else return;

	angle_rad_horizontal = cam_angle_horizontal * M_PI / 180.0;	// v angle
	angle_rad_vertical = cam_angle_vertical * M_PI / 180.0;		// u angle

	// x(u, v) = 70 * cosu * sinv
	cam_pos[0] = RADIUS_CAMERA * cos(angle_rad_vertical) * sin(angle_rad_horizontal);
	// y(u, v) = 70 * sinu
	cam_pos[1] = RADIUS_CAMERA * sin(angle_rad_vertical);
	// z(u, v) = 70 * cosu * cosv
	cam_pos[2] = RADIUS_CAMERA * cos(angle_rad_vertical) * cos(angle_rad_horizontal);

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 's' || key == 'S') {
		shadows_on = !shadows_on;
	}
	else if (key == 'f' || key == 'F') {
		freeze_sun = !freeze_sun;
	}
	else if (key == 'p' || key == 'P') {
		mystery_var = !mystery_var;
	}
}

void init_lists(void)
{
	vector3f cross;

	cross_product(cross, 
		-5.0f, 10.0f, -10.0f, 
		-5.0f, 10.0f, 10.0f,
		-5.0f, 0.0f, 10.0f
	);
	normal(cross);

	// East wall of the house
	glNewList(EAST_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(-5.0f, 10.0f, -10.0f);
			glVertex3f(-5.0f, 0.0f, -10.0f);
			glVertex3f(-5.0f, 0.0f, 10.0f);
			glVertex3f(-5.0f, 10.0f, 10.0f);
		}
		glEnd();
	}
	glEndList();

	cross[0] = -cross[0];

	// West wall of the house
	glNewList(WEST_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 0.0f, 10.0f);
			glVertex3f(5.0f, 0.0f, -10.0f);
			glVertex3f(5.0f, 10.0f, -10.0f);
		}
		glEnd();
	}
	glEndList();

	cross_product(cross,
		5.0f, 10.0f, 10.0f,
		5.0f, 0.0f, 10.0f,
		-5.0f, 0.0f, 10.0f
	);
	normal(cross);

	// South wall of the house
	glNewList(SOUTH_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(-5.0f, 0.0f, 10.0f);
			glVertex3f(5.0f, 0.0f, 10.0f);
			glVertex3f(5.0f, 10.0f, 10.0f);
		}
		glEnd();
	}
	glEndList();

	cross[2] = -cross[2];

	// North wall of the house
	glNewList(NORTH_WALL, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(5.0f, 10.0f, -10.0f);
			glVertex3f(5.0f, 0.0f, -10.0f);
			glVertex3f(-5.0f, 0.0f, -10.0f);
			glVertex3f(-5.0f, 10.0f, -10.0f);
		}
		glEnd();
	}
	glEndList();

	cross_product(cross,
		-5.0f, 10.0f, -10.0f,
		0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f,
		0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f
	);
	normal(cross);

	// East roof of the house
	glNewList(EAST_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f);
			glVertex3f(-5.0f, 10.0f, -10.0f);
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f);
		}
		glEnd();
	}
	glEndList();

	cross[0] = -cross[0];
	
	// West roof of the house
	glNewList(WEST_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f);
			glVertex3f(5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 10.0f, -10.0f);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f);
		}
		glEnd();
	}
	glEndList();

	cross_product(cross,
		0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f,
		5.0f, 10.0f, 10.0f,
		-5.0f, 10.0f, 10.0f
	);
	normal(cross);

	// South roof of the house
	glNewList(SOUTH_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, 10.0f);
			glVertex3f(-5.0f, 10.0f, 10.0f);
			glVertex3f(5.0f, 10.0f, 10.0f);
		}
		glEnd();
	}
	glEndList();
	
	cross[2] = -cross[2];

	// North roof of the house
	glNewList(NORTH_ROOF, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(0.0f, (GLfloat)SQRT_75_PLUS_10, -10.0f);
			glVertex3f(5.0f, 10.0f, -10.0f);
			glVertex3f(-5.0f, 10.0f, -10.0f);
		}
		glEnd();
	}
	glEndList();

	cross_product(cross,
		-40.0f, 0.0f, 40.0f,
		-40.0f, 0.0f, -40.0f,
		40.0f, 0.0f, -40.0f
	);
	normal(cross);

	// Grass polygon
	glNewList(GROUND, GL_COMPILE);
	{
		glBegin(GL_POLYGON);
		{
			glNormal3fv(cross);
			glVertex3f(-40.0f, 0.0f, -40.0f);
			glVertex3f(-40.0f, 0.0f, 40.0f);
			glVertex3f(40.0f, 0.0f, 40.0f);
			glVertex3f(40.0f, 0.0f, -40.0f);
		}
		glEnd();
	}
	glEndList();

	// Ground consists of POLY_COUNT = 100 polygons.
	// That means the ground consists of 10 x 10 polygons
	const GLuint EDGE_LENGTH = (GLuint)sqrt((double)POLY_COUNT);
	const GLfloat SUB_POLY_SIDE = 80.0f / EDGE_LENGTH;

	GLuint base = glGenLists(POLY_COUNT);
	GLuint i, j;
	GLfloat x_left, x_right;
	GLfloat z_back, z_front;

	for (i = 0U; i < EDGE_LENGTH; i++)
	{
		x_left = -40.0f + (GLfloat)(i * SUB_POLY_SIDE);
		x_right = -40.0f + (GLfloat)((i + 1) * SUB_POLY_SIDE);

		for (j = 0U; j < EDGE_LENGTH; j++)
		{
			z_back = -40.0f + (GLfloat)(j * SUB_POLY_SIDE);
			z_front = -40.0f + (GLfloat)((j + 1) * SUB_POLY_SIDE);

			glNewList(base + EDGE_LENGTH * i + j, GL_COMPILE);
			{
				glBegin(GL_POLYGON);
				{
					glNormal3fv(cross);
					glVertex3f(x_left, 0.0f, z_back);
					glVertex3f(x_left, 0.0f, z_front);
					glVertex3f(x_right, 0.0f, z_front);
					glVertex3f(x_right, 0.0f, z_back);
				}
				glEnd();
			}
			glEndList();
		}
	}
	for (GLuint k = 0; k < POLY_COUNT; k++) {
		ground_polygons_ids[k] = (GLushort)(base + k);
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
	//glEnable(GL_NORMALIZE);				// Normals Preservation for units
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
	glutKeyboardFunc(keyboard);

	// ----------- CALLBACK FUNCTIONS (END) ----------- //

	glutMainLoop();

	return EXIT_SUCCESS;
}