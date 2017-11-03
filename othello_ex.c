/*
*** FILE NAME   : othello_ex.c
*** DATE        : Sep. 26th 2017
*** WRITTEN By  : Che Shian Hung, JKL
*** PURPOSE		: The program use A* search and two different heuristic functions to
solve random 8 slide puzzle.
*** DESCRIPTION : The program uses four structures to carry out entrie process.
MyPoint is used to declare the coordinates for each tile. The program
will construct and keep track of the coordinates for each tile, so that
it can use these coordinates to move the tile. The Node struct contains
all the information for a state, like the path cost and the heuristic cost,
current values' position, what is the last action been made, the pointers
to its children and parent. The program use Nodes to create the tree and
search for the goal state. The list struct contains all the nodes that have
been created and sorts the nodes with the total cost (g + h). The solution
list struct contains a list of actions and moved values information, like
tile 2 moves down or tile 9 goes left. Inside aStar() function, it
will first create a tree from the initial state and create a list to sort
the cost. Then it will extract the node with the least code from the list
and check if it is the goal state. If not, it will expend the leaf into a
subtree and add the new leafs to the list. Once a goal state is found,
the it will trace the goal state back to the root and add the actions to
the solution list along the way. With the info in solution list, the
program can use the info to animate the action one by one.
*/

#include <stdio.h>               // standard C libraries
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <GL/glut.h>             // GLUT library


//@@***********************************************************************************@@
// Constants
#define WINDOW_XS 712					// Window size
#define WINDOW_YS 512
#define WINDOW_NAME "Othello"			// Window name
#define BOARD_SIZE 8
#define RAD_DEG 40
#define ANI_MSEC 5						// gap between frames
#define MIN -5000
#define MAX 5000
#define ALPHABETAHEIGHT 1
#define WHITE 1
#define BLACK 2

//@@***********************************************************************************@@
// Structures
typedef struct pt
{
	GLfloat x, y;
}MyPoint;

// List to sort the nodes
struct List;
typedef struct List {
	int val;
	struct List* next;
} List;

// Each block on the board piece
typedef struct block
{
	int state; // 0 = none, 1 = white, 2 = black;
	List* directionList; // list of avaliable directions
} Block;

// Node for each state
struct node;
typedef struct node {
	int alpha;
	int beta;
	int value;
	int whiteNum;
	int blackNum;
	int identity; // 0: ai(max), 1: player(min)
	int childrenSize;
	Block board[BOARD_SIZE][BOARD_SIZE];
	List* actionList;
	struct node *parent;
	struct node *children;
} Node;

// List to store the solution
struct solutionList;
typedef struct solutionList {
	int action;
	int movePosition;
	struct solutionList *next;
} SolutionList;

//@@***********************************************************************************@@
// Global Variables
/*
MyPoint bottomLeftPt;
MyPoint boxCordinates[8];
SolutionList *h1Solution = NULL;
SolutionList *h2Solution = NULL;
SolutionList *currentSolution = NULL;
*/

Block board[BOARD_SIZE][BOARD_SIZE];

int gameNumber; // if %2 == 0, player goes first. Else, computer goes first.
int playersColor; // 1: white, 2: black, 3: movable
int aisColor; // 1: white, 2: black, 3: movable
int whiteNum;
int blackNum;
int bestMove;
int value;

int restartButtonX = 538;
int restartButtonY = 50;
int buttonHeight = 30;
int buttonWidth = 150;

List* tempActionList;


/*
int values[9];
int initValues[9];

int gap = 1;						// step for animation
int h1Mode = 0;						// -2: manual finished, -1: manual, 0: before run, 1: running, 2: ready, 3: runningAnimation
int h2Mode = 0;						// -2: manual finished, -1: manual, 0: before run, 1: running, 2: ready, 3: runningAnimation
int hMethod = 0;					// 1: h1(), 2: h2()
int offSet = 10;
int recLength = 80;					// size of puzzle
int doAnimation = 0;				// 0: no animation, 1: up, 2: down, 3: left, 4: right
int selectedValue = -1;
int currentPosition;
int animationCounter;
int findingSolution = 0;
int runningAnimation = 0;
int solutionAnimationCounter = 0;
*/

//@@***********************************************************************************@@
// Function prototypes
void output(int x, int y, int mode, char *string);
void mouse_func(int button, int state, int x, int y);
void init_setup(int width, int height, char *windowName);
void display_func(void);
void keyboard_func(unsigned char c, int x, int y);
void animation_func(int val);
void reshape_handler(int width, int height);

