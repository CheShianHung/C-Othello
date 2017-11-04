/*
*** FILE NAME   : othello_ex.c
*** DATE        : Nov. 3rd 2017
*** WRITTEN By  : Che Shian Hung, JKL
*** PURPOSE		: The program use openGL to allow the user play Othello 
				  against AI implemented with Alpha-Beta Pruning searching algorithm
*** DESCRIPTION : In this program, the user has to play as white and the ai will play as black.
				  In each game, the user has to move first. The bright blue bot in each block is
				  used to indicated the possible moves for the user. The total white number and
				  the total black number are recorded in the right hand side. The user can always
				  start another game by simply clicking the restart button at the bottom right 
				  cornor. The evaluation function for the alpha-beta search algorithm is calculated
				  by the move positions in different perspective and the difference of the number
				  of black and white circles. For instance, in the getMoveValue function, the values
				  for the outter four corners are set to 70, meaning that if we predict the player
				  makes a move at one of the cornor, the value for the node will be added with -70, 
				  but if it is the ai does the cornor, the value will be added with 70. This
				  behavior can be observed in line 1010 and line 1015 in the expendNode function.
				  Based on the observation, with this evaluation function and searching implementation,
				  the ai will be the most difficult when the depth limit is 2. Any value more than 2
				  seems to overkill the selection. The evaluated values for many future option can
				  easily confuse the algorithm, causing the bad move for the current state to be 
				  selected.
*/

#include <stdio.h>						// standard C libraries
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <GL/glut.h>					// GLUT library


//@@***********************************************************************************@@
// Constants
#define WINDOW_NAME "Othello"			// Window name
#define WINDOW_XS 712					// Window size
#define WINDOW_YS 512
#define BOARD_SIZE 8
#define RAD_DEG 40
#define MIN -5000
#define MAX 5000
#define WHITE 1
#define BLACK 2
#define ALPHABETAHEIGHT 2				// Depth for the alpha beta program


//@@***********************************************************************************@@
// Structs

// general list
struct List;
typedef struct List {
	int val;
	struct List* next;
} List;

// Each block on the board piece, a board contains 64 blocks (if with size 8x8)
typedef struct block
{
	int state;							// 0 = none, 1 = white, 2 = black, 3 = possible move
	List* directionList;				// list of avaliable directions on the block
} Block;

// Node for each state
struct node;
typedef struct node {
	int id;								// node id
	int height;							// current depth for the node
	int originalMove;					// the original move from depth 0
	int value;							// the increment step value (part of evaluation)
	int whiteNum;						// white number of the current node
	int blackNum;						// black number of the current node
	int identity;						// 0: ai(max), 1: player(min)
	int childrenSize;					// number of children (possible move) from the node
	Block board[BOARD_SIZE][BOARD_SIZE];// the board of the node
	List* actionList;					// a list that contains of the movable position for the children (next step)
	struct node *parent;				// pointer points to the parent
	struct node *children;				// pointer points to a array of children
} Node;

//@@***********************************************************************************@@
// Global variables

Block board[BOARD_SIZE][BOARD_SIZE];	// the main board that the user is playing on

int playersColor;						// players perspective (1: white, 2: black)
int aisColor;							// ai's perspective (1: white, 2: black)
int whiteNum;							// white number on the current board
int blackNum;							// black number on the current board
int bestMove;							// a variable that uses to capture the best move during search
int nodeID;								// a variable that uses to count and assign the node id for each node created during search
int gameState;							// 1: players turn, 2: ais turn, 3: gameover

int restartButtonX = 538;				// x value of the restart button
int restartButtonY = 50;				// y value of the restart button
int buttonHeight = 30;					// height of the button
int buttonWidth = 150;					// width of the button

List* tempActionList;					// list that uses to capture the action list during the search

//@@***********************************************************************************@@
// Function

