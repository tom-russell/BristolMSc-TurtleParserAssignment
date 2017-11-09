#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "extension.h"
#include <fcntl.h>

/* SDL definitions */
    #define COM_LEN 25
    #define COM_CEN 35
    #define COM_RAD 30
    #define SPO_RAD 4
    #define PUD_SIZ 60
    #define PUD_X 70
    #define PUD_Y 5
    #define ARR_X 100
    #define ARR_Y1 15
    #define ARR_Y2 55
    #define AR_LX 80
    #define AR_RX 120
    #define GREY  150, 150, 150
    #define RED   255,   0,   0
    #define BLACK   0,   0,   0

int main(int argc, char **argv)
{
	Program prog;
	SDL_Simplewin sw;
	Token start_token;
    
    /* Keep a pointer to the start of the linked-list, for freeing later */
	start_token.next = NULL;
	prog.curr = &start_token;
    /*Initialise SDL */
	Neill_SDL_Init(&sw);

	/* Exit if not passed an input text file */
	if (argc != 2) {
		fprintf(stderr, "1 command line input file is required.\n");
		exit(no_input_file);
	}

	if ((prog.f_out = fopen(argv[1], "w")) == NULL) {
		fprintf(stderr, "Error occurred when opening output file.\n");
		exit(fopen_fail);
	}

	/* Initialise the program struct, then run the parser */
	InitProgram(&prog, &sw);
	ProgramMAIN(&prog);

	/* Wait until the user closes the SDL window */
	printf("Parsing Complete. To exit close the SDL window.\n");
	while (!sw.finished) {
		Neill_SDL_Events(&sw);
	}
    
	/* Clear up memory & graphics subsystems before */
	if (fclose(prog.f_out) != 0) {
		fprintf(stderr, "Error occurred when closing input file.\n");
		exit(fclose_fail);
	}
	atexit(SDL_Quit);

	return 0;
}

void InitProgram(Program *prog, SDL_Simplewin *sw)
{
	int i = 0;

	prog->sw = sw;
	prog->pen_active = 1;

	/* Initialise turtle coordinates to the window center & starting angle */
	prog->xy[0] = WWIDTH / 2.0;
	prog->xy[1] = WHEIGHT / 2.0;
	prog->angle = STARTANGLE;

	/* Initialise all VAR variables and the vars_in_use in the array to 0 */
	for (i = 0; i < TOTALVARS; i++) {
		prog->vars[i] = 0.0;
		prog->var_in_use[i] = 0;
	}
}

void LoadToken(Program *prog) 
{
	UpdateUI(prog);

	/* If next instruction has not been inputted yet */
	if (prog->curr->next == NULL) {
		prog->curr->next = malloc(sizeof(Token));
		prog->curr = prog->curr->next;
		prog->curr->next = NULL;

		while (scanf("%s", CURRENTTOKEN) != 1) {
			printf("Invalid Token Scanned.\n");
		}
		/* If the token exceeds the max token length parsing has failed */
		if (strlen(prog->curr->str) > MAXTOKENLEN) {
			ParseError("Token exceed mac token length.\n", prog);
		}
		/* Print the new token to the output file */
		fprintf(prog->f_out, "%s ", prog->curr->str);
	}
	/* Else, the program is in a DO loop so next instruction can be run */
	else {
		prog->curr = prog->curr->next;
	}
}

void CheckToken(Program *prog, char token[], char error_text[])
{
	/* Check if the next token is the expected token, then token is invalid */
	LoadToken(prog);
	if (!StrMatch(CURRENTTOKEN, token)) {
		ParseError(error_text, prog);
	}
}

void FreeMemory(Token *start_token) 
{
    Token *temp;
    /* Follow the linked list of tokens from the start, freeing each element */
    while (start_token != NULL) {
        temp = start_token;
        start_token = start_token->next;
        free(temp);
    }
}

void ParseError(char *phrase, Program *prog)
{
	/* Print the error statement, free allocated memory then exit the program */
	fprintf(stderr, "%s\nInvalid token entered: \"%s\"\n", phrase, CURRENTTOKEN);
	fprintf(stderr, "Close the SDL window to end the program. \n");

	/* Wait until the user presses a button or closes the SDL window */
	while (!prog->sw->finished) {
		Neill_SDL_Events(prog->sw);
	}
	exit(parse_fail);
}