void reset();
//void showBlockStatus(int position);
int getPosition(int x, int y); //will return 0 - 63
// Mode 0 (normal mode) => return 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
// Mode 1 (alpha beta mode) => return # of children
int boardScan(Block b[BOARD_SIZE][BOARD_SIZE], int mode); 
int checkDirection(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c, int direction);
void swapColors();
void setColors(int mode); // mode 0: player is white, mode 1: player is black
int flip(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c);
void stateReset(Block b[BOARD_SIZE][BOARD_SIZE]); // reset all state 3 to state 0 and delete all direction list
int getMoveValue(int r, int c);
int getFlipValue(int r, int c);
//int boardIsFull();
//int hasAvaliableMove();
void test();
void aiMove();

// mode 1: displyDirection, mode 2: displayAction 
void displayList(List* h, int mode);
List* deleteList(List* h);
List* addToListFront(List*h, int d);
List* listClone(List* sourceList, List* targetList);

int alphaBetaSearch();
int maxValue(Node* n, int alpha, int beta, float h);
int minVlaue(Node* n, int alpha, int bate, float h);

void expendNode(Node* n);
void destroyTree(Node* n);
void displayNode(Node* n);
void displayBoard(Block b[BOARD_SIZE][BOARD_SIZE]);

/*
void aStar();
void drawTile(char *val, float x, float y);
void setupPuzzle();
void newProblem();
void reloadProblem();
void drawAllTiles();
void switchPosition(int *ary, int a, int b);

int isGoal(int *ary);
int solvable(int *ary);
int heuristic(int *ary);
int getDirection(int p);
int getPosition(int x, int y);

void destroyTree(Node *n);
void expendTree(Node *n);

void displayList(List *h);
List* addToList(List *h, Node *n);
List* removeFront(List *h);

void displaySolutionList(SolutionList *h);
SolutionList* addToSolutionList(SolutionList * h, Node *n);
*/

//@@***********************************************************************************@@
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	init_setup(WINDOW_XS, WINDOW_YS, WINDOW_NAME);

	gameNumber = 0;
	playersColor = WHITE;
	aisColor = BLACK;
	reset();
	boardScan(board, 0);
	//test();

	glutDisplayFunc(display_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutTimerFunc(ANI_MSEC, animation_func, 0);

	glutMainLoop();

	return 1;
}	// end of main()

	//@@***********************************************************************************@@
void reshape_handler(int width, int height)
{
	glViewport(0, 0, width, height);							// sets the viewport
	glMatrixMode(GL_PROJECTION);								// projection matrix
	glLoadIdentity();											// loads identity matrix
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);	// 2D orthographic projection
}	// end of reshape_handler()

	//@@***********************************************************************************@@
void init_setup(int width, int height, char *windowName)
{
	//glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);  // single buffer, rgb color
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  // double buffer, rgb color
	glutInitWindowSize(width, height);			  // init. window size
	glutInitWindowPosition(5, 5);				  // init. window position
	glutCreateWindow(windowName);				  // window name
	glutReshapeFunc(reshape_handler);		      // sets the reshape call back
}	// end of init_setup()

	//@@***********************************************************************************@@
