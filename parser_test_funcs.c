#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "CUnit/Basic.h"

#define MAXPROGLEN 100
#define MAXTOKENLEN 10
#define NUMBERBASE 10
#define TOTALVARS 26
#define FNAME1 "turtletest1.txt"
#define FNAME2 "turtletest2.txt"
#define FNAME3 "turtletest3.txt"
#define FNAME4 "turtletest4.txt"
#define FNAME5 "turtletest5.txt"
#define F1TKNS 10
#define F2TKNS 9
#define F3TKNS 34
#define TOTALTESTS 5

#define StrMatch(A, B) (strcmp(A, B) == 0)
#define CURRENTTOKEN prog->tokens[prog->curr]
#define ERROR(phrase) {fprintf(stderr, "%s\nParsing error occurs in file %s on line %d. Input file token %d", phrase, __FILE__, __LINE__, prog->curr + 1);}

struct program {
	char tokens[MAXPROGLEN][MAXTOKENLEN];
	int *vars;
	int curr;
}typedef Program;

enum Exit_Codes {
    fopen_fail = 5,
    fclose_fail = 6
};

void InitProgram(Program *prog, char file_name[]);
/* Formal Grammar Functions */
void ProgramMAIN(Program *prog);
void ProgramINSTRCTLST(Program *prog);
void ProgramINSTRUCTION(Program *prog);
void ProgramFD(Program *prog);
void ProgramLT(Program *prog);
void ProgramRT(Program *prog);
void ProgramDO(Program *prog);
void ProgramSET(Program *prog);
int ProgramVARNUM(Program *prog);
int ProgramVAR(Program *prog);

/*Error testing global variables */
int ERRORTest = 0;
int FunctionCalls = 0;
int total_tokens[TOTALTESTS] = {F1TKNS, F2TKNS, F3TKNS};
int ProgramFDCalled = 0;
int ProgramLTCalled = 0;
int ProgramRTCalled = 0;

/* The suite initialization function.
 */
int init_suite1(void)
{
   return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
   return 0;
}

void InitTestStructs(Program prog[]) {
    
    //prog = malloc(sizeof(Program[3]));
    InitProgram(&prog[0], FNAME1);
    InitProgram(&prog[1], FNAME2);
    InitProgram(&prog[2], FNAME3);
}
        
void Test_InitProgram(void) {
    int t, i;
    
    
    Program prog[TOTALTESTS];
    InitTestStructs(prog);
    
    for (t = 0; t < TOTALTESTS; t++) {
        /* Check that curr variable is set to 0 */
        CU_ASSERT(prog[t].curr == 0);
        /* Check that all of the vars variables are set to 0 */
        for (i = 0; i < TOTALVARS; i++) {
            CU_ASSERT(prog[t].vars[i] == 0);
        }
        for (i = 0; i < MAXPROGLEN; i++){
            /* Check that the tokens have been copied into the token array */
            if (i < total_tokens[t]) {
                CU_ASSERT_STRING_NOT_EQUAL(prog[t].tokens[i], "");
            }
            /* Check that the empty token strings were set to null */
            else {
                CU_ASSERT_STRING_EQUAL(prog[t].tokens[i], "");
            }
        }
    }
}

void Test_ProgramMAIN(void) {
    int t, i, start_curr;
    Program prog[TOTALTESTS];
    InitTestStructs(prog);
    
    for (t = 0; t < TOTALTESTS; t++) {
        start_curr = prog[t].curr;
        ProgramMAIN(&prog[t]);
        /* Check that the function gives no error when program starts with a "{"
         * and gives an error if the program starts without a "{".
         */
        if (StrMatch(prog[t].tokens[0], "{")) {
            CU_ASSERT_FALSE(ERRORTest);
            /* Check that the current variable is correctly incrementing to 1 */
            CU_ASSERT_EQUAL(prog[t].curr, start_curr + 1);
            CU_ASSERT(prog[t].curr == 1);
        }
        else {
            CU_ASSERT_TRUE(ERRORTest);
        }
        
    }
}

