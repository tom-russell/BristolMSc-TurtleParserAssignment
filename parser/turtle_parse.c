#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

#define MAXTOKENLEN 10

int main(int argc, char **argv)
{
	Program prog;

	/* Exit if not passed an input text file */
	if (argc != 2) {
		fprintf(stderr, "1 command line input file is required.\n");
		exit(no_input_file);
	}

    /* Initialise the program struct, then run the parser */
    InitProgram(&prog, argv[1]);
    ProgramMAIN(&prog);
    
    /* Free allocated memory before program end */
    FreeMemory(&prog);
    
    return 0;
}

void InitProgram(Program *prog, char *filename)
{
	int i;
	FILE *fp;

	/* Set the 'current token' variable to the first token (0) */
	prog->curr = 0;

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
    for(i = 0; i < prog->total_tokens + 1; i++) {
        free(prog->tokens[i]);
    }
    /* Free the token pointer array */
    free(prog->tokens);
}

void ParseError(char *phrase, Program *prog) 
{
    /* Print the error statement, free allocated memory then exit the program */
    fprintf(stderr, "%s\nParse error occurs on token %d.\n", phrase, prog->curr+1);
    FreeMemory(prog);
    exit(parse_fail);
}