#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interp.h"

#define MAXTOKENLEN 10
#define DELAYMS 10

int main(int argc, char **argv)
{
	Program prog;
	SDL_Simplewin sw;

	Neill_SDL_Init(&sw);

	/* Exit if not passed an input text file */
	if (argc != 2) {
		fprintf(stderr, "1 command line input file is required.\n");
		exit(no_input_file);
	}

	/* Initialise the program struct, then run the parser */
	InitProgram(&prog, argv[1]);
	ProgramMAIN(&prog, &sw);

	/* Wait until the user presses esc or closes the SDL window */
	while (!sw.finished) {
		Neill_SDL_Events(&sw);
	}
	
	/* Clear up memory & graphics subsystems */
	FreeMemory(&prog);
	atexit(SDL_Quit);

	return 0;
}

void InitProgram(Program *prog, char *filename)
{
	int i = 0;
	FILE *fp;

	/* Set the 'current token' variable to the first token (0) */
	prog->curr = 0;
	/* Initialise turtle coordinates to the window center */
	prog->xy[0] = WWIDTH / 2.0; 
	prog->xy[1] = WHEIGHT / 2.0;
	prog->angle = STARTANGLE;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error occurred when opening input file.\n");
		exit(fopen_fail);
	}
	/* Count the total number of tokens & allocate space for the array */
	prog->total_tokens = CountTokens(fp);
	prog->tokens = (char **)malloc((prog->total_tokens + 1) * sizeof(char *));
	for (i = 0; i < prog->total_tokens + 1; i++) {
		prog->tokens[i] = (char *)calloc(MAXTOKENLEN, sizeof(char));
	}
	/* Copy all tokens from file into the token array */
	i = 0;
	while (fscanf(fp, "%s", prog->tokens[i++]) == 1);

	if (fclose(fp) != 0) {
		fprintf(stderr, "Error occurred when closing input file.\n");
		FreeMemory(prog);
		exit(fclose_fail);
	}

	/* Initialise all VAR variables and the vars_in_use in the array to 0 */
	for (i = 0; i < TOTALVARS; i++) {
		prog->vars[i] = 0.0;
		prog->var_in_use[i] = 0;
	}
}

int CountTokens(FILE *fp)
{
	int count = 0;
	char string[MAXTOKENLEN + 1];

	/* Count the number of tokens & check they are a valid length */
	while (fscanf(fp, "%s", string) == 1) {
		if (strlen(string) > MAXTOKENLEN) {
			fprintf(stderr, "Token exceeds max token length.\n");
			exit(token_len_error);
		}
		count++;
	}
	rewind(fp);
	return count;
}

void CheckToken(Program *prog, char token[], char error_text[])
{
	/* Check if the next token is the expected token,
	* if not then parsing has failed */
	prog->curr = prog->curr + 1;
	if (!StrMatch(CURRENTTOKEN, token)) {
		ParseError(error_text, prog);
	}
}

void FreeMemory(Program *prog)
{
	int i;

	/* Free all individual token strings */
	for (i = 0; i < prog->total_tokens + 1; i++) {
		free(prog->tokens[i]);
	}
	/* Free the token pointer array */
	free(prog->tokens);
}

void ParseError(char *phrase, Program *prog)
{
	/* Print the error statement, free allocated memory then exit the program */
	fprintf(stderr, "%s\nParse error occurs on token %d.\n", phrase, prog->curr + 1);
	FreeMemory(prog);
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

void TurtleMove(SDL_Simplewin *sw, int xy_vals[2][2])
{
	SDL_Delay(DELAYMS);
	Neill_SDL_SetDrawColour(sw, 255, 255, 255);

	SDL_RenderDrawLine(sw->renderer, xy_vals[0][0] , xy_vals[0][1], xy_vals[1][0], xy_vals[1][1]);
	/* Render the updated graphics in the SDL window */
	SDL_RenderPresent(sw->renderer);
	SDL_UpdateWindowSurface(sw->win);
	Neill_SDL_Events(sw);

	/* Exit if user has attempted to quit program */
	if (sw->finished) {
		atexit(SDL_Quit);
		exit(0);
	}
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
