#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


extern volatile bool polygon_high;
extern volatile bool spotlight_on;
extern volatile bool smooth_shade;

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