void display_func(void)
{
	int i;

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);			// background color
	glClear(GL_COLOR_BUFFER_BIT);				// clearing the buffer not to keep the color

	// draw the grids for the puzzle board
	glColor3f(0.5, 0.5, 0.5);			// setting pen color
	glBegin(GL_LINES);
	for (i = 0; i <= WINDOW_YS; i += WINDOW_YS / BOARD_SIZE)
	{
		glVertex2i(i, 0);				// vertical grid lines
		glVertex2i(i, WINDOW_YS);

		glVertex2i(0, i);				// horizontal grid lines
		glVertex2i(WINDOW_YS, i);
	}
	glEnd();

	// draw circles
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (board[j][i].state != 0) {
				int radius = 25;
				if (board[j][i].state == 1) {
					glColor3f(1, 1, 1);
				}
				else if (board[j][i].state == 2) {
					glColor3f(0, 0, 0);
				}
				else if (board[j][i].state == 3) {
					radius = 5;
					glColor3f(0, 0.9, 0.9);
				}
				glBegin(GL_POLYGON); // Circle at center of field
				for (float ang = 0.0; ang < 360.0; ang += 10.0) {
					float x = radius * cos(ang / RAD_DEG) + 32 + i * 64;
					float y = radius * sin(ang / RAD_DEG) + WINDOW_YS - 32 - j * 64;
					glVertex2f(x, y);
				}
				glEnd();
			}
		}
	}

	// print our string
	char str[5];
	glColor3f(0.9, 0.9, 0.9);
	output(558, 470, 2, "Othello");
	output(552, 435, 3, "CS4200/5200");
	output(552, 415, 3, "Che Shian Hung");
	output(552, 340, 3, "white num: ");
	snprintf(str, 3, "%d", whiteNum);
	output(660, 340, 3, str);
	output(552, 320, 3, "black num: ");
	snprintf(str, 3, "%d", blackNum);
	output(660, 320, 3, str);
	glEnd();
	// restart botton
	output(582, 61, 3, "restart");
	glBegin(GL_LINES);
	glVertex2i(restartButtonX, restartButtonY);
	glVertex2i(restartButtonX + buttonWidth, restartButtonY);

	glVertex2i(restartButtonX, restartButtonY);
	glVertex2i(restartButtonX, restartButtonY + buttonHeight);

	glVertex2i(restartButtonX + buttonWidth, restartButtonY);
	glVertex2i(restartButtonX + buttonWidth, restartButtonY + buttonHeight);

	glVertex2i(restartButtonX, restartButtonY + buttonHeight);
	glVertex2i(restartButtonX + buttonWidth, restartButtonY + buttonHeight);
	glEnd();

	glFlush();
	glutSwapBuffers();	// double buffering

}	// end of display_func()

	//@@***********************************************************************************@@
void keyboard_func(unsigned char c, int x, int y)
{
	switch (c)
	{

	case 'Q':
	case 'q':

		// delete direction lists in each block
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				board[i][j].directionList = deleteList(board[i][j].directionList);
			}
		}

		printf("Good Bye !\n");
		exit(0);				 // terminates the program
	}  // end of switch
}	// end of keyboard_func()

	//@@***********************************************************************************@@
void mouse_func(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && x < WINDOW_YS) {
		int position = getPosition(x, y);
		int r = position / BOARD_SIZE;
		int c = position % BOARD_SIZE;

		//printf("value: %d\n", getFlipValue(r, c));

		if (board[r][c].state == 3) {
			int flipNum = flip(board, r, c);
			whiteNum += flipNum + 1;
			blackNum -= flipNum;
			stateReset(board);
			aiMove();
			//swapColors();
			//int move = boardScan(board, 0);
			// board is full
			//if (move == 0){
				// game over
			//}
			//no more next move
			//else if (move == 2) {
				// keep moving
			//}

		}

		glutPostRedisplay();
	}
	// restart
	else if (state == GLUT_DOWN && 
		x < restartButtonX + buttonWidth && 
		x > restartButtonX && y < WINDOW_YS - restartButtonY && 
		y > WINDOW_YS - restartButtonY - buttonHeight) {
		playersColor = WHITE;
		aisColor = BLACK;
		reset();
		boardScan(board, 0);
	}
}

//@@***********************************************************************************@@
void animation_func(int val)
{
	glutPostRedisplay();
	glutTimerFunc(ANI_MSEC, animation_func, 0);

}	//end animation_func

	//@@***********************************************************************************@@
void output(int x, int y, int mode, char *string)
{
	int len, i;
	glRasterPos2i(x, y);
	len = (int)strlen(string);
	if (mode == 1)
	{
		for (i = 0; i < len; i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
		}
	}
	else if (mode == 2)
	{
		for (i = 0; i < len; i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
		}
	}
	else if (mode == 3) {
		for (i = 0; i < len; i++)
		{
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
		}
	}
}

//@@ reset the puzzle****************************************@@
void reset() {
	whiteNum = 2;
	blackNum = 2;
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (i == j && (i == 3 || i == 4)) {
				board[i][j].state = 1;
			}
			else if ((i == 3 && j == 4) || (i == 4 && j == 3)) {
				board[i][j].state = 2;
			}
			else board[i][j].state = 0;
			board[i][j].directionList = deleteList(board[i][j].directionList);
		}
	}
}

//@@ get the block position from the axis
int getPosition(int x, int y) {
	return y / (WINDOW_YS / BOARD_SIZE) * BOARD_SIZE + x / (WINDOW_YS / BOARD_SIZE);
}

