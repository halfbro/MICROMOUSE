#include <QueueList.h>
#include <StackList.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
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
  uint8_t targetPos;
  uint8_t desiredHeading;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

QueueList <instruction> instructions;

//hardware setup that should be done within another file

//define some global constants
#define X 16
#define Y 16
#define oneCell 16 //16 cm is one cell
uint8_t headings [] = {1, 2, 4, 8};
uint8_t globalHeading = 4;
coord globalCoord = {0, 0};
coord globalEnd = {0, 0};

//make the maze!
entry maze [X][Y];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void setup() {

  //HARDWARE INIT

}

/*
  uint8_t senseWalls()
  needs to return "wallReading". This is an 8 bit int representing the open walls. 1 is open 0 is blocked by wall
*/

//Instantiate the reflood maze with the most optimistic values
void instantiateReflood() {
  for (int j = 0; j < Y; j++) {
    for (int i = 0; i < X; i++) {
      maze[j][i].distance = calcCenter(i, j, X);
    }
  }
}

void instantiate() {
  for (int j = 0; j < Y; j++) {
    for (int i = 0; i < X; i++) {
      maze[j][i].distance = calcCenter(i, j, X);
      maze[j][i].walls = 15;
      //If this is the left column (0,x)
      if (i == 0) {
        maze[j][i].walls = 7;
      }
      //if this is the top row
      if (j == 0) {
        maze[j][i].walls = 14;
      }
      //if this is the bottom row
      if (j == (Y - 1)) {
        maze[j][i].walls = 13;
      }
      //If this is the righ column
      if (i == (X - 1)) {
        maze[j][i].walls = 11;
      }
      maze[0][0].walls = 6;
      maze[Y - 1][0].walls = 5;
      maze[0][X - 1].walls = 10;
      maze[X - 1][Y - 1].walls = 9;
    }
  }
}

void resetToCoord(coord desiredCoord) {
  for (int j = 0; j < Y; j++) {
    for (int i = 0; i < X; i++) {
      maze[j][i].distance = calcDist(i, j, desiredCoord.x, desiredCoord.y);
    }
  }
}

//Get the most optimistic distance between two coordinates in a grid
int calcDist(uint8_t posx, uint8_t posy, uint8_t targx, uint8_t targy ) {
  int dist = (int) (abs(targy - posy) + abs(targx - posx));
  return dist;
}

//Get the most optimistic distance between a given coordinate and a
//2x2 square in the center of a maze of dimension dim (dim must be even)
int calcCenter(uint8_t posx, uint8_t posy, uint8_t dim) {
  uint8_t center = dim / 2;
  int dist = 0;

  if (posy < center) {
    if (posx < center) {
      //You're in the top left of the maze
      dist = calcDist(posx, posy, (center - 1), (center - 1));
    } else {
      //You're in the top right of the maze
      dist = calcDist(posx, posy, center, (center - 1));
    }
  } else {
    if (posx >= center) {
      //You're in the bottom right of the maze
      dist = calcDist(posx, posy, center, center);
    } else {
      //You're in the bottom left of the maze
      dist = calcDist(posx, posy, (center - 1), center);
    }
  }
  return dist;
}


