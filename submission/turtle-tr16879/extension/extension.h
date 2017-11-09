#include "neillsdl2.h"

#define MAXSTACKSIZE 100
#define TOTALVARS 26
#define MAXTOKENLEN 10
#define STARTANGLE M_PI
#define TOLERANCE 0.000001

#define StrMatch(A, B) (strcmp(A, B) == 0)
#define CURRENTTOKEN prog->curr->str

typedef struct token_element {
	char str[MAXTOKENLEN + 1];
	struct token_element *next;
} Token;

typedef struct program {
	FILE *f_out;
	SDL_Simplewin *sw;
	double vars[TOTALVARS];
	int var_in_use[TOTALVARS];
	double xy[2];
	double angle;
	int pen_active; /* 1 = Pen down, 0 = Pen up */
	Token *curr;
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

/* General Functions */
void InitProgram(Program *prog, SDL_Simplewin *sw);
void LoadToken(Program *prog);
int CountTokens(FILE *fp);
void CheckToken(Program *prog, char token[], char error_text[]);
void FreeMemory(Token *start_token);
void ParseError(char *phrase, Program *prog);
void PushStack(P_stack *stack, Program *prog, double value);
double PopStack(P_stack *stack, Program *prog);
void TurtleMove(Program *prog, int xy_vals[2][2]);
int RoundVal(double value);
void AngleAdjust(Program *prog, double change);
void UpdateUI(Program *prog);
/* Parsing Functions */
void ProgramMAIN(Program *prog);
void ProgramINSTRCTLST(Program *prog);
void ProgramINSTRUCTION(Program *prog);
void ProgramFD(Program *prog);
void ProgramLT(Program *prog);
void ProgramRT(Program *prog);
void ProgramDO(Program *prog);
void PerformDOLoop(Program *prog, int var, double loop_end);
void ProgramSET(Program *prog);
void ProgramPOLISH(Program *prog, P_stack *stack);
int ProgramOP(Program *prog, P_stack *stack);
int ProgramVARNUM(Program *prog, double *value);
int ProgramVAR(Program *prog);
int ProgramNUM(Program *prog, double *num);
void ProgramPU(Program *prog);
void ProgramPD(Program *prog);