void swapColors() {
	int temp = playersColor;
	playersColor = aisColor;
	aisColor = temp;
}

// mode 0: player is white, mode 1: player is black
void setColors(int mode) {
	switch (mode) {
	case 0:
		playersColor = WHITE;
		aisColor = BLACK;
		break;
	case 1:
		playersColor = BLACK;
		aisColor = WHITE;
		break;
	}
}

void stateReset(Block b[BOARD_SIZE][BOARD_SIZE]) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (b[i][j].state == 3) {
				b[i][j].state = 0;
				b[i][j].directionList = deleteList(b[i][j].directionList);
			}
		}
	}
}

int getMoveValue(int r, int c) {
	// in the center 4 x 4 area
	if (r >= 2 && r <= 5 && c >= 2 && c <= 5) return 10;
	// inner four sides except corners
	else if (((r == 1 || r == 6) && c >= 2 && c <= 5) || ((c == 1 || c == 6) && r >= 2 && r <= 5)) return -10;
	// inner corners
	else if ((r == c && (r == 1 || r == 6)) || (r == 1 && c == 6) || (r == 6 && c == 1)) return -30;
	// outer corers
	else if ((r == c && (r == 0 || r == 7)) || (r == 0 && c == 7) || (r == 7 && c == 0)) return 50;
	// outer four sides 
	else return 25;
}

int getFlipValue(int r, int c) {
	// outter four sides except corners
	if (r == 0 || r == 7 || c == 0 || c == 7) return 2;
	else return 1;
}

int flip(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c) {
	int originalR = r;
	int originalC = c;
	int flipNum = 0;
	List* temp = b[r][c].directionList;
	//displayList(temp, 1);

	while (temp) {
		switch (temp->val) {
		// up
		case 0:
			r -= 2;
			while (r >= 0) {
				if (b[r][c].state == playersColor) {
					b[originalR][c].state = playersColor;
					r++;
					while (r < originalR) {
						b[r][c].state = playersColor;
						//printf("flip %d, %d\n", r, c);
						r++;
						flipNum++;
					}
					break;
				}
				else
					r--;
			}
			break;
		// right up
		case 1:
			r -= 2;
			c += 2;
			while (r >= 0 && c < BOARD_SIZE) {
				if (b[r][c].state == playersColor) {
					b[originalR][originalC].state = playersColor;
					r++;
					c--;
					while (r < originalR) {
						b[r][c].state = playersColor;
						r++;
						c--;
						flipNum++;
					}
					break;
				}
				else {
					r--;
					c++;
				}
			}
			break;
		// right
		case 2:
			c += 2;
			while (c < BOARD_SIZE) {
				if (b[r][c].state == playersColor) {
					b[r][originalC].state = playersColor;
					c--;
					while (c > originalC) {
						b[r][c].state = playersColor;
						c--;
						flipNum++;
					}
					break;
				}
				else
					c++;
			}
			break;
		// right down
		case 3:
			r += 2;
			c += 2;
			while (r < BOARD_SIZE && c < BOARD_SIZE) {
				if (b[r][c].state == playersColor) {
					b[originalR][originalC].state = playersColor;
					r--;
					c--;
					while (c > originalC) {
						b[r][c].state = playersColor;
						r--;
						c--;
						flipNum++;
					}
					break;
				}
				else {
					r++;
					c++;
				}
			}
			break;
		// down
		case 4:
			r += 2;
			while (r < BOARD_SIZE) {
				if (b[r][c].state == playersColor) {
					b[originalR][c].state = playersColor;
					r--;
					while (r > originalR) {
						b[r][c].state = playersColor;
						r--;
						flipNum++;
					}
					break;
				}
				else
					r++;
			}
			break;
			// left down
		case 5:
			r += 2;
			c -= 2;
			while (r < BOARD_SIZE && c >= 0) {
				if (b[r][c].state == playersColor) {
					b[originalR][originalC].state = playersColor;
					r--;
					c++;
					while (r > originalR) {
						b[r][c].state = playersColor;
						r--;
						c++;
						flipNum++;
					}
					break;
				}
				else {
					r++;
					c--;
				}
			}
			break;
			// left
		case 6:
			c -= 2;
			while (c >= 0) {
				if (b[r][c].state == playersColor) {
					b[r][originalC].state = playersColor;
					c++;
					while (c < originalC) {
						b[r][c].state = playersColor;
						c++;
						flipNum++;
					}
					break;
				}
				else
					c--;
			}
			break;
			// left up
		case 7:
			r -= 2;
			c -= 2;
			while (c >= 0 && r >= 0) {
				if (b[r][c].state == playersColor) {
					b[originalR][originalC].state = playersColor;
					r++;
					c++;
					while (r < originalR) {
						b[r][c].state = playersColor;
						r++;
						c++;
						flipNum++;
					}
					break;
				}
				else {
					r--;
					c--;
				}
			}
			break;
			// exception
		default:
			printf("Unrecongnized value passed into flipDirection function.\n");
			break;
		}
		r = originalR;
		c = originalC;
		temp = temp->next;
	}
	b[r][c].directionList = deleteList(b[r][c].directionList);
	//printf("player: %d\n(should be black)\n", playersColor);
	//printf("flip on (%d, %d)\n", r, c);
	//displayBoard(b);
	return flipNum;
}

