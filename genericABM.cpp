// Author: Jim Burchfield
// April 13, 2021
//
// A simple agent-based model framework using OpenGL for graphical output.
// Includes an agent, a set of environment layers, and some control aspects.
// Agents are allowed to move freely through continuous floating point space, 
// but the environment is a discrete grid for processing efficiency.
//
// Currently it does nothing, feel free to use it.
//
//
#include "GL/freeglut.h"
#include "GL/gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#define NAGENTS 1000
#define NLAYERS 5
#define GRIDRES 0.03
#define PI 3.14159265359
#define SPEED 0.005

using namespace std;

class ABMagent {
  // Our main actor in the simulation is the agent. Code any behaviours
  // that are independent of the environment here. Examples include
  // searching, resting, or moving.
  //
  // For simplicity and the smoothness of our graphical output our agents
  // will be coded to move in the world coordinates. These are floating
  // point numbers between -1.0 and 1.0, with { 0 , 0 } at the centre.
  // 
  protected:
  public:
    float x;
    float y;
    float angle;
    int state;
    bool active;
    ABMagent() {
      x = 0.0;
      y = 0.0;
      state = 0;
      active = true;
    }

    bool act() {
      // State-space representation of actions that can be
      // taken by the agent depending on its current state.
      // I recommend coding and invoking separate functions
      // for these so that you can invoke them from elsewhere
      // but nothing will stop you from having actions be
      // taken in the switch block.
      bool status = false;
      switch(state) {
        case 0:
          // Do something
          status = true;
          break;
        case 1:
          // Do something else
          status = true;
          break;
        default:
          // Do nothing
          break;
      }
      return status;
    }

    void setAngle (float theta) {
      // Allow the angle to be set from outside the function.
      // Check to make sure it is between 0 and 2pi radians.
      angle = theta;
      while(fixAngle()) {
      }
    }

    bool fixAngle() {
      // Checks if the angle is less than zero or greater 
      // than 2pi radians. Adds or subtracts 2pi radians as
      // appropriate. Returns true if the angle is in range,
      // returns false if the angle is adjusted.
      if(angle > 2 * PI) {
        angle -= 2 * PI;
        return false;
      } else if(angle < 0); {
        angle += 2 * PI;
        return false;
      }
      return true;
    }

    void move() {
      // Move behaviour. Moves the agent by SPEED in the
      // direction of angle.
      float i = cos(angle) * SPEED;
      float j = sin(angle) * SPEED;
      if(abs(i) < 1 && abs(j) < 1) {
        x += i;
        y += j;
      }
    }

    float rnorm() {
      // We use the Box-Muller transform to generate a normally
      // distributed random number.
      float u1 = rand() / (double)RAND_MAX;
      float u2 = rand() / (double)RAND_MAX;
      float Z = sqrt(-2 * log(u1)) * cos(2 * PI * u2);
      return Z;
    }

    void search() {
      // Searching behaviour. Need to find food something?
      // Randomly modifies the agent's direction and then
      // moves.
      //
      // If something is obstructing the agent, reject the
      // move in the controller code.
      // 
       float deflection = rnorm() * 0.25 * PI;
      setAngle(angle + deflection);
      move();
    }
};
// Instatiate the number of agents that we want in our simulation.
ABMagent ABMagents[NAGENTS];

class environment {
  // Our environment object will consist of a number of grids containing 
  // environmental variables of interest. The number of cells will be
  // determined by GRIDRES and the number of layers is NLAYERS.
  //
  // By default, most of the values are protected here so you will need
  // to use functions to get most of them. Some more environmentally
  // relevant behaviours and behaviors that modify the environment can 
  // be coded here as well.
  protected:
    int bound = 1 + 2/GRIDRES;
    int grid[int(1+2/GRIDRES)][int(1+2/GRIDRES)][NLAYERS];
    float colour[NLAYERS][3];
  public:
    environment() {
      // Our default constructor picks a random colour for
      // each layer and sets up NLAYERS blank layers. You'll
      // probably want to update the grid manually or read 
      // in the relevant data from files.
      srand(time(NULL));
      for(int i = 0; i < NLAYERS; i++) { // Layer
        for(int j = 0; j < 3; j++) {
          colour[i][j] = rand() % 100/100.0;
        }
        for(int j = 0; j < bound; j++) { // X
        for(int k = 0; k < bound; k++) { // Y
          grid[j][k][i] = 0;
        }
        }
      }
    }
    int gridCoord(float x) {
      // Returns the nearest grid coordinate to a given world
      // coordinate. Since our grid is square, this works for
      // both x and y coordinates.
      return int(round((1 + x)/GRIDRES));
    }
    float worldCoord(int x) {
      // Returns the world coordinate of a given grid
      // coordinate. Since our grid is square, this works for
      // both x and y coordinates.
      return x * GRIDRES - 1;
    }
    bool inspect(int x, int y, int layer) {
      // Given two grid coordinates and a layer, check if the
      // value in the grid layer at that coordinate is
      // non-zero.
      if (x * y < 0 || x >= bound || y >= bound) return false;
      bool yes = grid[x][y][layer] != 0;
      return yes;
    }
    bool check(float x, float y, int layer) {
      // ibid. Takes coordinates in world coordinates instead
      // of grid coordinates.
      bool yes = inspect(gridCoord(x), gridCoord(y), layer);
      return yes;
    }
    bool alter(int x, int y, int layer, bool increase) {
      // Checks to make sure the given coordinate is in-grid,
      // increments the specified cell by 1 if increase is 
      // true or decrements it by 1 if increase is false.
      // Returns false if the input is out of bounds.
      if (x * y > 0 && x < bound && y < bound) {
        grid[x][y][layer] += increase * 1 - !increase * 1;
        return true;
      } else {
        return false;
      }
    }
    float red(int layer) {
      // Takes a layer as an argument, returns the red value
      // of the representation for that layer.
      return colour[layer][0];
    }
    float green(int layer) {
      // Takes a layer as an argument, returns the green 
      // value of the representation for that layer.
      return colour[layer][1];
    }
    float blue(int layer) {
      // Takes a layer as an argument, returns the blue value
      // of the representation for that layer.
      return colour[layer][2];
    }
    void push(int x, int y, int layer, int value) {
      // Overwrite a value at coordinate (x, y) in layer with value.
      grid[x][y][layer] = value;
    }
};
environment enviro;

