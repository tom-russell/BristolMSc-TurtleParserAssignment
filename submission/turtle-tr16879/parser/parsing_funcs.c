#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

void ProgramMAIN(Program *prog)
{
	/* If the first token is not a "{", file is invalid */
	if (!StrMatch(CURRENTTOKEN, "{")) {
		ParseError("File Doesn't start with '{'.", prog);
	}

	/* If the first token is a "{", move to the next token, continue parsing */
	prog->curr = prog->curr + 1;
	ProgramINSTRCTLST(prog);
    
    /* Once parsing has finished check for remaining tokens */
    prog->curr = prog->curr + 1;
    if (!StrMatch(CURRENTTOKEN, "")) {
		ParseError("Tokens present after main instruction list braces have " 
        "closed.", prog);
	}
}

void ProgramINSTRCTLST(Program *prog)
{
	/* If the current token is "}" this instruction list has finished */
	if (StrMatch(CURRENTTOKEN, "}")) {
		return;
	}
	/* Otherwise, perform the next token instruction */
	ProgramINSTRUCTION(prog);
	/* Instruction list continues recursively until the end of the list */
	prog->curr = prog->curr + 1;
	ProgramINSTRCTLST(prog);
}

void ProgramINSTRUCTION(Program *prog)
{
	/* If the token is a valid instruction, perform the instruction */
	if (StrMatch(CURRENTTOKEN, "FD")) {
        if (TESTING == 1) printf("Forward\n");
        ProgramFD(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "LT")) {
        if (TESTING == 1) printf("Left Turn\n");
		ProgramLT(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "RT")) {
	if (TESTING == 1) printf("Right Turn\n");
		ProgramRT(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "DO")) {
        if (TESTING == 1) printf("DO Loop\n");
		ProgramDO(prog);
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "SET")) {
        if (TESTING == 1) printf("SET Statement\n");
        ProgramSET(prog);
		return;
	}

	/* If the token is not recognised, parsing has failed */
	ParseError("INSTRUCTION or '}' Expected.", prog);
}

void ProgramFD(Program *prog)
{
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }

	return;
}

void ProgramLT(Program *prog)
{
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }

	return;
}

void ProgramRT(Program *prog)
{
	prog->curr = prog->curr + 1;
	/* Check if the next token is a valid VAR/NUM */
	if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Number or Variable token expected.", prog);
    }

	return;
}

void ProgramDO(Program *prog)
{
	/* If looping variable token is invalid, then parsing has failed */
	prog->curr = prog->curr + 1;
	if (ProgramVAR(prog) == -1) {
		ParseError("Variable identifier token expected.", prog);
	}

	/* Parse 'FROM' token */
	CheckToken(prog, "FROM", "FROM token missing from 'DO' statement.");

	/* Parse loop start value token */
	prog->curr = prog->curr + 1;
	if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Loop start var/num token expected.", prog);
    }
	/* Parse 'TO' token */
	CheckToken(prog, "TO", "TO token missing from 'DO' statement.");

	/* Parse loop end value token */
	prog->curr = prog->curr + 1;
	if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Loop end var/num token expected.", prog);
    }

	/* Parse '{', instruction list open bracket token  */
	CheckToken(prog, "{", "'{' token missing from 'DO' statement.");

	/* Begin to parse the instruction list that follows the DO loop */
    prog->curr = prog->curr + 1;
    ProgramINSTRCTLST(prog);
}

void ProgramSET(Program *prog)
{
	/* Parse token for variable to be set */
	prog->curr = prog->curr + 1;
	if (ProgramVAR(prog) == -1) {
		ParseError("Expected Variable token to be SET.", prog);
	}

	/* Parse ':=' token */
	CheckToken(prog, ":=", "':=' token missing from 'SET' statement.");

	/* Calculate the polish expression */
	prog->curr = prog->curr + 1;
	ProgramPOLISH(prog);

	return;
}

void ProgramPOLISH(Program *prog)
{
	/* If the token is a ';' the polish statement has finished */
	if (StrMatch(CURRENTTOKEN, ";")) {
		return;
	}
	/* Check if the token is a valid operator */
	else if (ProgramOP(prog)) {
        if (TESTING == 1) printf("\tOP\n");
	}
	/* Check if the token is a variable/number */
	else {
		if (ProgramVARNUM(prog) == 0) {
	    /* If token is neither variable or number, exit the program */
	    ParseError("Expected Operator, VAR/NUM or ';' for polish expression.",
	                prog);
        }
	}

	/* Continue parsing the Polish expression */
	prog->curr = prog->curr + 1;
	ProgramPOLISH(prog);
	return;
}



int ProgramOP(Program *prog)
{
    char operator = CURRENTTOKEN[0];
    /* If the token is more than one character, it is not a valid operator */
    if  (strlen(CURRENTTOKEN) > 1) {
        return 0;
    }
    /* Return 1 only if the token is a valid operator */
    switch (operator) {
        case '+':
            return 1;
        case '-':
            return 1;
        case '*':
            return 1;
        case '/':
            return 1;
    }
    
	return 0;
}

int ProgramVARNUM(Program *prog)
{
	/* If the token is a single Uppercase letter, the token is valid */
	if (ProgramVAR(prog) != -1) {
        return 1;
	}
	/* Else if the token is a number, the token is valid */
	else if (ProgramNUM(prog)) {
        if (TESTING == 1) printf("\tNUM\n");
		return 1;
	}
	
	return 0;
}

int ProgramVAR(Program *prog)
{
	/* If the token is a single Uppercase letter token is a valid VAR */
	if (strlen(CURRENTTOKEN) == 1 && isupper(CURRENTTOKEN[0])) {
		if (TESTING == 1) printf("\tVAR\n");
        return 0;
	}/* If not, return a -1 to indicate the token is not a variable */
	else return -1;
}

int ProgramNUM(Program *prog)
{
	unsigned int i = 0, varlen, neg = 0, decimal = 0;
    varlen = strlen(CURRENTTOKEN);
    
	/* Allow the number to be valid if the first char is a minus symbol */
	if (CURRENTTOKEN[0] == '-') {
		i = 1;
		neg = 1;
	}

	/* Determine if the token is a valid integer or double value */
	for (; i < strlen(CURRENTTOKEN); i++) {
		/* If the character is not a digit and not a decimal point, the
		 * token is not a valid number. */
		if (isdigit(CURRENTTOKEN[i]) == 0 && CURRENTTOKEN[i] != '.') {
			return 0;
		}
		/* Allow one decimal point character to be part of a valid number */
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

    /* Return true (1) if the token is a valid number */
	return 1;
}