// Mode 0 (player mode) => return 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
// Mode 1 (ai mode) => return # of children
int boardScan(Block b[BOARD_SIZE][BOARD_SIZE], int mode) {
	// assume the board is full
	int nextMoveAvaliable = 0;
	int childNum = 0;
	if (mode == 1) tempActionList = NULL;
	for (int r = 0; r < BOARD_SIZE; r++) {
		for (int c = 0; c < BOARD_SIZE; c++) {
			// if the block is empty
			if (b[r][c].state == 0) {
				int movable = 0;
				if (nextMoveAvaliable == 0) nextMoveAvaliable = 2;

				// check all neighbors
				// up
				if (r - 1 >= 0) {
					if (b[r - 1][c].state == aisColor) {
						int check = checkDirection(b, r, c, 0);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 0);
							if (movable == 0) movable = 1;
						}
					}
				}
				// up right
				if (r - 1 >= 0 && c + 1 < BOARD_SIZE) {
					if (b[r - 1][c + 1].state == aisColor) {
						int check = checkDirection(b, r, c, 1);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 1);
							if (movable == 0) movable = 1;
						}
					}
				}
				// right
				if (c + 1 < BOARD_SIZE) {
					if (b[r][c + 1].state == aisColor) {
						int check = checkDirection(b, r, c, 2);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 2);
							if (movable == 0) movable = 1;
						}
					}
				}
				// down right
				if (r + 1 < BOARD_SIZE && c + 1 < BOARD_SIZE) {
					if (b[r + 1][c + 1].state == aisColor) {
						int check = checkDirection(b, r, c, 3);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 3);
							if (movable == 0) movable = 1;
						}
					}
				}
				// down
				if (r + 1 < BOARD_SIZE) {
					if (b[r + 1][c].state == aisColor) {
						int check = checkDirection(b, r, c, 4);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 4);
							if (movable == 0) movable = 1;
						}
					}
				}
				// left down
				if (r + 1 < BOARD_SIZE && c - 1 >= 0) {
					if (b[r + 1][c - 1].state == aisColor) {
						int check = checkDirection(b, r, c, 5);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 5);
							if (movable == 0) movable = 1;
						}
					}
				}
				// left
				if (c - 1 >= 0) {
					if (b[r][c - 1].state == aisColor) {
						int check = checkDirection(b, r, c, 6);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 6);
							if (movable == 0) movable = 1;
						}
					}
				}
				// left up
				if (r - 1 >= 0 && c - 1 >= 0) {
					if (b[r - 1][c - 1].state == aisColor) {
						int check = checkDirection(b, r, c, 7);
						if (check == 1) {
							b[r][c].directionList = addToListFront(b[r][c].directionList, 7);
							if (movable == 0) movable = 1;
						}
					}
				}

				// change block state
				if (movable == 1) {	
					childNum++;
					b[r][c].state = 3;
					if (mode == 1) {
						tempActionList = addToListFront(tempActionList, r * 8 + c);
					}
					//else b[r][c].state = 3;

				}
				else if (movable == 1 && nextMoveAvaliable != 1) nextMoveAvaliable = 1;
				else if (b[r][c].directionList) {
					b[r][c].directionList = deleteList(b[r][c].directionList);
				}
			}
		}
	}
	if(mode == 0) return nextMoveAvaliable;
	else if (mode == 1) {
		return childNum;
	}
}

