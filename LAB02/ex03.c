#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include <string.h>

char** parseCommand(char * input, char delimiter) {

	int i, k = 0;
	int j = 2;
	int inputSize = strlen(input) + 1;
	char currentpath[1024];
	char** result = (char**) malloc (inputSize * sizeof(char*));
	
	for(i = 0; i < inputSize; i++) {
		result[i] = (char*) malloc (1024 * sizeof(char));	
	}

	i = 0;

	// Loads the filename of the program to be executed.
	strcpy(result[0], "sh");
	strcpy(result[1], "-c"); 

	// Parses the input string of the command.
	while(input[i]) {

		// In case ' ' is found, then every character before different than whitespace 
		// belongs to the same word which is one argument of the command to be executed.
		if(input[i] == ' ') {
			
			// If the character that follows is not ("), then this parameter is done. 
			// Otherwise, this parameter must include the following characters until another (") 
			// is found; j is the index of the parameter of the parsed command.
			if(input[i+1] != '\"') {
				j++;
				k = 0;
			} else {

				result[j][k] = ' ';
				k++;
			}
		// If input[i] is ", then every character after input[i] until the next " must belong 
		// to the same parameter.
		} else if(input[i] == '\"') {
			
			i++;
			while(input[i] && input[i] != '\"') {
				result[j][k] = input[i];
				i++;
				k++;
			}

			if(input[i+2] != '\"') {
				j++;
				k = 0;
			} else {
				result[j][k] = ' ';
				k++;
				i++;
			}

		} else {
			// Saves the character input[i] as part of the parameter result[j];
			// i allows iteration over the characters of input.
			// j allows iteration over the parameters of the command.
			result[j][k] = input[i];
			k++;
		}

		i++;
	}

	// The last character of the array of chars used to indicate the parameters to execvp function 
	// must be NULL.
	result[j + 1] = NULL;

	return result;

}

int main() {

	char input[1024];
	char ** command;
	char currentpath[1024];
	char * exitC[] = {"sh", "-c", "exit", NULL};
	pid_t pid;

	if(getcwd(currentpath, sizeof(currentpath)) == NULL) {
		strcpy(currentpath, "~ ");
	}

	printf("\nRunning from directory: \n%s \n\nEnter command: ", currentpath);
	fgets(input, 1024, stdin);
	printf("\n\n");

	while(strcmp(input, "exit\n") != 0) {

		pid = fork();

		if(pid != 0) {

			waitpid(pid, 0, 0);

		} else {

			command = parseCommand(input, ' ');

			execvp(command[0], command);
			exit(0);
		}

		if(getcwd(currentpath, sizeof(currentpath)) == NULL) {
			strcpy(currentpath, "~ ");
		}

		printf("\nRunning from directory: \n%s \n\nEnter command: ", currentpath);
		fgets(input, 1024, stdin);
	}

	pid = fork();

	if(pid != 0) {

         waitpid(pid, 0, 0);

	} else {
		execvp(exitC[0], exitC);
	}

	return 0;

}
