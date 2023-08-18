#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char keyword_list[11][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline", "test"};
char recognized_char_list[11][1] = {'.', ',', '-', '_', '[', ']', '{', '}', '"', '/', '*'};

// lexical analyzer(la)
char *la_out[1000][5000];
int la_out_index = 0;

// la functions
int isIdentifier(char s[]);
int isKeyword(char s[]);
int isCharRecognized(char ch);

// parser
char var_list[100][50];
int var_value_list[100];
int var_index = 0;

int open_block = 0;	
int close_block = 0;

// parser functions
int isVarDeclared(char s[]);
int getVarValue(char s[]);
void setVarValue(char s[], int value);

// for loop handling
int loop_start_index_list[50];
int loop_finish_index_list[50];
int loop_complete_list[50];

int loop_num = 0;
int in_loop = 0; // is it on loop or not flag
int parse_i = 0; // parse's index
int process = 1; // to not write to output situations like for -3 times out 3.
int no_process_loop_num = -3; // no process's index for making process 1 again

// output
char output[1000][5000];
int output_index = 0;
char output_line[5000];

// main functions
void LexicalAnalyze(FILE* inputFile);
void Parse(char *input[1000][5000]);

int main(int argc, char* argv[])
{
	FILE* inputFile;
	
	char inputFileName[30];
	strcpy(inputFileName, "");
	strcat(inputFileName, argv[1]);
	strcat(inputFileName, ".ba");
	
	inputFile = fopen(inputFileName, "r");
	
	if (!inputFile)
	{
		printf("[Error] FileNotFound");
		exit(1);
	}

	LexicalAnalyze(inputFile);
	Parse(la_out);

	int i;
	for (i = 0; i < output_index + 1; i++)
	{
		if (i != output_index)
			printf("%s\n", output[i]);
		else
			printf("%s", output[i]);
	}	

	return 0;	
}