int checkDirection(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c, int direction) {
	switch (direction) {
	// up
	case 0:
		r -= 2;
		while (r >= 0 && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else
				r--;
		}
		break;
	// right up
	case 1:
		r -= 2;
		c += 2;
		while (r >= 0 && c < BOARD_SIZE && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else {
				r--;
				c++;
			}
		}
		break;
	// right
	case 2:
		c += 2;
		while (c < BOARD_SIZE && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else
				c++;
		}
		break;
	// right down
	case 3:
		r += 2;
		c += 2;
		while (r < BOARD_SIZE && c < BOARD_SIZE && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else {
				r++;
				c++;
			}
		}
		break;
	// down
	case 4:
		r += 2;
		while (r < BOARD_SIZE && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else
				r++;
		}
		break;
	// left down
	case 5:
		r += 2;
		c -= 2;
		while (r < BOARD_SIZE && c >= 0 && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else {
				r++;
				c--;
			}
		}
		break;
	// left
	case 6:
		c -= 2;
		while (c >= 0 && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else
				c--;
		}
		break;
	// left up
	case 7:
		r -= 2;
		c -= 2;
		while (c >= 0 && r >= 0 && b[r][c].state != 0) {
			if (b[r][c].state == playersColor) {
				return 1;
			}
			else {
				r--;
				c--;
			}
		}
		break;
	// exception
	default:
		printf("Unrecongnized value passed into flipDirection function.\n");
		break;
	}
	return 0;
}

/*
// Each block on the board piece
typedef struct block
{
int state; // 0 = none, 1 = white, 2 = black;
DirectionList *directionList; // list of avaliable directions
} Block;

// Node for each state
struct node;
typedef struct node {
int alpha;
int beta;
int value;
int whiteNum;
int blackNum;
int identity; // 0: ai(max), 1: player(min)
int childrenSize;
Block board[BOARD_SIZE][BOARD_SIZE];
struct node *parent;
struct node *children;
} Node;
*/

void aiMove() {
	// check current
	// create root
	Node* r = (Node*)malloc(sizeof(Node));
	r->alpha = MIN;
	r->beta = MAX;
	r->value = 0;
	r->whiteNum = whiteNum;
	r->blackNum = blackNum;
	r->identity = 0;
	r->childrenSize = 0;
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			r->board[i][j].state = board[i][j].state;
			r->board[i][j].directionList = NULL;
		}
	}
	r->actionList = NULL;
	r->parent = NULL;
	r->children = NULL;

	// check current
	setColors(1); // black
	r->childrenSize = boardScan(r->board, 1); // 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
	r->actionList = tempActionList;

	// if there is children
	if (r->childrenSize > 0) {
		bestMove = -1;
		int value = maxValue(r, MIN, MAX, 0);
	}

	destroyTree(r);
}

// ai's side
int maxValue(Node* n, int alpha, int beta, float h) {
	if (n->childrenSize == 0 || h > ALPHABETAHEIGHT) {
		return n->value;
	}
	int v = MIN;
	expendNode(n);

}

// player's side
int minValue(Node* n, int alpha, int beta, float h) {

}

void displayBoard(Block b[BOARD_SIZE][BOARD_SIZE]) {
	printf("========================================\nPrint board\n");
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			printf("%d ", b[i][j].state);
		}
		printf("\n");
	}
	printf("========================================\n\n");
}

/*
r->alpha = MIN;
r->beta = MAX;
r->value = 0;
r->whiteNum = whiteNum;
r->blackNum = blackNum;
r->identity = 0;
r->childrenSize = 0;
for (int i = 0; i < BOARD_SIZE; i++) {
for (int j = 0; j < BOARD_SIZE; j++) {
r->board[i][j].state = board[i][j].state;
r->board[i][j].directionList = NULL;
}
}
r->actionList = NULL;
r->parent = NULL;
r->children = NULL;

// check current
swapColors();
r->childrenSize = boardScan(r->board, 1); // 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
r->actionList = tempActionList;
*/

