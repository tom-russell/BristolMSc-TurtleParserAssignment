#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "extension.h"

#define DegreeToRads(A) ( (A * M_PI) / 180.0 )

void ProgramMAIN(Program *prog)
{
	LoadToken(prog);
	if (!StrMatch(CURRENTTOKEN, "{")) {
		ParseError("File Doesn't start with '{'.", prog);
	}

	/* If the first token is a "{", move to the next token, continue parsing */
	LoadToken(prog);

	ProgramINSTRCTLST(prog);
}

void ProgramINSTRCTLST(Program *prog)
{
	/* If the current token is "}" this instruction list has ended */
	if (StrMatch(CURRENTTOKEN, "}")) {
		return;
	}
	/* Otherwise, perform the next token instruction */
	ProgramINSTRUCTION(prog);
	/* Instruction list continues recursively until the end of the list */
	LoadToken(prog);
	ProgramINSTRCTLST(prog);
}

void ProgramINSTRUCTION(Program *prog)
{
	/* If the token is a valid instruction, perform the instruction */
	if (StrMatch(CURRENTTOKEN, "FD")) {
		ProgramFD(prog);
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
		ProgramDO(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "SET")) {
		ProgramSET(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "PU")) {
		ProgramPU(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "PD")) {
		ProgramPD(prog);
		return;
	}

	/* If the token is not recognised, parsing has failed */
	ParseError("INSTRUCTION or '}' Expected.", prog);
}

void ProgramFD(Program *prog)
{
	int xy_vals[2][2];
	double value;
	LoadToken(prog);
	/* Retrieve the value of the variable or number token */
	value = ProgramVARNUM(prog);
	/* Save previous xy coordinates */
	xy_vals[0][0] = (int)RoundVal(prog->xy[0]);
	xy_vals[0][1] = (int)RoundVal(prog->xy[1]);
	/* Update the x/y coordinates and move the 'turtle' */
	prog->xy[0] += value * sin(prog->angle);
	prog->xy[1] += value * cos(prog->angle);
	xy_vals[1][0] = (int)RoundVal(prog->xy[0]);
	xy_vals[1][1] = (int)RoundVal(prog->xy[1]);
	if (TESTING == 1) printf("Coordinates - X: %.2f, Y: %.2f\n", prog->xy[0] - 400, prog->xy[1] - 300);
	
	if (prog->pen_active == True) {
		TurtleMove(prog, xy_vals);
	}

	return;
}

void ProgramLT(Program *prog)
{
	double value;
	LoadToken(prog);
	/* Retrieve the value of the variable or number token */
	value = ProgramVARNUM(prog);
	/* Change the angle value by the given amount (left turn is positive) */
	AngleAdjust(prog, DegreeToRads(value));
	if (TESTING == 1) printf("angle: %.3f\n", (prog->angle * 180) / M_PI);
	DrawUI(prog);
	return;
}

void ProgramRT(Program *prog)
{
	double value;
	LoadToken(prog);
	/* Retrieve the value of the variable or number token */
	value = ProgramVARNUM(prog);
	/* Change the angle value by the given amount (right turn is negative) */
	AngleAdjust(prog, -DegreeToRads(value));
	if (TESTING == 1) printf("angle: %.3f\n", (prog->angle * 180) / M_PI);
	DrawUI(prog);
	return;
}

void ProgramDO(Program *prog)
{
	int var;
	double loop_end;

	/* If looping variable token is invalid, then parsing has failed */
	LoadToken(prog);
	if ((var = ProgramVAR(prog)) == -1) {
		ParseError("Variable identifier token expected.", prog);
	}
	/* If variable is in use parsing has failed, else continue & set var as in use */
	if (prog->var_in_use[var] == 1) {
		ParseError("You cannot modify a variable already in use.", prog);
	}
	prog->var_in_use[var] = 1;

	/* Parse 'FROM' token */
	CheckToken(prog, "FROM", "FROM token missing from 'DO' statement.");

	/* Parse loop start value token */
	LoadToken(prog);
	prog->vars[var] = ProgramVARNUM(prog);

	/* Parse 'TO' token */
	CheckToken(prog, "TO", "TO token missing from 'DO' statement.");

	/* Parse loop end value token */
	LoadToken(prog);
	loop_end = ProgramVARNUM(prog);

	/* Parse '{', instruction list open bracket token  */
	CheckToken(prog, "{", "'{' token missing from 'DO' statement.");

	/* Perform the specified do loop, then free the variable for use again */
	PerformDOLoop(prog, var, loop_end);
	prog->var_in_use[var] = 0;
}

void PerformDOLoop(Program *prog, int var, double loop_end)
{
	Token *loopstart;

	/* Check if the loop range is valid (loop end is a bigger number than loop start) */
	if (prog->vars[var] > loop_end) {
		ParseError("Loop range must fit the format: start <= end.", prog);
	}

	/* Keep a reference to the start of the instruction loop */
	LoadToken(prog);
	loopstart = prog->curr;

	/* Var starts at 1 less, since the increment occurs at the start of the loop */
	prog->vars[var]--;

	/* Loop the given Variable between the two given values */
	while (prog->vars[var] < loop_end) {
		prog->vars[var]++;
		/* Set the current token to the first instruction of the DO loop,
		* then perform the instruction list for the loop. */
		prog->curr = loopstart;
		ProgramINSTRCTLST(prog);
	}
}

void ProgramSET(Program *prog)
{
	int var;
	P_stack stack;
	stack.top = 0;

	/* Parse token for variable to be set */
	LoadToken(prog);
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
	LoadToken(prog);
	ProgramPOLISH(prog, &stack);

	/* Set the variable to the result of the polish expression */
	prog->vars[var] = PopStack(&stack, prog);

	return;
}

void ProgramPOLISH(Program *prog, P_stack *stack)
{
	/* If the token is a ';' the polish statement has finished */
	if (StrMatch(CURRENTTOKEN, ";")) {
		if (stack->top != 1) {
			ParseError("Invalid Polish expression, too few operators.", prog);
		}
		return;
	}
	/* If the token is a valid operator, perform the operation */
	else if (ProgramOP(prog, stack)) {
	}
	/* If the token is a variable/number, add the value to the stack */
	else {
		PushStack(stack, prog, ProgramVARNUM(prog));
	}

	/* Continue parsing the Polish expression */
	LoadToken(prog);
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
		if (val2 > -0.000001 && val2 < 0.000001) {
			ParseError("Attempted division by 0.\n", prog);
		}
		PushStack(stack, prog, val1 / val2);
		return 1;
	}

	return 0;
}

double ProgramVARNUM(Program *prog)
{
	int var;
	double num;

	/* If the token is a single Uppercase letter, return the variable value */
	if ((var = ProgramVAR(prog)) >= 0) {
		return prog->vars[var];
	}
	/* If the token is a valid number, return the value of the number */
	else if (ProgramNUM(prog, &num)) {
		return num;
	}

	/* If token is neither variable or number, exit the program */
	ParseError("Number or Variable token expected.", prog);
	return 0.0;
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
	/* Disallow tokens with no digits, (.), (-), (-.) */
	if (varlen == (decimal + neg)) {
		return 0;
	}

	/* If token is a valid number, set the value of num and return */
	*num = atof(CURRENTTOKEN);
	return 1;
}

void ProgramPU(Program *prog) {
	/* Lift the pen so no line is drawn during an FD instruction */
	prog->pen_active = False;
}

void ProgramPD(Program *prog) {
	/* Drop the pen so a line is drawn during an FD instruction */
	prog->pen_active = True;
}