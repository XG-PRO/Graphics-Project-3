typedef float point3f[3];


#define SQRT_75_PLUS_10 18.66025403784439

const point3f EAST_WALL_COORDINATES[4] = {
	{-5.0f, 10.0f, -10.0f},
	{ -5.0f, 0.0f, -10.0f},
	{-5.0f, 0.0f, 10.0f},
	{-5.0f, 10.0f, 10.0f}
};
const point3f WEST_WALL_COORDINATES[4] = {
	{5.0f, 10.0f, 10.0f},
	{5.0f, 0.0f, 10.0f},
	{5.0f, 0.0f, -10.0f},
	{5.0f, 10.0f, -10.0f}
};
const point3f SOUTH_WALL_COORDINATES[4] = {
	{-5.0f, 10.0f, 10.0f},
	{-5.0f, 0.0f, 10.0f},
	{5.0f, 0.0f, 10.0f},
	{5.0f, 10.0f, 10.0f}
};
const point3f NORTH_WALL_COORDINATES[4] = {
	{5.0f, 10.0f, -10.0f},
	{5.0f, 0.0f, -10.0f},
	{-5.0f, 0.0f, -10.0f},
	{-5.0f, 10.0f, -10.0f}
};
const point3f EAST_ROOF_COORDINATES[4] = {
	{0.0f, (float)SQRT_75_PLUS_10, -10.0f},
	{-5.0f, 10.0f, -10.0f},
	{-5.0f, 10.0f, 10.0f },
	{0.0f, (float)SQRT_75_PLUS_10, 10.0f}
};
const point3f WEST_ROOF_COORDINATES[4] = {
	{0.0f, (float)SQRT_75_PLUS_10, 10.0f},
	{5.0f, 10.0f, 10.0f},
	{5.0f, 10.0f, -10.0f},
	{0.0f, (float)SQRT_75_PLUS_10, -10.0f}
};
const point3f SOUTH_ROOF_COORDINATES[3] = {
	{0.0f, (float)SQRT_75_PLUS_10, 10.0f},
	{-5.0f, 10.0f, 10.0f},
	{5.0f, 10.0f, 10.0f},
};
const point3f NORTH_ROOF_COORDINATES[3] = {
	{0.0f, (float)SQRT_75_PLUS_10, -10.0f},
	{5.0f, 10.0f, -10.0f},
	{-5.0f, 10.0f, -10.0f},
};
const point3f GROUND_COORDINATES[4] = {
	{-40.0f, 0.0f, -40.0f},
	{-40.0f, 0.0f, 40.0f},
	{40.0f, 0.0f, 40.0f},
	{40.0f, 0.0f, -40.0f}
};