//input: a coord representing the current position
//output: the coords of the next desired position based on the heading and current position.... WTF is a heading
coord bearingCoord(coord currCoord, uint8_t heading) {
  coord nextCoord = {0, 0};
  switch (heading) {
    case 1:
      nextCoord.x = currCoord.x;
      nextCoord.y = currCoord.y - 1;
      break;
    case 2:
      nextCoord.x = currCoord.x;
      nextCoord.y = currCoord.y + 1;
      break;
    case 4:
      nextCoord.x = currCoord.x + 1;
      nextCoord.y = currCoord.y;
      break;
    case 8:
      nextCoord.x = currCoord.x - 1;
      nextCoord.y = currCoord.y;
      break;
  }
  return nextCoord;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
/*
  INPUT: A Coord representing the current coordiante and the robots current heading
  OUTPUT: An optimal direction away from the current coordinate.
*/
uint8_t orient(coord currCoord, uint8_t heading) {

  coord leastNext = {0, 0};
  //This is the absolute largest value possible (dimension of maze squared)
  int leastNextVal = sizeof(maze) * sizeof(maze);
  uint8_t leastDir = heading;

  //If there is a bitwise equivalence between the current heading and the cell's value, then the next cell is accessible
  if ((maze[currCoord.x][currCoord.y].walls & heading) != 0) {
    //Define a coordinate for the next cell based onthis heading and set the leastNextVal t its value
    coord leastnextTemp = bearingCoord(currCoord, heading);

    if (checkBounds(leastnextTemp)) {
      leastNext = leastnextTemp;
      leastNextVal = maze[leastNext.y][leastNext.x].distance;
    }
  }

  for (int i = 0; i < sizeof(headings); i++) {
    uint8_t dir = headings[i];
    //if this dir is accessible
    if ((maze[currCoord.y][currCoord.x].walls & dir) != 0) {
      //define the coordiante for this dir
      coord dirCoord = bearingCoord(currCoord, dir);

      if (checkBounds(dirCoord)) {
        //if this dir is more optimal than continuing straight
        if (maze[dirCoord.y][dirCoord.x].distance < leastNextVal) {
          //update teh value of leastNextVal
          leastNextVal = maze[dirCoord.y][dirCoord.x].distance;
          //update the value of leastnext to this dir
          leastNext = dirCoord;
          leastDir = dir;
        }
      }
    }
  }
  return leastDir;
}
//Take a coordinate and test if it is within the allowable bounds
bool checkBounds(coord Coord) {
  if ((Coord.x >= X) || (Coord.y >= Y) || (Coord.x < 0) || (Coord.y < 0)) {
    return false;
  } else {
    return true;
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/*
  Input:coord
  Output: An integer that is the least neighbor
*/
int checkNeighbors(coord Coord) {
  int minVal = sizeof(maze) * sizeof(maze);
  for (int i = 0; i < sizeof(headings); i++) {
    uint8_t dir = headings [i];
    //if this dir is accessible
    if ((maze[Coord.y][Coord.x].walls & dir) != 0 ) {
      //Get the coordinate of the accessible neighbor
      coord neighCoord = bearingCoord(Coord, dir);
      if (checkBounds(neighCoord)) {
        //if the neighbor is less than the current recording minimum value, update the minimum value
        //if minVal is null, set it right away, otherwise test
        if (maze[neighCoord.y][neighCoord.x].distance < minVal) {
          minVal = maze[neighCoord.y][neighCoord.x].distance;
        }
      }
    }
  }
  return minVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//Given the coordinate, test and return if the coordinate is bounded on three sides (dead end)
bool isDead(coord coord) {
  bool deadEnd = false;
  if (checkBounds(coord)) {
    uint8_t bounds = maze[coord.y][coord.x].walls;
    //bounds is the integer from exploratory maze that represents the known walls of the coordinate
    if ((bounds == 1) || (bounds == 2) || (bounds == 4) || (bounds == 8))
      deadEnd = true;
  }
}
bool isEnd (coord Coord, coord DesiredArray[]) {
  bool End = false;
  for (int i = 0; i < sizeof(DesiredArray); i++) {
    coord Desired = DesiredArray[i];
    if (checkBounds(Coord)) {
      if ((Coord.x == Desired.x) && (Coord.y == Desired.y)) {
        End = true;
      }
    }
  }
  return End;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/*
  INPUT: Coordinate to update, and a direction representing the wall to add
  OUTPUT: Update to coordinate adding the wall provided as an argument
*/
void coordUpdate(coord coordinate, uint8_t wallDir) {
  if (checkBounds(coordinate)) {
    if ((maze[coordinate.y][coordinate.x].walls & wallDir) != 0) {
      maze[coordinate.y][coordinate.x].walls = maze[coordinate.y][coordinate.x].walls - wallDir;
    }
  }
}

/*
  INPUT: Current Robot coordinate
  OUTPUT: Update maze for learned walls
*/
void floodFillUpdate(coord currCoord, coord desired[]) {
  StackList<coord> entries;

  maze[currCoord.y][currCoord.x].walls = Sense_Walls(); //the sensewalls action needs to return an unsigned byte
  entries.push(currCoord);

  for (int i = 0; i < sizeof(headings); i++) {
    uint8_t dir = headings [i];
    //If there's a wall in this dir
    if ((maze[currCoord.y][currCoord.x].walls & dir ) == 0 ) {
      //create a temporary working coordinate
      coord workingCoord = {currCoord.x, currCoord.y};
      switch (dir) {
        case 1:
          workingCoord.y = workingCoord.y - 1;
          coordUpdate(workingCoord, 1);
          break;
        case 2:
          workingCoord.y = workingCoord.y - 1;
          coordUpdate(workingCoord, 1);
          break;
        case 4:
          workingCoord.y = workingCoord.y - 1;
          coordUpdate(workingCoord, 1);
          break;
        case 8:
          workingCoord.y = workingCoord.y - 1;
          coordUpdate(workingCoord, 1);
          break;
      }
      //If the working Entry is a valid entry and not a dead end, push it onto the stack
      if (checkBounds(workingCoord) && (!isEnd(workingCoord, desired))) {
        entries.push(workingCoord);
      }
    }
  }
  //While the entries stack is not empty
  while (!entries.isEmpty()) {
    //Pop an entry from the stack
    coord workingEntry = entries.pop();
    int neighCheck = checkNeighbors(workingEntry);
    //If the least neighbor of the working entry is not one less than the value of the wroking entry
    if (neighCheck + 1 != maze[workingEntry.y][workingEntry.x].distance) {
      maze[workingEntry.y][workingEntry.x].distance = neighCheck + 1;
      for (int i = 0; i < sizeof(headings); i++) {
        uint8_t dir = headings[i];
        if ((maze[workingEntry.y][workingEntry.x].walls & dir) != 0) {
          coord nextCoord = bearingCoord(workingEntry, dir);
          if (checkBounds(nextCoord)) {
            if (!isEnd(nextCoord, desired)) {
              entries.push(nextCoord);
            }
          }
        }
      }
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

instruction createInstruction (coord currCoord, coord nextCoord, uint8_t nextHeading) {
  uint8_t desiredHeading = nextHeading;
  /*switch(nextHeading){
    case 1:
      desiredHeading = 1;
    case 2:
      desiredHeading = 2;
    case 4:
      desiredHeading = 4;
    case 8:
      desiredHeading = 8;
    }*/

  instruction turnMove = {oneCell, desiredHeading};
  return turnMove;
}

void executeInstruction (instruction instruct) {
  turn(instruct.desiredHeading);
  moveDist(instruct.targetPos);

}

void floodfill(coord desired[], coord current, bool isMoving) {
  coord currCoord = current;
  uint8_t heading = globalHeading;

  while (maze[currCoord.y][currCoord.x].distance != 0) {
    floodFillUpdate(currCoord, desired);
    uint8_t nextHeading = orient(currCoord, heading);
    coord nextCoord = bearingCoord(currCoord, nextHeading);

    if (isMoving) {
      //Call createInstruction to push a new instruction to the stack
      instructions.push(createInstruction(currCoord, nextCoord, nextHeading));

      //Pop the next instruction from the instructions queue and execute it
      executeInstruction(instructions.pop());
    }

    //After executing the intsturction update the values of the local and global position variables
    currCoord = nextCoord;
    heading = nextHeading;
    //If the robot has actually moved, update the global position variables
    if (isMoving) {
      globalHeading = heading;
      globalCoord = currCoord;
    }
  }
  //Set the global end as the current coordinate
  globalEnd = currCoord;
}

//Call a function based off the desired direction
void turn (uint8_t desiredHeading) {
  if (desiredHeading = 1) {
    turnTowardsNorth();
  }
  else if (desiredHeading = 2) {
    turnTowardsSouth();
  }
  else if (desiredHeading = 4) {
    turnTowardsEast();
  }
  else if (desiredHeading = 8) {
    turnTowardsWest();
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Dummy functions for movement
void moveDist(uint8_t targetPos) {
  return;
}
void turnTowardsNorth() {
  return;
}
void turnTowardsSouth() {
  return;
}
void turnTowardsEast() {
  return;
}
void turnTowardsWest() {
  return;
}
uint8_t Sense_Walls() {
  uint8_t walldir = 15;
  return walldir;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//called after the maze has been figured out and we return to the 0,0 position
void reflood() {
  //Refill the maze for most optimistic values, but now the maze has walls
  instantiateReflood();

  //Run flood fill but without actual motion
  coord desired[] = {{X - 1, Y - 1}, {X - 1, Y}, {X, Y - 1}, {X, Y}};
  coord currCoord = {0, 0};
  floodfill(desired, currCoord, false);

  //Now, the robot is still at the start, but the maze distance values have been updated with the walls discovered
  //So we follow the maze creating instructions
  createSpeedQueue();
  //We now have a queue of instructions.

}

//Trace the maze back to the end creating instructions and adding them to the queue
void createSpeedQueue() {
  coord workingCoord = globalCoord;
  uint8_t workingDir = globalHeading;
  uint8_t workingDist = oneCell;
  while ((workingCoord.x != globalEnd.x) && (workingCoord.y != globalEnd.y)) {
    uint8_t optimalDir = orient(workingCoord, workingDir);

    //If the direction is the same, accumulate distance
    if (optimalDir == workingDir) {
      workingDist += oneCell;
    } else {
      //if the optimal is different from the working, add the working and the accumulated distance
      instruction nextInstruction = {workingDist, optimalDir};
      instructions.push(nextInstruction);
      //Reset the distance to one square and update the workingDir
      workingDist = oneCell;
      workingDir = optimalDir;
    }

    //update workingCoord to the next optimal coord
    workingCoord = bearingCoord(workingCoord, optimalDir);
  }
}


void loop() {

  // set up all serial readings of sensors

  coord desired[] = {{X - 1, Y - 1}, {X - 1, Y}, {X, Y - 1}, {X, Y}};
  floodfill(desired, globalCoord, true);
  coord returnCoord[] = {{0, 0}};
  resetToCoord(returnCoord[0]);
  //Run fill to return to the start coord
  floodfill(returnCoord, globalCoord, true);

  //Reflood the maze
  reflood();

  //Pop instructions from the front of the queue until its empty
  while (!instructions.isEmpty()) {
    executeInstruction(instructions.pop());
  }
}

