#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "interp.h"

#define DegreeToRads(A) ( (A * M_PI) / 180.0 )

void ProgramMAIN(Program *prog, SDL_Simplewin *sw)
{
    /* IF the first token is not a "{", file is invalid */
	if (!StrMatch(CURRENTTOKEN, "{")) {
		ParseError("File Doesn't start with '{'.", prog);
	}

	/* If the first token is a "{", move to the next token, continue parsing */
	prog->curr = prog->curr + 1;
	ProgramINSTRCTLST(prog, sw);

	/* Once parsing has finished check for remaining tokens */
	prog->curr = prog->curr + 1;
	if (!StrMatch(CURRENTTOKEN, "")) {
		ParseError("Tokens present after main instruction list braces have "
			"closed.", prog);
	}
}

void ProgramINSTRCTLST(Program *prog, SDL_Simplewin *sw)
{
	/* Check once per instruction if user has attempted to quit */
	Neill_SDL_Events(sw);
	if (sw->finished) {
		atexit(SDL_Quit);
		FreeMemory(prog);
		exit(0);
	}

	/* If the current token is "}" this instruction list has finished */
	if (StrMatch(CURRENTTOKEN, "}")) {
		return;
	}
	/* Otherwise, perform the next token instruction */
	ProgramINSTRUCTION(prog, sw);
	/* Instruction list continues recursively until the end of the list */
	prog->curr = prog->curr + 1;
	ProgramINSTRCTLST(prog, sw);
}

void ProgramINSTRUCTION(Program *prog, SDL_Simplewin *sw)
{
	/* If the token is a valid instruction, perform the instruction */
	if (StrMatch(CURRENTTOKEN, "FD")) {
		ProgramFD(prog, sw);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "LT")) {
		ProgramLT(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "RT")) {
		ProgramRT(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "DO")) {
		ProgramDO(prog, sw);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "SET")) {
		ProgramSET(prog);
		return;
	}

	/* If the token is not recognised, file is invalid */
	ParseError("INSTRUCTION or '}' Expected.", prog);
}

void ProgramFD(Program *prog, SDL_Simplewin *sw)
{
	int xy_vals[2][2];
	double value;
	
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog, &value) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }
	
	/* Store previous x/y coordinates as integers */
	xy_vals[0][0] = RoundVal(prog->xy[0]);
	xy_vals[0][1] = RoundVal(prog->xy[1]);
	/* Calculate the new x/y coordinates and move the 'turtle' */
	prog->xy[0] += value * sin(prog->angle);
	prog->xy[1] += value * cos(prog->angle);
	/* Store the new x/y coordinates as integers */
	xy_vals[1][0] = RoundVal(prog->xy[0]);
	xy_vals[1][1] = RoundVal(prog->xy[1]);
	
	/* Use the previous and current x/y coordinates to move the turtle in SDL */
	TurtleMove(sw, xy_vals);

	return;
}

void ProgramLT(Program *prog)
{
	double value;
	
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog, &value) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }
	
	/* Change the angle value by the given amount (left turn is positive) */
	AngleAdjust(prog, DegreeToRads(value));

	return;
}

void ProgramRT(Program *prog)
{
	double value;
	
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog, &value) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }
    
	/* Change the angle value by the given amount (right turn is negative) */
	AngleAdjust(prog, -DegreeToRads(value));

	return;
}

void ProgramDO(Program *prog, SDL_Simplewin *sw)
{
	int var;
	double loop_end;

	/* If looping variable token is invalid, then parsing has failed */
	prog->curr = prog->curr + 1;
	if ((var = ProgramVAR(prog)) == -1) {
		ParseError("Variable identifier token expected.", prog);
	}
	/* If variable is already 'in use' then the file is invalid, otherwise
	 * continue & set variable to 'in use' */
	if (prog->var_in_use[var] == 1) {
		ParseError("You cannot modify a variable already in use.", prog);
	}
	prog->var_in_use[var] = 1;

	/* Parse 'FROM' token */
	CheckToken(prog, "FROM", "FROM token missing from 'DO' statement.");

	/* Parse loop start value token */
	prog->curr = prog->curr + 1;
    if (ProgramVARNUM(prog, &prog->vars[var]) == 0) {
        /* If token is neither variable or number, exit the program */
        ParseError("Loop start var/num token expected.", prog);
    }

	/* Parse 'TO' token */
	CheckToken(prog, "TO", "TO token missing from 'DO' statement.");

	/* Parse loop end value token */
	prog->curr = prog->curr + 1;
	if (ProgramVARNUM(prog, &loop_end) == 0) {
        /* If token is neither variable or number, exit the program */
        ParseError("Loop start var/num token expected.", prog);
    }

	/* Parse '{', instruction list open bracket token  */
	CheckToken(prog, "{", "'{' token missing from 'DO' statement.");

	/* Perform the specified do loop, then free the variable for use again */
	PerformDOLoop(prog, sw, var, loop_end);
	prog->var_in_use[var] = 0;
}

void PerformDOLoop(Program *prog, SDL_Simplewin *sw, int var, double loop_end)
{
	int loopstart;

	/* Check if the loop range is valid (loop end > loop start) */
	if (prog->vars[var] > loop_end) {
		ParseError("Loop range must fit the format: start <= end.", prog);
	}

	/* Keep a reference to the start of the instruction loop */
	prog->curr = prog->curr + 1;
	loopstart = prog->curr;

	/* Var starts at 1 less, since increment occurs at the start of the loop */
	prog->vars[var]--;

	/* Loop the given Variable between the two given values */
	while (prog->vars[var] < loop_end) {
		prog->vars[var]++;
		/* Set the current token to the first instruction of the DO loop,
		 * then perform the instruction list for the loop. */
		prog->curr = loopstart;
		ProgramINSTRCTLST(prog, sw);
	}
}