void expendNode(Node* n) {
	if (n->childrenSize > 0) {
		//printf("expending...\n");
		n->children = (Node*)calloc(n->childrenSize, sizeof(Node));
		List* l = n->actionList;
		int i = 0;
		while (l) {
			printf("\n\n\n\n");
			n->children[i].alpha = n->alpha;
			n->children[i].beta = n->beta;
			n->children[i].value = n->value;
			n->children[i].whiteNum = n->whiteNum;
			n->children[i].blackNum = n->blackNum;
			if (n->identity == 0) n->children[i].identity = 1; // player (min)
			else n->children[i].identity = 0; // ai (max)
			n->children[i].childrenSize = 0;
			for (int j = 0; j < BOARD_SIZE; j++) {
				for (int k = 0; k < BOARD_SIZE; k++) {
					n->children[i].board[j][k].state = n->board[j][k].state;
					n->children[i].board[j][k].directionList = listClone(n->board[j][k].directionList, n->children[i].board[j][k].directionList);
				}
			}
			//displayBoard(n->board);
			n->children[i].actionList = NULL;
			n->children[i].parent = n;
			n->children[i].children = NULL;
			// mode 0: player is white, mode 1: player is black
			if (n->children[i].identity == 1)
				setColors(1);
			else
				setColors(0);
			int flipNum = flip(n->children[i].board, l->val / BOARD_SIZE, l->val % BOARD_SIZE);
			swapColors();
			stateReset(n->children[i].board);
			n->children[i].childrenSize = boardScan(n->children[i].board, 1);
			n->children[i].actionList = tempActionList;
			if (n->children[i].identity == 1) {
				n->children[i].whiteNum = n->whiteNum - flipNum;
				n->children[i].blackNum = n->blackNum + flipNum + 1;
				n->children[i].value = n->value + getMoveValue(l->val / BOARD_SIZE, l->val % BOARD_SIZE);
			}
			else {
				n->children[i].whiteNum = n->whiteNum + flipNum + 1;
				n->children[i].blackNum = n->blackNum - flipNum;
				n->children[i].value = n->value - getMoveValue(l->val / BOARD_SIZE, l->val % BOARD_SIZE);
			}

			displayBoard(n->children[i].board);
			displayNode(&n->children[i]);

			l = l->next;
			i++;
		}
	}
}

void displayNode(Node* n) {
	if (n) {
		printf("========================\nDisplay Node:\n");
		printf("alpha: %d\nbeta: %d\nvalue: %d\nwhiteNum: %d\nblackNum: %d\n", n->alpha, n->beta, n->value, n->whiteNum, n->blackNum);
		if (n->identity == 0) printf("identity: ai, max\n");
		else printf("identity: player, min\n");
		printf("children size: %d\n", n->childrenSize);
		displayList(n->actionList, 2);
		printf("========================\n\n");
	}
}

void destroyTree(Node* n) {
	if (n != NULL) {
		n->parent = NULL;
		n->actionList = deleteList(n->actionList);
		for (int i = 0; i < n->childrenSize; i++) {
			if(n->children)
				destroyTree(&n->children[i]);
		}
		n = NULL;
		free(n);
	}
}


List* deleteList(List* h) {
	if (h != NULL) {
		while (h) {
			List* temp = h;
			h = temp->next;
			free(temp);
			temp = NULL;
		}
		h = NULL;
	}
	return h;
}

// mode 1: displyDirection, mode 2: displayAction 
void displayList(List* h, int mode) {
	switch (mode) {
	case 1:
		if (h) {
			printf("Display direction: ");
			List *l = h;
			while (l) {
				switch (l->val) {
				case 0: printf("up "); break;
				case 1: printf("rightUp "); break;
				case 2: printf("right "); break;
				case 3: printf("rightDown "); break;
				case 4: printf("down "); break;
				case 5: printf("leftDown "); break;
				case 6: printf("left "); break;
				case 7: printf("leftUp "); break;
				}
				l = l->next;
			}
			printf("\n");
		}
		else {
			printf("empty list\n");
		}
		break;
	case 2:
		if (h) {
			printf("Display action positions (0 - 63): ");
			List *l = h;
			while (l) {
				printf("%d ", l->val);
				l = l->next;
			}
			printf("\n");
		}
		else {
			printf("empty list\n");
		}
		break;
	}
}

List* addToListFront(List* h, int d) {
	List* newList = (List*)malloc(sizeof(List));
	newList->val = d;
	newList->next = NULL;
	// if the list is empty
	if (!h) {
		h = newList;
	}
	// if none empty list
	else {
		newList->next = h;
		h = newList;
	}
	return h;
}

List* listClone(List* sourceList, List* targetList) {
	if (targetList) targetList = deleteList(targetList);
	List* temp = sourceList;
	while (temp) {
		targetList = addToListFront(targetList, temp->val);
		temp = temp->next;
	}
	return targetList;
}

void test() {
}