// openGL functions
void output(int x, int y, int mode, char *string);
void mouse_func(int button, int state, int x, int y);
void init_setup(int width, int height, char *windowName);
void display_func(void);
void keyboard_func(unsigned char c, int x, int y);
void reshape_handler(int width, int height);

void reset();												// reset the game
void swapColors();											// swap the players and ai's perspective
void setColors(int mode);									// mode 0: player is white, mode 1: player is black
void stateReset(Block b[BOARD_SIZE][BOARD_SIZE]);			// reset all state 3 to state 0 and delete all direction list
int aiMove();												// trigger alpha beta search
int getPosition(int x, int y);								// will return 0 - 63
int getMoveValue(int r, int c);								// part of evaluation, will be added to the node when the node is created
int boardScan(Block b[BOARD_SIZE][BOARD_SIZE], int mode);	// mode 0: normal mode, mode 1: search mode 
int flip(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c);	// place the circle in a specific block and flip the circles
int checkDirection(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c, int direction); // check if a move on a specific position will be vaild to flip in a specific direction

void displayList(List* h, int mode);						// display the list information in the console (mode 1: displyDirection, mode 2: displayAction)
List* deleteList(List* h);									// delete the list 
List* addToListFront(List*h, int d);						// add a value to the front of the list
List* listClone(List* sourceList, List* targetList);		// clone a list to another list

int maxValue(Node* n, int alpha, int beta, float h);		// half of the alpha-beta search function
int minValue(Node* n, int alpha, int bate, float h);		// another half of the alpha-beta search function

void expendNode(Node* n);									// expend a node and initialize the childrens' value
void destroyTree(Node* n);									// destroy the root node and all of its children
void displayNode(Node* n);									// display a node's information in the terminal

void displayBoard(Block b[BOARD_SIZE][BOARD_SIZE]);			// visualize the board in the terminal

//@@***********************************************************************************@@
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	init_setup(WINDOW_XS, WINDOW_YS, WINDOW_NAME);

	reset();
	boardScan(board, 0);

	glutDisplayFunc(display_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);

	glutMainLoop();

	return 1;
}

//@@***********************************************************************************@@
void reshape_handler(int width, int height)
{
	glViewport(0, 0, width, height);							// sets the viewport
	glMatrixMode(GL_PROJECTION);								// projection matrix
	glLoadIdentity();											// loads identity matrix
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);	// 2D orthographic projection
}

//@@***********************************************************************************@@
void init_setup(int width, int height, char *windowName)
{
	//glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);  // single buffer, rgb color
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// double buffer, rgb color
	glutInitWindowSize(width, height);				// init. window size
	glutInitWindowPosition(5, 5);					// init. window position
	glutCreateWindow(windowName);					// window name
	glutReshapeFunc(reshape_handler);				// sets the reshape call back
}

//@@***********************************************************************************@@
void display_func(void)
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);			// background color
	glClear(GL_COLOR_BUFFER_BIT);					// clearing the buffer not to keep the color

	// draw the grids for the puzzle board
	glColor3f(0.5, 0.5, 0.5);		
	glBegin(GL_LINES);
	for (int i = 0; i <= WINDOW_YS; i += WINDOW_YS / BOARD_SIZE)
	{
		glVertex2i(i, 0);		
		glVertex2i(i, WINDOW_YS);
		glVertex2i(0, i);
		glVertex2i(WINDOW_YS, i);
	}
	glEnd();

	// draw circles
	int i;
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (board[j][i].state != 0) {
				int radius = 25;
				if (board[j][i].state == 1) {		// white circle 
					glColor3f(1, 1, 1);
				}
				else if (board[j][i].state == 2) {	// black circle
					glColor3f(0, 0, 0);
				}
				else if (board[j][i].state == 3) {	// possible move circle
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

	// output string at the right
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
	if (gameState == 3) {							// when game is over
		if (whiteNum > blackNum) {
			output(552, 200, 3, "Congratulations!");
			output(552, 175, 3, "You win!");
		}
		else if(blackNum > whiteNum)
			output(552, 200, 3, "Oops! You lose!");
		else
			output(552, 200, 3, "Tie!");
	}
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
	glutSwapBuffers();								// double buffering

}	

//@@***********************************************************************************@@
void keyboard_func(unsigned char c, int x, int y)
{
	switch (c)
	{
	// quit the program
	case 'Q':
	case 'q':
		// delete pointers
		tempActionList = NULL;
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				board[i][j].directionList = deleteList(board[i][j].directionList);
			}
		}

		printf("Good Bye !\n");
		exit(0);		
	}
}

