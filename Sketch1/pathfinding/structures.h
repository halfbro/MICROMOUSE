
#include <stdint.h>

typedef uint8_t Maze [16][16];

enum ActionType {
	STOP_RUN = 0,
	MOVE_FORWARD,
	TURN_CW,
	TURN_CCW,
	SENSE_WALLS,
	AWAIT_INPUT,
};

struct Action {
	enum Actiontype type;
	int8_t data;
};

struct coord
{
	uint8_t x;
	uint8_t y;
};

struct entry
{
	int distance;
	uint8_t walls;
};

struct instruction
{
	float targetPos;
	float desiredHeading;
};