void drawAll () {
  // We use a box covering the entire area that is drawn each time
  // to fade the previous display out. This gives a cool motion 
  // blur idk I'm a fish biologist.
  glClearColor(0, 0, 0, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0, 0.0, 0.0, 0.2);
  glOrtho(-1.0,1.0,-1.0,1.0,-1.0,1.0);
  glBegin(GL_QUADS);
    glVertex2f(1.0,1.0);
    glVertex2f(1.0,-1.0);
    glVertex2f(-1.0,-1.0);
    glVertex2f(-1.0,1.0);
  glEnd();

  // Draw our environment. I'm using points, but you could use GL_LINES
  // or similar
  glPointSize(10.0);
  glBegin(GL_POINTS);
    for(int i = 0; i < NLAYERS; i++) {
      // Choose the colour associated with our layer from
      // a colour ramp specified in the environment.
      glColor3f(enviro.red(i), enviro.green(i), enviro.blue(i));
      for(int j = 0; j < 1 + 2/GRIDRES; j++) { // X
      for(int k = 0; k < 1 + 2/GRIDRES; k++) { // Y
        if(enviro.inspect(j,k,i)) {
          glVertex2f(enviro.worldCoord(j), enviro.worldCoord(k));
        }
      }
      }
    }
  glEnd();


  // Draw our agents. We use points here but feel free to get creative
  glPointSize(3.0);
  glColor3f(1.0,1.0,1.0);
  glBegin(GL_POINTS);
    for(int i = 0; i < NAGENTS; i++) {
      glVertex2f(ABMagents[i].x, ABMagents[i].y);
    }
  glEnd();
  glFlush();
}

void keyboard (unsigned char key, int xmouse, int ymouse) {
  // Want to add user control? Put it here. This is especially important if you
  // want a way to end the program if you run it in fullscreen mode.
}

void updateTime (int value) {
  // This function is your director. It is regularly called by glutTimerFunc
  // and ends with calls to update the display and creates a new timer
  // function.
  //
  // Keep glutPostRedisplay() and the glutTimerFunc at the end of the function.
  // This method is not frame-independent (ie, the frame updates after the code
  // here), so try to keep it concise.
  //
  for (int i = 0; i < NAGENTS; i++) {
    ABMagents[i].search();
  }
  glutPostRedisplay();
  glutTimerFunc(7, updateTime, 0);
}

bool readDataFile (string fname, int layer) {
  int dimension = 1 + 2/GRIDRES;
  int row[dimension];
  char delim = ',';
  fstream fin;
  fin.open(fname,ios::in);
  string temp, line, word;
  if(layer > NLAYERS) {
    cout << "Too many input files. Input at most: " << NLAYERS;
    cout << "input files.";
    return false;
  }

  while(fin >> temp) {
    for (int i = 0; i < dimension; i++) {
      getline(fin, line);
      stringstream s(line);
      for (int j = 0; j < dimension; j++) {
        getline(s, word, delim);
  if(word != "") enviro.push(j, i, layer, stoi(word,nullptr,10));
      }
    }
  }
  fin.close();
  return true; 
}

int main (int argc, char **argv) {

  // Setup: set the random number generator's seed, initialize our display
  // window.
  srand (time(NULL));

  // Read in data files in order
  for (int i = 1; i < argc; ++i) {
    readDataFile(argv[i], i - 1);
  }

  // Pretend to pass argc and argv to glut but really don't.
  int fargc = 0;
  char *fargv[1] = {(char*)" "};
  glutInit(&fargc, fargv); 

  // Initialize our window.
  glutInitDisplayMode(GLUT_SINGLE);
  glutInitWindowSize(800, 800);
  glutInitWindowPosition(100, 100);
  int win = glutCreateWindow("C++ Agent Based Model");

  // Register OpenGL functional callbacks for handling keyboard inputs,
  // updating the display, and our controller function.
  glutKeyboardFunc(keyboard);
  glutDisplayFunc(drawAll);
  glutTimerFunc(25, updateTime, 0);

  // Fire up OpenGL.
  glutMainLoop();

  // Anything here won't be run until the window is closed. Want to cout some
  // stats or other information? Write a file? Do something else? put it here.
  return 0;
}