void Parse(char *la_out[1000][5000])
{
	char type[20];
	char value[1000];
	char line_num_s[1000];
	
	char *addorsub = calloc(3, sizeof(char));
	int a;
	int b;
	
	int state = 0;
	
	int i; // for la_out's line num
	if (in_loop)
		i = loop_start_index_list[loop_num-1];
	else
		i = parse_i; 
	
    while(1)
	{		
		switch(state)
		{
			case 0:
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (in_loop && i == loop_finish_index_list[loop_num-1] + 1) // if loop ends return
					return;
				
				if (in_loop && !loop_num) // rearranging in_loop flag
					in_loop = 0;
					
				if (i == loop_finish_index_list[no_process_loop_num] + 1 && !process) // to quit process
				{
					process = 1;
					no_process_loop_num = -3;
				}

				// for exiting la_out
				if (strcmp(type, "LA_END") == 0)
				{
					if (open_block != close_block)
					{
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
						printf("[Error] blocks do not match at line %s", line_num_s);
						exit(1);
					}
					
					return;
				}
					
					
				// error handling
			    if (strcmp(type, "Keyword") != 0 && strcmp(type, "EndOfLine") != 0 && strcmp(type, "OpenBlock") != 0 && strcmp(type, "CloseBlock") != 0)
				{	
					if (strcmp(type, "Seperator") == 0 || strcmp(type, "Slash") == 0 || strcmp(type, "Asteriks") == 0)
						printf("[Error] expected expression before '%s' at line %s", value, line_num_s);
						
					if (strcmp(type, "MinusSign") == 0 || strcmp(type, "IntConstant") == 0)
						printf("[Error] expected expression before '%s' at line %s", value, line_num_s); // belki daha düzenlenebilir!!!!!!
						
					if (strcmp(type, "UnderScore") == 0)
						printf("[Error] variables cannot start with '%s' at line %s", value, line_num_s);
						
					if (strcmp(type, "Identifier") == 0)
					{
						if (!isVarDeclared(value))
							printf("[Error] '%s' undeclared at line %s", value, line_num_s);
						else
							printf("[Error] expected expression before '%s' at line %s", value, line_num_s);
					}
					
			    	exit(1);
				}
				
			    if (strcmp(value, "int") == 0)
				{	
			    	state = 1;
					break;	
				}
			    if (strcmp(value, "move") == 0)
				{	
			    	state = 2;
					break;	
				}
				if (strcmp(value, "add") == 0 || strcmp(value, "sub") == 0)
				{	
			    	state = 3;
					break;	
				}
				if (strcmp(value, "loop") == 0)
				{					
			    	state = 4;
					break;	
				}
				if (strcmp(value, "out") == 0)
				{	
			    	state = 5;
					break;	
				}
				if (strcmp(value, "test") == 0)
				{
					state = 6;
					break;
				}
				if (strcmp(type, "OpenBlock") == 0 && !in_loop)
				{
					open_block++;
					
					state = 0;
					break;
				}
				if (strcmp(type, "CloseBlock") == 0 && !in_loop)
				{
					close_block++;
					
					if (close_block > open_block)
					{
						printf("[Error] unexpected '%s' at line %s", value, line_num_s);
						exit(1);
					}
					
					state = 0;
					break;
				}
				if (strcmp(value, "to") == 0 || strcmp(value, "from") == 0 || strcmp(value, "times") == 0 || strcmp(value, "newline") == 0)
				{
					printf("[Error] expected expression before '%s' at line %s", value, line_num_s);
					exit(1);
				}
				
				state = 0;
			    break;
			
			case 1: // int (var. declaration)
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "Identifier") != 0)
				{
					printf("[Error] variable expected after 'int' at line %s", line_num_s);
					exit(1);
				}
					
				if (isVarDeclared(value))
				{
					printf("[Error] '%s' at line %s is already declared", value, line_num_s);
					exit(1);	
				}
				
				if (process)			
				{
					int z;
					for(z = 0; value[z]; z++)
						value[z] = tolower(value[z]);
									
					strcpy(var_list[var_index++], value);
					setVarValue(value, 0); // if variable gets created, it gets 0 value
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "EndOfLine") != 0)
				{
					if (strcmp(type, "LA_END") == 0)
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
							
					printf("[Error] '.' expected at line %s", line_num_s);
					exit(1);
				}
				
				state = 0;
				break;
			    
			case 2: // move (assignment)
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
								
				if (strcmp(type, "Identifier") != 0 && strcmp(type, "IntConstant") != 0)
				{
					printf("[Error] variable or int constant expected after 'move' at line %s", line_num_s);
					exit(1);
				}
				if (strcmp(type, "Identifier") == 0)
				{
					if (!isVarDeclared(value))
					{
						printf("[Error] '%s' undeclared at line %s", value, line_num_s);
						exit(1);
					}
					
					a = getVarValue(value);
				}
				else if (strcmp(type, "IntConstant") == 0)
				{
					a = atoi(value);
				}
					
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				// to avoid real numbers and decimal points
				if (strcmp(type, "EndOfLine") == 0)
				{
					sscanf(*la_out[i],"%s %s %s", &type, &value, &line_num_s);
					
					if (strcmp(type, "IntConstant") == 0 && atoi(value) != 0)
						printf("[Error] real number is not valid at line %s", line_num_s);
					else
						printf("[Error] decimal point should not be displayed at line %s", line_num_s);
					
					exit(1);
				}
				
				// grammar check
				if (strcmp(value, "to") != 0)
				{
					printf("[Error] '%s' unexpected after 'move' at line %s", value, line_num_s);	
					exit(1);
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "Identifier") != 0)
				{
					printf("[Error] variable expected after 'to' at line %s", line_num_s);
					exit(1);	
				}		
				if (!isVarDeclared(value))
				{
					printf("[Error] '%s' undeclared at line %s", value, line_num_s);
					exit(1);
				}
				
				if (process)
					setVarValue(value, a);
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "EndOfLine") != 0)
				{
					if (strcmp(type, "LA_END") == 0)
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
							
					printf("[Error] '.' expected at line %s", line_num_s);
					exit(1);
				}
				
				state = 0;
				break;
			
			case 3: // add or sub
				addorsub = calloc(3, sizeof(char));
				strcat(addorsub, value);

				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				a = 0; // for variable or intconstant on the left side
				b = 0; // for variable on the right side
				
				if (strcmp(type, "Identifier") != 0 && strcmp(type, "IntConstant") != 0)
				{
					printf("[Error] variable or int constant expected after '%s' at line %s", addorsub, line_num_s); 
					exit(1);
				}
				if (strcmp(type, "Identifier") == 0)
				{
					if (!isVarDeclared(value))
					{
						printf("[Error] '%s' undeclared at line %s", value, line_num_s);
						exit(1);
					}
					
					a = getVarValue(value);
				}
				else if (strcmp(type, "IntConstant") == 0)
				{
					a = atoi(value);
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				// to avoid real numbers and decimal points
				if (strcmp(type, "EndOfLine") == 0) 
				{
					sscanf(*la_out[i],"%s %s %s", &type, &value, &line_num_s);
					
					if (strcmp(type, "IntConstant") == 0 && atoi(value) != 0)
						printf("[Error] real numbers are not valid at line %s", line_num_s);
					else
						printf("[Error] decimal points should not be displayed at line %s", line_num_s);
						
					exit(1);
				}
				
				// grammar check
				if (strcmp(addorsub, "add") == 0 && strcmp(value, "to") != 0)		
				{
					printf("[Error] 'to' expected after '%s' at line %s", addorsub, line_num_s);
					exit(1);
				}
				if (strcmp(addorsub, "sub") == 0 && !strcmp(value, "from") == 0)		
				{
					printf("[Error] 'from' expected after '%s' at line %s", addorsub, line_num_s);
					exit(1);
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "Identifier") != 0)
				{
					printf("[Error] variable expected on the rightmost side at line %s", line_num_s);
					exit(1);	
				}	
				if (!isVarDeclared(value))
				{
					printf("[Error] '%s' undeclared at line %s", value, line_num_s);
					exit(1);
				}
				
				b = getVarValue(value);
				
				if (strcmp(addorsub, "add") == 0 && process)
					setVarValue(value, b + a);
					
				if (strcmp(addorsub, "sub") == 0 && process)
					setVarValue(value, b - a);
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "EndOfLine") != 0)
				{
					if (strcmp(type, "LA_END") == 0)
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
							
					printf("[Error] '.' expected at line %s", line_num_s);
					exit(1);
				}
				
				state = 0;
				break;
				
			case 4: // loop, for (i = 5; i > 0; i--)
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				int loop_index = 0;
				int var_loop = 0;
				char index_name[20];
				
				if (strcmp(type, "Identifier") != 0 && strcmp(type, "IntConstant") != 0)
				{
					printf("[Error] variable or int constant expected after 'loop' at line %s", line_num_s);
					exit(1);
				}

				if (strcmp(type, "IntConstant") == 0)
				{
					loop_index = atoi(value);
					var_loop = 0;
				}
				else // identifier
				{
					loop_index = getVarValue(value);
					strcpy(index_name, value);
					var_loop = 1;
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(value, "times") != 0)
				{
					printf("[Error] 'loop int_value times' format expected at line %s", line_num_s);
					exit(1);
				}
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				char *expected_end = calloc(10, sizeof(char));
				
				int loop_open_block = 0;
				int loop_close_block = 0;
				
				if (strcmp(type, "OpenBlock") == 0)
				{
					expected_end = "CloseBlock";
					loop_open_block++;
				}	
				else
				{
					expected_end = "EndOfLine";
					i--;
				}
				
				loop_start_index_list[loop_num] = i;
					
				while (1)
				{
					sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
					
					if (strcmp(type, "OpenBlock") == 0)
						loop_open_block++;
						
					if (strcmp(type, "CloseBlock") == 0)
						loop_close_block++;
					
					if (strcmp(type, "LA_END") == 0)
					{
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
						printf("[Error] ']' is missing at line %s", line_num_s);
						exit(1);
					}
					
					if (loop_close_block > loop_open_block)
					{
						printf("[Error] '[' is missing at line %s", line_num_s);
						exit(1);
					}
					
					if (strcmp(type, expected_end) == 0 && loop_open_block == loop_close_block)
					{
						in_loop = 1;
						loop_finish_index_list[loop_num] = i;
						break;
					}
				}
				
				if (loop_index < 1) // if loop_index is negative or zero, there will be no writing to console so process is 0
				{
					loop_index = 1;
					if (process)
						no_process_loop_num = loop_num; // to know which loop will not process the data
						
					process = 0;
				}
				
				loop_num++;	
				
				int la;
				for (la = 0; la < loop_index; la++)
				{
					if (var_loop)
						setVarValue(index_name, loop_index - la);
						
					Parse(la_out);
				}
				
				if (var_loop)
					setVarValue(index_name, 0);
				
				loop_complete_list[loop_num-1] = 1;
				
				i = loop_finish_index_list[loop_num-1];
				parse_i = i;
				
				if (loop_complete_list[loop_num-2])
				{
					in_loop = 0;
					return;
				}
				
				loop_complete_list[loop_num-1] = 0;
				loop_num--;
					
				state = 0;
				break;	
			
			case 5: // out
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				char *value_s = calloc(105, sizeof(char)); // for converting int to string
				
				int list_element_exists;
				int endofline_after_out = 0; // for "out."s invalidity
				
				if (strcmp(type, "EndOfLine") == 0)
					endofline_after_out = 1;
				
				while (strcmp(type, "EndOfLine") != 0) 
				{
					list_element_exists = 0;
					
					if (strcmp(type, "Identifier") == 0)
					{
						if (!isVarDeclared(value))
						{
							printf("[Error] '%s' undeclared at line %s", value, line_num_s);
							exit(1);	
						}
						
						list_element_exists = 1;
						
						if (process)
						{
							itoa(getVarValue(value), value_s, 10);
							strcat(output_line, value_s);
						}	
					}
					else if (strcmp(type, "IntConstant") == 0)
					{
						list_element_exists = 1;
						if (process)
							strcat(output_line, value);
					}
					else if (strcmp(type, "StringConstant") == 0)
					{
						list_element_exists = 1;
						
						if (process)
						{
							memmove(value, value + 1, strlen(value));
							
							int xyz;
							for(xyz = 0; xyz < strlen(value); xyz++)
								if (value[xyz] == '"') // i kept string constants with quotation marks thats why i delete them here
									value[xyz] = ' ';
									
							strcat(output_line, value);	
						}
						
					}
					else if (strcmp(value, "newline") == 0)
					{	
						list_element_exists = 1;
						
						if (process)
						{
							strcpy(output[output_index++], output_line);
							strcpy(output_line, "");
							strcpy(output[output_index], output_line);
						}
					}
					
					if (!list_element_exists)
					{
						if (strcmp(type, "MinusSign") == 0)
						{
							sscanf(*la_out[i],"%s %s %s", &type, &value, &line_num_s);
							
							if (strcmp(type, "MinusSign") == 0 || (strcmp(type, "IntConstant") == 0 && atoi(value) < 0))
							{
								printf("[Error] only one minus sign allowed at line %s", line_num_s);
								exit(1);
							}
							
							sscanf(*la_out[--i],"%s %s %s", &type, &value, &line_num_s);
						}
						printf("[Error] non-list element '%s' exists at line %s", value, line_num_s);
						exit(1);
					}
					
					sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
											
					if (strcmp(type, "Seperator") != 0 && strcmp(type, "EndOfLine") != 0)
					{
						if (strcmp(type, "LA_END") == 0)
							sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
						
						printf("[Error] '.' expected at line %s", line_num_s);
						exit(1);
					}
					
					if (strcmp(type, "EndOfLine") == 0)
					{
						if (process)
							strcpy(output[output_index], output_line);
					}
					else // seperator
					{
						sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
						
						if (strcmp(type, "EndOfLine") == 0)
						{
							printf("[Error] list element expected before '%s' at line %s", value, line_num_s);
							exit(1);	
						}
					}
				}	
				
				if (endofline_after_out) // "out." is not valid
				{
					printf("[Error] list element expected before '%s' at line %s", value, line_num_s);
					exit(1);	
				}
				
				state = 0;
				break;
				
			case 6: // test
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				char test_var_name[20];
				int test_var_value;
				char test_var_value_s[100];
								
				if (strcmp(type, "Identifier") != 0)
				{
					printf("[Error] variable expected after 'test' at line %s", line_num_s);
					exit(1);
				}
				if (!isVarDeclared(value))
				{
					printf("[Error] '%s' undeclared at line %s", value, line_num_s);
					exit(1);
				}
				
				strcpy(test_var_name, value);
				test_var_value = getVarValue(value);
				
				sscanf(*la_out[i++],"%s %s %s", &type, &value, &line_num_s);
				
				if (strcmp(type, "EndOfLine") != 0)
				{
					if (strcmp(type, "LA_END") == 0)
						sscanf(*la_out[i-2],"%s %s %s", &type, &value, &line_num_s);
							
					printf("[Error] '.' expected after variable at line %s", line_num_s);
					exit(1);
				}
				
				if (process)
				{
					strcpy(output[output_index++], output_line);
				
					itoa(test_var_value, test_var_value_s, 10);
					strcpy(output_line, "  ");
					strcat(output_line, test_var_name);
					strcat(output_line, ":");
					strcat(output_line, test_var_value_s);
					
					strcpy(output[output_index++], output_line);
					strcpy(output_line, "");
				}
				
				state = 0;
				break;
		}
    }
}