void PushStack(P_stack *stack, Program *prog, double value)
{
	/* Exit the program if the stack has 'overflowed' */
	if (stack->top > MAXSTACKSIZE) {
		ParseError("Polish expression too long, exceeds max stack size.", prog);
	}
	/* Add the given value to the the top of the stack */
	stack->s[stack->top] = value;
	stack->top = stack->top + 1;
}

double PopStack(P_stack *stack, Program *prog)
{
	stack->top--;
	/* Exit the program if the stack has 'underflowed' */
	if (stack->top < 0) {
		ParseError("Invalid Polish expression. Too many operators.", prog);
	}
	/* Return the value from the top of the stack */
	return stack->s[stack->top];
}

void TurtleMove(Program *prog, int xy_vals[2][2])
{
	Neill_SDL_SetDrawColour(prog->sw, 255, 255, 255);
	
	/* Draw the line in SDL for the current movement */
	SDL_RenderDrawLine(prog->sw->renderer, xy_vals[0][0], xy_vals[0][1],
	                                       xy_vals[1][0], xy_vals[1][1]);
	/* Render the updated graphics in the SDL window */
	SDL_RenderPresent(prog->sw->renderer);
	SDL_UpdateWindowSurface(prog->sw->win);
	Neill_SDL_Events(prog->sw);
    
	UpdateUI(prog);
}

int RoundVal(double value)
{
	double intpart, fractpart;
	int round_val = (int)value;

	fractpart = modf(value, &intpart);
    
    /* Takes input double value, rounds to the nearest integer value */
	if (fractpart < 0.5) {
		return round_val;
	}
	else {
		return round_val + 1;
	}
}

void AngleAdjust(Program *prog, double change)
{
	prog->angle = prog->angle + change;

	/* If the angle is outside the range 0 <= h < 2pi, add/sub 2pi until it 
	 * fits in the correct range */
	while (prog->angle < 0.0) {
		prog->angle = prog->angle + (2 * M_PI);
	}
	while (prog->angle >= 2 * M_PI) {
		prog->angle = prog->angle - (2 * M_PI);
	}
}

void UpdateUI(Program *prog) {
	int tempx, tempy;
	SDL_Rect UIRect;
	
    /* Calculate values for compass coordinates */
	tempx = RoundVal((COM_LEN * sin(prog->angle)) + COM_CEN);
	tempy = RoundVal((COM_LEN * cos(prog->angle)) + COM_CEN);

	/* Draw compass UI to the screen */
	Neill_SDL_SetDrawColour(prog->sw, GREY);
	Neill_SDL_RenderFillCircle(prog->sw->renderer, COM_CEN, COM_CEN, COM_RAD);
	Neill_SDL_SetDrawColour(prog->sw, RED);
	SDL_RenderDrawLine(prog->sw->renderer, COM_CEN, COM_CEN, tempx, tempy);
	Neill_SDL_SetDrawColour(prog->sw, BLACK);
	Neill_SDL_RenderFillCircle(prog->sw->renderer, COM_CEN, COM_CEN, SPO_RAD);
	/* Draw Pen up/down indicator UI to the screen */
	UIRect.h = UIRect.w = PUD_SIZ;
	UIRect.y = PUD_Y;
	UIRect.x = PUD_X;
	Neill_SDL_SetDrawColour(prog->sw, GREY);
	SDL_RenderFillRect(prog->sw->renderer, &UIRect);
	Neill_SDL_SetDrawColour(prog->sw, RED);
	SDL_RenderDrawLine(prog->sw->renderer, ARR_X, ARR_Y1, ARR_X, ARR_Y2);
	/*Calculate coordinates and draw the arrow in the correct direction */
	if (prog->pen_active == 1) {
		tempy = ARR_Y2;
	}
	else {
		tempy = ARR_Y1;
	}
	SDL_RenderDrawLine(prog->sw->renderer, ARR_X, tempy, AR_LX, COM_CEN);
	SDL_RenderDrawLine(prog->sw->renderer, ARR_X, tempy, AR_RX, COM_CEN);
	
	/* Render the updated graphics in the SDL window */
	SDL_RenderPresent(prog->sw->renderer);
	SDL_UpdateWindowSurface(prog->sw->win);
	/* Check if the user has attempted to exit the program */
	Neill_SDL_Events(prog->sw);
	if (prog->sw->finished) {
		atexit(SDL_Quit);
		exit(0);
	}
}