//@@***********************************************************************************@@
void mouse_func(int button, int state, int x, int y)
{
	// when click on the board
	if (state == GLUT_DOWN && x < WINDOW_YS) {
		// get block position
		int position = getPosition(x, y);
		int r = position / BOARD_SIZE;
		int c = position % BOARD_SIZE;

		// if the block is a valid and game is not over
		if (board[r][c].state == 3 && gameState != 3) {
			setColors(0);											// switch to player's perspective
			// make player's move and flip
			int flipNum = flip(board, r, c);
			whiteNum += flipNum + 1;
			blackNum -= flipNum;

			glutPostRedisplay();
			//sleep(1);
			
			//displayBoard(board); // for testing
			int m = aiMove();										// ai's turn
			if (m == -1) {											// if no move for ai
				int move = boardScan(board, 0);						// scan the board for ai
				if (move == 0) gameState = 3;						// if the game is over
				else if (move == 2) {								// no valid move for ai but the game is not finished
					setColors(0);									// change to player's perspectivce
					boardScan(board, 0);							// scan the board for player
				}
			}
			else {													// if there is at least a possible move for ai
				setColors(1);										// change to ai's perspective
				boardScan(board, 2);								// scan the board for ai

				// make ai's move and flip
				flipNum = flip(board, m / BOARD_SIZE, m % BOARD_SIZE);
				whiteNum -= flipNum;
				blackNum += flipNum + 1;

				swapColors();										// switch to player's perspective
				int move = boardScan(board, 0);						// scan the board for ai
				if (move == 0) gameState = 3;						// if the game is over
				else if (move == 2) {								// no valid move for ai but the game is not finished
					int m = aiMove();								// ai's turn
					setColors(1);									// change to ai's perspective 
					boardScan(board, 2);							// scan the board for ai

					// make ai's move and flip
					flipNum = flip(board, m / BOARD_SIZE, m % BOARD_SIZE);
					whiteNum -= flipNum;
					blackNum += flipNum + 1;

					swapColors();									// switch to player's perspective	
					int m2 = boardScan(board, 0);					// scan the board for the player
					if (m2 == 0) {									// if the game is over 
						gameState = 3;
					}
				}
			}
		}

		glutPostRedisplay();
	}
	// when restart button is clicked, restart the game
	else if (state == GLUT_DOWN && 
		x < restartButtonX + buttonWidth && 
		x > restartButtonX && y < WINDOW_YS - restartButtonY && 
		y > WINDOW_YS - restartButtonY - buttonHeight) {
		reset();
		boardScan(board, 0);
		glutPostRedisplay();
	}
}

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

//@@***********************************************************************************@@
// reset the game
void reset() {
	gameState = 1;
	whiteNum = 2;
	blackNum = 2;
	setColors(0);
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (i == j && (i == 3 || i == 4)) board[i][j].state = 1;
			else if ((i == 3 && j == 4) || (i == 4 && j == 3)) board[i][j].state = 2;
			else board[i][j].state = 0;
			board[i][j].directionList = deleteList(board[i][j].directionList);
		}
	}
}

//@@***********************************************************************************@@
// get the position from the x-y coordinates
int getPosition(int x, int y) {
	return y / (WINDOW_YS / BOARD_SIZE) * BOARD_SIZE + x / (WINDOW_YS / BOARD_SIZE);
}