void ProgramSET(Program *prog)
{
	int var;
	P_stack stack;
	stack.top = 0;

	/* Parse token for variable to be set */
	prog->curr = prog->curr + 1;
	if ((var = ProgramVAR(prog)) == -1) {
		ParseError("Variable identifier token expected.", prog);
	}
	/* Check if variable is in use before continuing */
	if (prog->var_in_use[var] == 1) {
		ParseError("You cannot modify a variable already in use.", prog);
	}

	/* Parse ':=' token */
	CheckToken(prog, ":=", "':=' token missing from 'SET' statement.");

	/* Calculate the polish expression */
	prog->curr = prog->curr + 1;
	ProgramPOLISH(prog, &stack);

	/* Set the variable to the result of the polish expression */
	prog->vars[var] = PopStack(&stack, prog);

	return;
}

void ProgramPOLISH(Program *prog, P_stack *stack)
{
    double value;
    
	/* If the token is a ';' the polish statement has finished */
	if (StrMatch(CURRENTTOKEN, ";")) {
		if (stack->top != 1) {
			ParseError("Invalid Polish expression, too few operators.", prog);
		}
		return;
	}
	/* If the token is a variable/number, add the value to the stack */
	else if (ProgramVARNUM(prog, &value) == 1){
		PushStack(stack, prog, value);
	}
	/* If the token is a valid operator, perform the operation */
	else {
	    if (ProgramOP(prog, stack) == 0) {
	        /* If token is neither VAR/NUM or operator, parsing has failed */
	        ParseError("Expected Operator, VAR/NUM or ';' for Polish "
	                   "expression.", prog);
        }
	}

	/* Continue parsing the Polish expression */
	prog->curr = prog->curr + 1;
	ProgramPOLISH(prog, stack);
	return;
}

int ProgramOP(Program *prog, P_stack *stack)
{
	double val1, val2;
	char operator = CURRENTTOKEN[0];
	
	/* If the token is more than one character, it is not a valid operator */
	if (strlen(CURRENTTOKEN) > 1) {
		return 0;
	}
	/* If the token is a valid operator, perform the operation on the top 2
	* stack values and place the result on the stack */
	switch (operator) {
	case '+':
		val2 = PopStack(stack, prog);
		val1 = PopStack(stack, prog);
		PushStack(stack, prog, val1 + val2);
		return 1;
	case '-':
		val2 = PopStack(stack, prog);
		val1 = PopStack(stack, prog);
		PushStack(stack, prog, val1 - val2);
		return 1;
	case '*':
		val2 = PopStack(stack, prog);
		val1 = PopStack(stack, prog);
		PushStack(stack, prog, val1 * val2);
		return 1;
	case '/':
		val2 = PopStack(stack, prog);
		val1 = PopStack(stack, prog);
        /* Exit if user is attempting to divide by 0 */
		if (val2 > -TOLERANCE && val2 < TOLERANCE) {
			ParseError("Attempted division by 0.\n", prog);
		}
		PushStack(stack, prog, val1 / val2);
		return 1;
	}

	return 0;
}

int ProgramVARNUM(Program *prog, double *value)
{
	int var;
	double num;

	/* If the token is a single Uppercase letter, set 'value' to the specified
	 * variable's current value */
	if ((var = ProgramVAR(prog)) >= 0) {
		*value = prog->vars[var];
		return 1;
	}
	/* If the token is a valid number, set value to the number */
	else if (ProgramNUM(prog, &num)) {
		*value = num;
		return 1;
	}
	
	/* If the token is neither a VAR nor a NUM, return false (0) */
	return 0;
}

int ProgramVAR(Program *prog)
{
	int varlen;
	varlen = strlen(CURRENTTOKEN);

	/* If the token is a single Uppercase letter, return the variable */
	if (varlen == 1 && isupper(CURRENTTOKEN[0])) {
		return CURRENTTOKEN[0] - 'A';
	}/* If not, return a -1 to indicate the token is not a variable */
	else return -1;
}

int ProgramNUM(Program *prog, double *num)
{
	int i = 0, varlen, neg = 0, decimal = 0;
	varlen = strlen(CURRENTTOKEN);

	/* Allow the number to be valid if the first char is a minus symbol */
	if (CURRENTTOKEN[0] == '-') {
		i = 1;
		neg = 1;
	}

	/* Determine if the token is a valid integer or double value */
	for (; i < varlen; i++) {
		/* If the character is not a digit and not the first decimal place,
		* the token is not a valid number. */
		if (isdigit(CURRENTTOKEN[i]) == 0 && CURRENTTOKEN[i] != '.') {
			return 0;
		}
		/* Allow one decimal point character '.' to be part of a valid number */
		else if (CURRENTTOKEN[i] == '.') {
			decimal++;
			if (decimal > 1) {
				return 0;
			}
		}
	}
    /* Disallow tokens with no digits, ".", "-", "-." */
    if (varlen == (decimal + neg)) {
        return 0;
    }

	/* If token is a valid number, set the value of num and return */
	*num = atof(CURRENTTOKEN);
	return 1;
}
