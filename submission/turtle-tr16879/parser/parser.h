#define MAXSTACKSIZE 100
#define TOTALVARS 26
#define TESTING 0    /* TESTING = 0 TURNS OFF PRINTOUT STATEMENTS */

#define StrMatch(A, B) (strcmp(A, B) == 0)
#define CURRENTTOKEN prog->tokens[prog->curr]

typedef struct program {
	char **tokens;
	int total_tokens;
	int curr;

} Program;

enum Exit_Codes {
	token_len_error = 3, 
	no_input_file = 4,
	fopen_fail = 5,
	fclose_fail = 6,
	parse_fail = 7
};

/* General Functions */
void InitProgram(Program *prog, char *filename);
int CountTokens(FILE *fp);
void CheckToken(Program *prog, char token[], char error_text[]);
void FreeMemory(Program *prog);
void ParseError(char *phrase, Program *prog); 
/* Parsing/Formal Grammar Functions */
void ProgramMAIN(Program *prog);
void ProgramINSTRCTLST(Program *prog);
void ProgramINSTRUCTION(Program *prog);
void ProgramFD(Program *prog);
void ProgramLT(Program *prog);
void ProgramRT(Program *prog);
void ProgramDO(Program *prog);
void ProgramSET(Program *prog);
void ProgramPOLISH(Program *prog);
int ProgramOP(Program *prog);
int ProgramVARNUM(Program *prog);
int ProgramVAR(Program *prog);
int ProgramNUM(Program *prog);