int isVarDeclared(char s[])
{
	char c[20];
	strcpy(c, s);
	
	int j;
	for (j = 0; c[j]; j++)
		c[j] = tolower(c[j]);
	
	int i;
	for (i = 0; i < 100; i++)
		if(strcmp(c, var_list[i]) == 0)
			return 1;
	
	return 0;	
}

int getVarValue(char s[])
{
	char c[20];
	strcpy(c, s);
	
	int j;
	for (j = 0; c[j]; j++)
		c[j] = tolower(c[j]);
		
	int i;
	for (i = 0; i < 100; i++)
		if (strcmp(c, var_list[i]) == 0)
			break;
	
	return var_value_list[i];
}

void setVarValue(char s[], int value)
{
	char c[20];
	strcpy(c, s);
	
	int j;
	for (j = 0; c[j]; j++)
		c[j] = tolower(c[j]);
		
	int i;
	for (i = 0; i < 100; i++)
		if (strcmp(c, var_list[i]) == 0)
			break;
			
	var_value_list[i] = value;
}

void LexicalAnalyze(FILE* inputFile)
{	
	// added for project 3
	char *caseout = calloc(10, sizeof(char)); // for line extraction to la_out
	int line_num = 1;
	char *line_num_s = calloc(1005, sizeof(char));
	line_num_s = "1";
	
	char *word = calloc(5, sizeof(char));
	int state = 0; // initial state
	int i = 0; // for storing word's lenght
	char ch;
	
	// flags
	int commentIsClosed = 1;
	int curlyComment = 0;
	int multiLineComment = 1;
	int stringConstantIsClosed = 1;
		
	while(!feof(inputFile))
	{
		switch (state)
		{
			case 0: // determining the next state or failure
				ch = fgetc(inputFile);	
				i = 0; // resetting word index
				
				if (ch == '\n')
				{
					line_num++;
					line_num_s = calloc(1005, sizeof(char));
					itoa(line_num, line_num_s, 10);
				}

				caseout = calloc(20, sizeof(char));

				if (isspace(ch) || ch == EOF)
				{
					state = 0;
					break;	
				}
				if (ch == '.')
				{
					strcat(caseout, "EndOfLine . ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					state = 0;
					break;
				}
				if (ch == ',')
				{
					strcat(caseout, "Seperator , ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					state = 0;
					break;
				}
				if (ch == '_')
				{
					strcat(caseout, "UnderScore _ ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					state = 0;
					break;	
				}
				if (ch == '[')
				{
					strcat(caseout, "OpenBlock [ ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					state = 0;
					break;	
				}
				if (ch == ']')
				{
					strcat(caseout, "CloseBlock ] ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					state = 0;
					break;	
				}
				if (!isCharRecognized(ch))
				{
					printf("[Error] UnrecognizedCharacter %c at line %d.", ch, line_num); 
					exit(1);
				}
				if (isalpha(ch))
				{
					word = calloc(25, sizeof(char));
					caseout = calloc(60, sizeof(char));
					
					word[i++] = ch;
					state = 1;	
					break;
				}
				if (isdigit(ch) || ch == '-')
				{
					word = calloc(105, sizeof(char));
					caseout = calloc(150, sizeof(char));
					
					word[i++] = ch;
					state = 2;
					break;
				}
				if (ch == '"')
				{
					word = calloc(1005, sizeof(char));
					caseout = calloc(1050, sizeof(char));
					
					word[i++] = ch;
					stringConstantIsClosed = 0;
					state = 3;
					break;
				}
				if (ch == '{')
				{
					curlyComment = 1;
					commentIsClosed = 0;
					state = 4;
					break;	
				}
				if (ch == '/')
				{
					ch = fgetc(inputFile);
					if (ch == '*' || ch == '/')
					{
						curlyComment = 0;
						commentIsClosed = 0;
						
						if (ch == '*')
							multiLineComment = 1;
						else
							multiLineComment = 0;
							
						state = 4;	
						break;
					}
					
					strcat(caseout, "Slash / ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					
					fseek(inputFile, -1, SEEK_CUR);
					state = 0;
					break;
				}
				if (ch == '}')
				{
					printf("[Error] LeftCurlyBracketMissing at line %d", line_num);
					exit(1);
				}
				if (ch == '*')
				{
					ch = fgetc(inputFile);
					if (ch == '/')
					{
						printf("[Error] '/*' is missing at line %d", line_num); // düzenlenir
						exit(1);
					}
					
					strcat(caseout, "Asteriks * ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					
					fseek(inputFile, -1, SEEK_CUR);
					state = 0;
					break;
				}
				
				state = 0;
				break;	
				
			case 1: // reading identifier or keyword
				ch = fgetc(inputFile);

				if (!isalnum(ch) && ch != '_')
				{
					word[i] = '\0';
					
					if (isIdentifier(word))
						strcat(caseout, "Identifier ");
					else if (isKeyword(word))
						strcat(caseout, "Keyword ");
					
					strcat(caseout, word);
					strcat(caseout, " ");
					strcat(caseout, line_num_s);
					*la_out[la_out_index++] = caseout;
					
					// if ch is end of file and we fseek 1 back, this would create an infinite loop
					if (ch != EOF)
						fseek(inputFile, -1, SEEK_CUR);

					state = 0;
					break;	
				}				
				
				word[i++] = ch;
				
				if (strlen(word) == 21)
				{
					printf("[Error] VariableNameTooLong at line %d", line_num);
					exit(1);
				}
				
				state = 1;
				break;
				
			case 2: // reading integer constant
				ch = fgetc(inputFile);
				
				if (word[0] == '-' && ch == '0') // -0 or -023 is not valid int. constant
				{
					printf("[Error] IntConstantCannotStartWithZero at line %d", line_num);
					exit(1);
				}
				
				if (word[0] == '0' && isdigit(ch)) // 053 is not valid int. constant
				{
					printf("[Error] IntConstantCannotStartWithZero at line %d", line_num);
					exit(1);
				}

				if (!isdigit(ch))
				{
					word[i] = '\0';
					
					if (strlen(word) == 1 && word[0] == '-') // if word == -
					{
						strcat(caseout, "MinusSign - ");
						strcat(caseout, line_num_s);
						
						*la_out[la_out_index++] = caseout;
					}
					else
					{
						strcat(caseout, "IntConstant ");
						strcat(caseout, word);
						strcat(caseout, " ");
						strcat(caseout, line_num_s);
						
						*la_out[la_out_index++] = caseout;
					}
					
					// if ch is end of file and we fseek 1 back, this would create an infinite loop
					if (ch != EOF)
						fseek(inputFile, -1, SEEK_CUR);
						
					state = 0;
					break;
				}
				
				word[i++] = ch;
				
				if ((word[0] != '-' && strlen(word) == 101) || (word[0] == '-' && strlen(word) == 102)) 
				{
					printf("[Error] IntConstantTooBig at line %d", line_num);
					exit(1);
				}
				
				state = 2;
				break;
			
			case 3: // reading string constant
				ch = fgetc(inputFile);
 				
				if (ch == '"')
				{
					word[i] = '\0';

					strcat(caseout, "StringConstant ");
					strcat(caseout, word);
					strcat(caseout, " ");
					strcat(caseout, line_num_s);
					
					*la_out[la_out_index++] = caseout;
					
					stringConstantIsClosed = 1;
					state = 0;
					break;
				}
				
				if (isspace(ch))
					ch = '"';
				
				word[i++] = ch;
				
				if (strlen(word) == 1002)
				{
					printf("[Error] StringConstantTooLong at line %d", line_num);
					exit(1);
				}
				
				state = 3;
				break;
				
			case 4: // comment
				ch = fgetc(inputFile);
				
				// to keep track of line_numbers
				if (ch == '\n')
				{
					line_num++;
					line_num_s = calloc(1005, sizeof(char));
					itoa(line_num, line_num_s, 10);
				}
					
				// ignoring everything unless ch is }
				if (ch == '}' && curlyComment)
				{
					commentIsClosed = 1;
					state = 0;
					break;
				}
				
				// ignoring everything unless sees newline
				if (ch == '\n' && !curlyComment && !multiLineComment)
				{
					commentIsClosed = 1;
					multiLineComment = 1;
					state = 0;
					break;
				}
				
			    // ignoring everything unless sees */
				if (ch == '*' && !curlyComment && multiLineComment)
				{
					ch = fgetc(inputFile);
					if (ch == '/')
					{
						commentIsClosed = 1;
						state = 0;
						break;
					}
					
					fseek(inputFile, -1, SEEK_CUR);
					state = 4;
					break;
				}
				
				state = 4;
				break;		
		}
	}
	
	fclose(inputFile);
	
	if (!commentIsClosed)
	{
		printf("[Error] CommentLeftOpen at line %s", line_num_s);
		exit(1);
	}
	
	if (!stringConstantIsClosed)
	{
		printf("[Error] StringConstantLeftOpen at line %s", line_num_s);
		exit(1);
	}
	
	*la_out[la_out_index] = "LA_END LA_END LA_END";

}

int isIdentifier(char s[])
{
	char c[20];
	strcpy(c, s);
	
	int j;
	for(j = 0; c[j]; j++)
		c[j] = tolower(c[j]);
	
	int i;
	for (i = 0; i < 11; i++)
		if (strcmp(c, keyword_list[i]) == 0)
			return 0;
	
	return 1;
}	

int isKeyword(char s[])
{
	// max. keyword is newline and its length is 7
	// so if s's length is greater then 7 it cannot be a keyword
	// there is no need to search it in the list
	if (strlen(s) > 7)
		return 0;
		
	int j;
	for(j = 0; s[j]; j++)
		s[j] = tolower(s[j]);
		
	int i;
	for (i = 0; i < 11; i++)
		if (strcmp(s, keyword_list[i]) == 0)
			return 1;
	
	return 0;
}

int isCharRecognized(char ch)
{
	// if ch is alphanumerical then it is recognized
	// there is no need to search it in the list
	if (isalnum(ch))
		return 1;
		
	int i;
	for(i = 0; i < 11; i++)
		if (*recognized_char_list[i] == ch)
			return 1;
	
	return 0;
}

