#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "extension.h"
#include <fcntl.h>

int main(int argc, char **argv)
{
	Program prog;
	SDL_Simplewin sw;
	Token start_token;

	start_token.next = NULL;
	prog.curr = &start_token;

	Neill_SDL_Init(&sw);

	/* Exit if not passed an input text file */
	if (argc != 2) {
		fprintf(stderr, "1 command line input file is required.\n");
		exit(no_input_file);
	}

	if ((prog.f_out = fopen(argv[1], "w")) == NULL) {
		fprintf(stderr, "Error occurred when opening input file.\n");
		exit(fopen_fail);
	}

	/* Initialise the program struct, then run the parser */
	InitProgram(&prog, &sw);
	ProgramMAIN(&prog);

	/* Wait until the user presses esc or closes the SDL window */
	printf("Parsing Complete. To exit close the SDL window.\n");
	while (!sw.finished) {
		Neill_SDL_Events(&sw);
	}

	/* Clear up memory & graphics subsystems */
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
	prog->pen_active = True;

	/* Initialise turtle coordinates to the window center */
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
	int valid = 0;

	DrawUI(prog);

	/* If next instruction has not been inputted yet */
	if (prog->curr->next == NULL) {
		prog->curr->next = malloc(sizeof(Token));
		prog->curr = prog->curr->next;
		prog->curr->next = NULL;

		while (valid == 0) {
			if (scanf("%s", CURRENTTOKEN) == 1) {
				valid = 1;
			}
			else {
				printf("Invalid Token Scanned.\n");
			}

			if (strlen(prog->curr) > MAXTOKENLEN) {
				printf("Token exceeds max token length.\n");
			}
		}
		fprintf(prog->f_out, "%s ", prog->curr);
	}
	/* Else, the program is in a DO loop so next instruction is already defined */
	else {
		prog->curr = prog->curr->next;
	}
}

void CheckToken(Program *prog, char token[], char error_text[])
{
	/* Check if the next token is the expected token,
	* if not then parsing has failed */
	LoadToken(prog);
	if (!StrMatch(CURRENTTOKEN, token)) {
		ParseError(error_text, prog);
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
	SDL_Delay(DELAYMS);
	Neill_SDL_SetDrawColour(prog->sw, 255, 255, 255);
	SDL_RenderDrawLine(prog->sw->renderer, xy_vals[0][0], xy_vals[0][1], xy_vals[1][0], xy_vals[1][1]);
	/* Render the updated graphics in the SDL window */
	SDL_RenderPresent(prog->sw->renderer);
	SDL_UpdateWindowSurface(prog->sw->win);
	Neill_SDL_Events(prog->sw);
	
	CalcLineMarkerXY(prog, xy_vals);
	Neill_SDL_SetDrawColour(prog->sw, 255, 0, 0);
	SDL_RenderDrawLine(prog->sw->renderer, 
						prog->line_marker_xy[0][0], prog->line_marker_xy[0][1],
						prog->line_marker_xy[1][0], prog->line_marker_xy[1][1]);

	DrawUI(prog);
}

void CalcLineMarkerXY(Program *prog, int xy_vals[2][2])
{
	double x, y;

	/* Colour the old marker back to white */
	Neill_SDL_SetDrawColour(prog->sw, 255, 255, 255);
	SDL_RenderDrawLine(prog->sw->renderer,
		prog->line_marker_xy[0][0], prog->line_marker_xy[0][1],
		prog->line_marker_xy[1][0], prog->line_marker_xy[1][1]);


	prog->line_marker_xy[1][0] = xy_vals[1][0];
	prog->line_marker_xy[1][1] = xy_vals[1][1];

	/* Work out the start x/y for the line marker */
	x = prog->xy[0] + (10 * -sin(prog->angle));
	y = prog->xy[1] + (10 * -cos(prog->angle));
	prog->line_marker_xy[0][0] = (int)RoundVal(x);
	prog->line_marker_xy[0][1] = (int)RoundVal(y);

	//printf("\n%d\t%d\t%d\t%d\n", )
}

int RoundVal(double value)
{
	double intpart, fractpart;
	int round_val = (int)value;

	fractpart = modf(value, &intpart);

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

	/* If the angle is outside the range 0 <= h < 2pi,
	* add/sub 2pi until it fits in the correct range */
	while (prog->angle < 0.0) {
		prog->angle = prog->angle + (2 * M_PI);
	}
	while (prog->angle >= 2 * M_PI) {
		prog->angle = prog->angle - (2 * M_PI);
	}
}

void DrawUI(Program *prog) {
	int tempx, tempy;
	SDL_Rect UIRect;

	tempx = RoundVal((25 * sin(prog->angle)) + 35);
	tempy = RoundVal((25 * cos(prog->angle)) + 35);

	/* Compass */
	Neill_SDL_SetDrawColour(prog->sw, 150, 150, 150);
	Neill_SDL_RenderFillCircle(prog->sw->renderer, 35, 35, 30);
	Neill_SDL_SetDrawColour(prog->sw, 255, 0, 0);
	SDL_RenderDrawLine(prog->sw->renderer, 35, 35, tempx, tempy);
	Neill_SDL_SetDrawColour(prog->sw, 0, 0, 0);
	Neill_SDL_RenderFillCircle(prog->sw->renderer, 35, 35, 4);
	/* Pen up/down indicator */
	UIRect.h = UIRect.w = 60;
	UIRect.y = 5;
	UIRect.x = 70;
	Neill_SDL_SetDrawColour(prog->sw, 150, 150, 150);
	SDL_RenderFillRect(prog->sw->renderer, &UIRect);
	Neill_SDL_SetDrawColour(prog->sw, 255, 0, 0);
	SDL_RenderDrawLine(prog->sw->renderer, 100, 15, 100, 55);
	if (prog->pen_active == True) {
		tempy = 55;
	}
	else {
		tempy = 15;
	}
	SDL_RenderDrawLine(prog->sw->renderer, 100, tempy, 80, 35);
	SDL_RenderDrawLine(prog->sw->renderer, 100, tempy, 120, 35);
	/* Render the updated graphics in the SDL window */
	SDL_RenderPresent(prog->sw->renderer);
	SDL_UpdateWindowSurface(prog->sw->win);
	Neill_SDL_Events(prog->sw);
	if (prog->sw->finished) {
		atexit(SDL_Quit);
		exit(0);
	}
}