//@@***********************************************************************************@@
// swap the player's and ai's perspective
void swapColors() {
	int temp = playersColor;
	playersColor = aisColor;
	aisColor = temp;
}

//@@***********************************************************************************@@
// mannualy set the perspective (mode 0: player is white, mode 1: player is black)
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

//@@***********************************************************************************@@
// set the state 3 to state 0 from the board and delete all direction lists
void stateReset(Block b[BOARD_SIZE][BOARD_SIZE]) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (b[i][j].state == 3) b[i][j].state = 0;
			if(b[i][j].directionList) b[i][j].directionList = deleteList(b[i][j].directionList);
		}
	}
}

//@@***********************************************************************************@@
// get the evaluated value from a specific block when a move is made
int getMoveValue(int r, int c) {
	// in the center 4 x 4 area
	if (r >= 2 && r <= 5 && c >= 2 && c <= 5) return 0;
	// inner four sides except corners
	else if (((r == 1 || r == 6) && c >= 2 && c <= 5) || ((c == 1 || c == 6) && r >= 2 && r <= 5)) return -30;
	// inner corners
	else if ((r == c && (r == 1 || r == 6)) || (r == 1 && c == 6) || (r == 6 && c == 1)) return -50;
	// outer corers
	else if ((r == c && (r == 0 || r == 7)) || (r == 0 && c == 7) || (r == 7 && c == 0)) return 70;
	// blocks next to four cornors
	else if (r == 1 || r == 6 || c == 1 || c == 6) return -30;
	// outer four sides 
	else return 50;
}

