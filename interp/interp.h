#include "neillsdl2.h"

#define MAXSTACKSIZE 100
#define TOTALVARS 26
#define TESTING 0
#define STARTANGLE M_PI

#define StrMatch(A, B) (strcmp(A, B) == 0)
#define CURRENTTOKEN prog->tokens[prog->curr]

typedef struct program {
	char **tokens;
	int total_tokens;
	int curr;
	double vars[TOTALVARS];
	int var_in_use[TOTALVARS];
	double xy[2];
	double angle;
} Program;

typedef struct polish_stack {
	double s[MAXSTACKSIZE];
	int top;
} P_stack;

enum Exit_Codes {
	token_len_error = 3, 
	no_input_file = 4,
	fopen_fail = 5,
	fclose_fail = 6,
	parse_fail = 7
};

enum Operators {
	OP_add, OP_sub, OP_mul, OP_div
};

/* General Functions */
void InitProgram(Program *prog, char *filename);
int CountTokens(FILE *fp);
void CheckToken(Program *prog, char token[], char error_text[]);
void FreeMemory(Program *prog);
void ParseError(char *phrase, Program *prog);
void PushStack(P_stack *stack, Program *prog, double value);
double PopStack(P_stack *stack, Program *prog);
void TurtleMove(SDL_Simplewin *sw, int xy_vals[2][2]);
int RoundVal(double value);
void AngleAdjust(Program *prog, double change);
/* Parsing Functions */
void ProgramMAIN(Program *prog, SDL_Simplewin *sw);
void ProgramINSTRCTLST(Program *prog, SDL_Simplewin *sw);
void ProgramINSTRUCTION(Program *prog, SDL_Simplewin *sw);
void ProgramFD(Program *prog, SDL_Simplewin *sw);
void ProgramLT(Program *prog);
void ProgramRT(Program *prog);
void ProgramDO(Program *prog, SDL_Simplewin *sw);
void PerformDOLoop(Program *prog, SDL_Simplewin *sw, int var, double loop_end);
void ProgramSET(Program *prog);
void ProgramPOLISH(Program *prog, P_stack *stack);
int ProgramOP(Program *prog, P_stack *stack);
double ProgramVARNUM(Program *prog);
int ProgramVAR(Program *prog);
int ProgramNUM(Program *prog, double *num);