void Test_ProgramINSTRCTLST(void) {
    int t, i;
    Program prog[TOTALTESTS];
    InitTestStructs(prog);
    
    for (t = 0; t < TOTALTESTS; t++) {
        FunctionCalls = 0;
        ProgramINSTRCTLST(&prog[t]);
        
        CU_ASSERT_EQUAL(total_tokens[t], FunctionCalls);
    }
}

void Test_ProgramINSTRUCTION(void) {
    int t, i;
    Program prog[TOTALTESTS];
    InitTestStructs(prog);
    
    for (t = 0; t < TOTALTESTS; t++) {
        ProgramFDCalled = 0;
        ProgramLTCalled = 0;
        ProgramRTCalled = 0;
        ERRORTest = 0;
        
        ProgramINSTRUCTION(&prog[t]);
        
        /* If the token is FD, only ProgramFD should be called */
        if (StrMatch(CURRENTTOKEN, "FD")) {
            CU_ASSERT_TRUE(ProgramFDCalled);
            CU_ASSERT_FALSE(ProgramLTCalled);
            CU_ASSERT_FALSE(ProgramRTCalled);
            CU_ASSERT_FALSE(ERRORTest);
        }/* If the token is LT, only ProgramRT should be called */
        else if (StrMatch(CURRENTTOKEN, "LT")) {
            CU_ASSERT_TRUE(ProgramLTCalled);
            CU_ASSERT_FALSE(ProgramFDCalled);
            CU_ASSERT_FALSE(ProgramRTCalled);
            CU_ASSERT_FALSE(ERRORTest);
        }/* If the token is RT, only ProgramRT should be called */
        else if (StrMatch(CURRENTTOKEN, "RT")) {
            CU_ASSERT_TRUE(ProgramRTCalled);
            CU_ASSERT_FALSE(ProgramFDCalled);
            CU_ASSERT_FALSE(ProgramLTCalled);
            CU_ASSERT_FALSE(ERRORTest);
        }
        else {/* If the token was not an instruction, an error should be thrown */
            CU_ASSERT_TRUE(ERRORTest);
            CU_ASSERT_FALSE(ProgramFDCalled);
            CU_ASSERT_FALSE(ProgramLTCalled);
            CU_ASSERT_FALSE(ProgramRTCalled);
        }
    }
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite Suite_ProgramTests = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   Suite_ProgramTests = CU_add_suite("Program_Tests", init_suite1, clean_suite1);
   if (NULL == Suite_ProgramTests) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(Suite_ProgramTests, "test of ProgramMain", Test_ProgramMAIN))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

void InitProgram(Program *prog, char file_name[])
{

	int i;
	FILE *fp;
    
    /* Set the 'current' variable to the first token (0) */
    prog->curr = 0;
    
    /* Initialise all token strings to null */
	for (i = 0; i < MAXPROGLEN; i++) {
		prog->tokens[i][0] = '\0';
	}
	/* Initialise all VAR variables in the array to 0 */
	prog->vars = calloc(TOTALVARS, sizeof(int));
	
	if ((fp = fopen(file_name, "r")) == NULL) {
		exit(fopen_fail);
	}
	/* Copy all tokens from file into the token array */
	i = 0;
	while (fscanf(fp, "%s", prog->tokens[i++]) == 1 && i < MAXPROGLEN);
    
	if (fclose(fp) != 0) {
		exit(fclose_fail);
	}
}

void ProgramMAIN(Program *prog)
{
    /* If the first token is not a "{", exit the program */
	if (!StrMatch(CURRENTTOKEN, "{")) {
		ERRORTest = 1;
        ERROR("File Doesn't start with '{'.");
        return;
	}
    /* If the first token is correct, move to the next token, continue parsing */
	prog->curr = prog->curr + 1;
	ERRORTest = 0;
}