//@@***********************************************************************************@@
// make a move in a specific block and flip the circles
int flip(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c) {
	int flipNum = 0;												// number of circles fliped
	
	int originalR = r;
	int originalC = c;
	List* temp = b[r][c].directionList;								// read the direction list from the block

	while (temp) {
		switch (temp->val) {
		// up
		case 0:
			r -= 2;													// start checking from the second circle since the first one is correct for sure
			while (r >= 0) {
				if (b[r][c].state == playersColor) {				// if we found the player's circle, flip the circles in between
					b[originalR][c].state = playersColor;
					r++;
					while (r < originalR) {	
						b[r][c].state = playersColor;
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
		default:															// exception
			printf("Unrecongnized value passed into flipDirection function.\n");
			break;
		}
		// reset the variables and look for next direction
		r = originalR;
		c = originalC;
		temp = temp->next;
	}
	b[r][c].directionList = deleteList(b[r][c].directionList);				// delete the directionList

	return flipNum;
}

//@@***********************************************************************************@@
// Mode 0 (player mode) => return 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
// Mode 1 (ai mode) => return # of children
// Mode 2 (flip ai mode) => not show availabe moves
int boardScan(Block b[BOARD_SIZE][BOARD_SIZE], int mode) {
	int isFull = 1;															// assume the board is full
	int childNum = 0;														// children counter
	int nextMoveAvaliable = 0;

	stateReset(b);															// reset board state

	if (mode == 1) tempActionList = NULL;									// initialize tempActionList

	// scane thorugh each block
	for (int r = 0; r < BOARD_SIZE; r++) {
		for (int c = 0; c < BOARD_SIZE; c++) {
			if (b[r][c].state == 0) {										// if the block is empty
				int movable = 0;											// if the block is a possible move
				if (isFull == 1) isFull = 0;								// the board is not full anymore

				// check all neighbors, if any surrounding neighbor is aisColor, check the direction from the neightbor
				// up
				if (r - 1 >= 0) {
					if (b[r - 1][c].state == aisColor) {
						int check = checkDirection(b, r, c, 0);
						if (check == 1) {									// if the block is a valid move in the direction
							b[r][c].directionList = addToListFront(b[r][c].directionList, 0); // add the direction to the direction list
							if (movable == 0) movable = 1;					// the block is movable 
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
					if(mode != 2) b[r][c].state = 3;
					if (mode == 1) {
						tempActionList = addToListFront(tempActionList, r * 8 + c);
					}
				}
				if (movable == 1) nextMoveAvaliable = 1;					// if the block is valid for the next move, we do not need to skip a step
				else if (b[r][c].directionList) {							// if there is no move and there are values in the direction list, delete the list
					b[r][c].directionList = deleteList(b[r][c].directionList); 
				}
			}
		}
	}
	if (isFull == 1) nextMoveAvaliable = 0;									// the board is completely full
	else if (nextMoveAvaliable != 1) nextMoveAvaliable = 2;					// if no move available but the board is not full yet
	if(mode != 1) return nextMoveAvaliable;									// mode 0 & mode 2: return the avaliability for the next move
	else return childNum;													// mode 1: return # of possible moves 
}

//@@***********************************************************************************@@
// check if a block is a valid move from a specific direction
int checkDirection(Block b[BOARD_SIZE][BOARD_SIZE], int r, int c, int direction) {
	switch (direction) {
	// up
	case 0:
		r -= 2;																// check the second to save performance
		while (r >= 0 && b[r][c].state != 0 && b[r][c].state != 3) {		// if next block is not empty or a hint block and in bound
			if (b[r][c].state == playersColor) {							// if the block has players circle
				return 1;													// the block is valid
			}
			else
				r--;
		}
		break;
	// right up
	case 1:
		r -= 2;
		c += 2;
		while (r >= 0 && c < BOARD_SIZE && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (c < BOARD_SIZE && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (r < BOARD_SIZE && c < BOARD_SIZE && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (r < BOARD_SIZE && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (r < BOARD_SIZE && c >= 0 && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (c >= 0 && b[r][c].state != 0 && b[r][c].state != 3) {
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
		while (c >= 0 && r >= 0 && b[r][c].state != 0 && b[r][c].state != 3) {
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

//@@***********************************************************************************@@
// ai's turn, trigger the alpha beta search if doable
int aiMove() {
	// initialize variables
	nodeID = 0;
	bestMove = -1;

	// create root
	Node* r = (Node*)malloc(sizeof(Node));
	r->id = nodeID;
	r->height = 0;
	r->originalMove = -1;
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
	r->childrenSize = boardScan(r->board, 1);							// 0: board is full, 1: next move available, 2: no avaliable move but the board is not full
	r->actionList = tempActionList;
	tempActionList = NULL;

	// if there is children
	if (r->childrenSize > 0) {
		int value = maxValue(r, MIN, MAX, 0.0f);
	}

	// destroy the tree
	destroyTree(r);

	// return the position for the next move
	return bestMove;
}

//@@***********************************************************************************@@
// return the action with the highest evaluation value
int maxValue(Node* n, int alpha, int beta, float h) {
	if (n->childrenSize == 0 || h >= ALPHABETAHEIGHT)					// when the node cannot be expend or reach the depth limit
		return n->value + (n->blackNum - n->whiteNum);
	int v = MIN;
	expendNode(n);
	for (int i = 0; i < n->childrenSize; i++) {
		int min = minValue(&n->children[i], alpha, beta, h + 1);
		if (min > v) v = min;
		if (v >= beta) return v;										// pruning
		if (alpha < v) {												// update alpha
			alpha = v;
			bestMove = n->children[i].originalMove;						// record the move
		}
	}
	return v;
}

//@@***********************************************************************************@@
// return the action with the lowest evaluation value
int minValue(Node* n, int alpha, int beta, float h) {
	if (n->childrenSize == 0 || h >= ALPHABETAHEIGHT)					// when the node cannot be expend or reach the depth limit
		return n->value + (n->blackNum - n->whiteNum);
	int v = MAX;
	expendNode(n);
	for (int i = 0; i < n->childrenSize; i++) {
		int max = maxValue(&n->children[i], alpha, beta, h + 1);
		if (max < v) v = max;
		if (v <= alpha) return v;										// pruning
		if (beta > v) beta = v;											// update beta
	}
	return v;
}

//@@***********************************************************************************@@
// expend the node and initialize its childrens' values
void expendNode(Node* n) {
	if (n->childrenSize > 0) {											// if there are children in this node
		n->children = (Node*)calloc(n->childrenSize, sizeof(Node));
		List* l = n->actionList;
		int i = 0;
		while (l) {														// iterating through each children
			// initializing
			nodeID++;
			n->children[i].id = nodeID;
			n->children[i].height = n->height + 1;
			if (n->height == 0) {
				n->children[i].originalMove = l->val;
			}
			else {
				n->children[i].originalMove = n->originalMove;
			}
			n->children[i].value = n->value;
			n->children[i].whiteNum = n->whiteNum;
			n->children[i].blackNum = n->blackNum;
			n->children[i].childrenSize = 0;
			if (n->identity == 0) n->children[i].identity = 1; // player (min)
			else n->children[i].identity = 0; // ai (max)
			for (int j = 0; j < BOARD_SIZE; j++) {
				for (int k = 0; k < BOARD_SIZE; k++) {
					n->children[i].board[j][k].state = n->board[j][k].state;
					n->children[i].board[j][k].directionList = listClone(n->board[j][k].directionList, n->children[i].board[j][k].directionList);
				}
			}
			n->children[i].actionList = NULL;
			n->children[i].parent = n;
			n->children[i].children = NULL;

			// get the child's board ready and the relevant values
			if (n->children[i].identity == 1)
				setColors(1);
			else
				setColors(0);
			int flipNum = flip(n->children[i].board, l->val / BOARD_SIZE, l->val % BOARD_SIZE);
			swapColors();
			stateReset(n->children[i].board);
			n->children[i].childrenSize = boardScan(n->children[i].board, 1);
			n->children[i].actionList = tempActionList;
			tempActionList = NULL;
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

			l = l->next;
			i++;
		}
	}
}

//@@***********************************************************************************@@
// display the node information in the terminal
void displayNode(Node* n) {
	if (n) {
		printf("========================\nDisplay Node:\n");
		printf("id: %d\nheight: %d\noriginal move: %d\nvalue: %d\nwhiteNum: %d\nblackNum: %d\n", n->id, n->height, n->originalMove, n->value, n->whiteNum, n->blackNum);
		if (n->identity == 0) printf("identity: ai, max\n");
		else printf("identity: player, min\n");
		if(n->parent)
			printf("parent id: %d\n", n->parent->id);
		printf("children size: %d\n", n->childrenSize);
		displayList(n->actionList, 2);
		printf("========================\n\n");
	}
}

//@@***********************************************************************************@@
// display the board in terminal
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

//@@***********************************************************************************@@
// destroy the node and all of its children
void destroyTree(Node* n) {
	if (n != NULL) {
		n->parent = NULL;
		n->actionList = deleteList(n->actionList);
		for (int i = 0; i < n->childrenSize; i++) {
			if(n->children) destroyTree(&n->children[i]);
		}
		n = NULL;
		free(n);
	}
}

//@@***********************************************************************************@@
// delete the list and set the list pointer to null
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

//@@***********************************************************************************@@
// display the list in terminal (mode 1: displyDirection, mode 2: displayAction)
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

//@@***********************************************************************************@@
// add a new list at front of a list
List* addToListFront(List* h, int d) {
	List* newList = (List*)malloc(sizeof(List));
	newList->val = d;
	newList->next = NULL;
	if (!h) h = newList;
	else {
		newList->next = h;
		h = newList;
	}
	return h;
}

//@@***********************************************************************************@@
// clone the list to another list
List* listClone(List* sourceList, List* targetList) {
	if (targetList) targetList = deleteList(targetList);
	List* temp = sourceList;
	while (temp) {
		targetList = addToListFront(targetList, temp->val);
		temp = temp->next;
	}
	return targetList;
}