void ProgramINSTRCTLST(Program *prog)
{
/* If the current token is "}" parsing of this instruction list has completed */
	if (StrMatch(CURRENTTOKEN, "}")) {
		return;
	}
    /* Otherwise, perform the instruction of the current token */
	FunctionCalls++;
    prog->curr = prog->curr + 1;
    /* Function calls itself, to recursively continue the instruction list */
	prog->curr = prog->curr + 1;
	ProgramINSTRCTLST(prog);
}

void ProgramINSTRUCTION(Program *prog)
{
    /* If the token is a recognised instruction, perform the instruction */
	if (StrMatch(CURRENTTOKEN, "FD")) {
		//ProgramFD(prog);
        ProgramFDCalled = 1;
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "LT")) {
		//ProgramLT(prog);
        ProgramLTCalled = 1;
		return;
	}
	else if (StrMatch(CURRENTTOKEN, "RT")) {
		//ProgramRT(prog);
        ProgramRTCalled = 1;
		return;
	}
    
    /* If the token is not recognised, exit the program */
	//ERROR("INSTRUCTION or '}' Expected.");
    ERRORTest = 1;
}

void ProgramFD(Program *prog)
{
	int val;
	prog->curr = prog->curr + 1;
	val = ProgramVARNUM(prog);
	printf("Forward %d\n", val);
    

	return;
}

void ProgramLT(Program *prog)
{
	int val;
	prog->curr = prog->curr + 1;
	val = ProgramVARNUM(prog);
	printf("Left Turn %d degrees\n", val);

	return;
}

void ProgramRT(Program *prog)
{
	int val;
    prog->curr = prog->curr + 1;
	val = ProgramVARNUM(prog);
	printf("Right Turn %d degrees\n", val);

	return;
}

void ProgramDO(Program *prog)
{
    int var, from, to;
    
    /* variable */
    prog->curr = prog->curr + 1;
    if ((var = ProgramVAR(prog)) == '0') {
		ERROR("Variable token expected.");
	}
    
    /* FROM */
    prog->curr = prog->curr + 1;
    if (!StrMatch(CURRENTTOKEN, "FROM")) {
		ERROR("'FROM' token expected.");
	}
    
    /* from value */
    prog->curr = prog->curr + 1;
    from = ProgramVARNUM(prog);
    
    /* TO */
    prog->curr = prog->curr + 1;
    if (!StrMatch(CURRENTTOKEN, "TO")) {
		ERROR("'TO' token expected.");
	}
    
    /* to value */
    prog->curr = prog->curr + 1;
    to = ProgramVARNUM(prog);
    
    /* Instruction list open bracket '{' */
    prog->curr = prog->curr + 1;
    if (!StrMatch(CURRENTTOKEN, "{")) {
		ERROR("'{' token expected.");
	}
    
    ProgramINSTRCTLST(prog);
}

void ProgramSET(Program *prog)
{
	return;
}

int ProgramVARNUM(Program *prog)
{
	int i, varlen, cnt = 0;
    char var;
	varlen = strlen(CURRENTTOKEN);

	/* If the token is a variable, return the variable value */


	/* Else check if the token is a valid integer number */
	for (i = 0; i < varlen; i++) {
		if (isdigit(CURRENTTOKEN[i]) != 0) {
			cnt++;
		}
		/* Return the integer value of the token if it is a valid number */
		if (cnt == varlen) {
			return strtol(CURRENTTOKEN, NULL, NUMBERBASE);
		}
	}

	/* If the token is a single Uppercase letter, return the corresponding variable */
	if ((var = ProgramVAR(prog)) != -1) {
		return prog->vars[var];
	}

	/* If token is neither variable or number, exit the program */
	ERROR("Number or Variable token expected.");
}

int ProgramVAR(Program *prog) {
	int varlen;
	varlen = strlen(CURRENTTOKEN);

	if (varlen == 1 && isupper(CURRENTTOKEN[0])) {
		return CURRENTTOKEN[0] - 'A';
	}
	else return -1;
}



