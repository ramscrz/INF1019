/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo interpreter.c
 * PUC-Rio
 */

#include<stdio.h>
#include<stdlib.h>
#include"producerconsumer.h"
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<sys/wait.h>
#include"globalsettings.h"

char **params;
Resources* fifoData;

int commandFifoSegment = 0;
int newProgramsSegment = 0;
int execSegment[COMMAND_SIZE];
int paramsSegment[COMMAND_SIZE];
int paramSegment[COMMAND_SIZE];

void deallocateSharedMemory(int);

int parseCommand(char* input, char* result, int* parameters);

int main(int argc, char** argv) {

	char command[COMMAND_SIZE];
	char *result;
	int *parameters;
	int sharedMemId = 0;
	int nparams = 0;

	int i = 0, j = 0, k = 0, t = 0;

	FILE *log, *inputFile = NULL;

	signal(SIGTERM, deallocateSharedMemory);

	pid_t interpreterPid = -1, schedulerPid = -1;

	log = fopen("Interpreter_Log.txt", "w");
	if(log == NULL) {
		printf("\nThe Log File for the Interpreter could not be created.\n");
		exit(1);
	}

	fclose(log);
	log = NULL;

	if(argc != 3 && argc != 4) {
		printf("\n\nThe name of a scheduler program and a shared memory id number must be provided as argument for this command interpreter to work.\n\n");
		printLog("Interpreter_Log.txt", "\n\nThe name of a scheduler program and a shared memory id number must be provided as argument for this command interpreter to work.\n\n", "a");
		exit(1);
	}

	printf("\n\n======== Command Interpreter ========\n\n");
	printLog("Interpreter_Log.txt", "\n\n======== Command Interpreter ========\n\n", "a");
	printf("Initializing...\n");
	printLog("Interpreter_Log.txt", "Initializing...\n", "a");

	sharedMemId = strtol(argv[2], NULL, 10);
	
	if(sharedMemId <= 0) {
		printf("\nThe provided Shared Memory Identifier is not valid. Exiting program...\n");
		printLog("Interpreter_Log.txt", "\nThe provided Shared Memory Identifier is not valid. Exiting program...\n", "a");
		exit(1);
	}

	commandFifoSegment = shmget(sharedMemId, sizeof(Resources), IPC_CREAT | S_IRUSR | S_IWUSR);

	if(commandFifoSegment < 0) {

		printLog("Interpreter_Log.txt", "\nERROR in shmget for Command FIFO.\n", "a");
		printf("\n\nERROR in shmget for Command FIFO: %s\n\n", strerror(errno));

		shmdt(&commandFifoSegment);
		shmctl(commandFifoSegment, IPC_RMID, 0);
		commandFifoSegment = shmget(sharedMemId, sizeof(Resources), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		
		if(commandFifoSegment < 0) {

			shmdt(&commandFifoSegment);
			shmctl(commandFifoSegment, IPC_RMID, 0);

			printLog("Interpreter_Log.txt", "\nERROR: Could not allocate shared memory for Fifo and Programs (shmget failed).\n", "a");

			exit(1);
		}

	}

	fifoData = (Resources*) shmat(commandFifoSegment, 0, 0);

	shmctl(commandFifoSegment, IPC_RMID, 0);

	if(!fifoData) {

		shmdt(fifoData);
		shmdt(&commandFifoSegment);

		printLog("Interpreter_Log.txt", "\nERROR: shmat failed to get shared memory address of the Fifo data.\n", "a");

		exit(1);
	}

	//Criar escalonador e pegar o pid dele.

	interpreterPid = getpid();

	fifoData->max = FIFO_SIZE;
	fifoData->nelem = 0;
	fifoData->head = 0;

	schedulerPid = fork();

	if(schedulerPid > 0) {

		if(argc == 4) {

			inputFile = fopen(argv[argc-1], "r");

			if(inputFile != NULL) {

				if(fgets(command, COMMAND_SIZE - 1, inputFile) == NULL) {
					fclose(inputFile);
					inputFile = NULL;
				}

			}

		}

		if(inputFile == NULL) {

			#ifdef SET_DEBUG
			printf("\n\nWAITING NEW COMMAND PID %d\n\n", schedulerPid);
			#endif

			printLog("Interpreter_Log.txt", "\n=====> WAITING FOR NEW COMMAND...\n", "a");

			printf("\nEnter command: ");
			fgets(command, COMMAND_SIZE, stdin);

			printLog("Interpreter_Log.txt", command, "a");		

			#ifdef SET_DEBUG
			printf("\nEntered command: %s\n", command);
			#endif
		}

		while(strcmp(command, "quit\n") != 0) {

			result = (char*) malloc (PARAMETERS_SIZE * sizeof(char));

			if(result == NULL) {
				printf("\nERROR: Could not allocate memory for RESULT.\n");
				exit(1);
			}

			parameters = (int*) malloc (PARAMETERS_SIZE * sizeof(int));

			if(parameters == NULL) {
				printf("\nERROR: Could not allocate memory for PARAMETERS.\n");
				exit(1);
			}

			nparams = parseCommand(command, result, parameters);

			#ifdef SET_DEBUG
			printf("\n\n Retrieved Command: NPARAMS = %d ; Exec = %s\n\n", nparams, result);
			#endif

			if(nparams > 0) {

				#ifdef SET_DEBUG
					printf("\nParsed data:\n\tCommand: %s\n\tNumber of parameters: %d\n\tProgram: %s\n", command, nparams, result);
				#endif

				#ifdef SET_DEBUG
					for(i = 0; i < nparams; i++) {
						printf("\tParameter[%d] = %d\n", i, parameters[i]);
					}

					if(fifoData->newPrograms[fifoData->head].parameters != NULL) printf("\nParameters of most recent command in FIFO are valid!\n");
					else printf("\nParameters of most recent command in FIFO is NULL\n");
				#endif

				if(producer(fifoData, schedulerPid, result, parameters, nparams) >= 0) {
					
					free(result);
					result = NULL;

					free(parameters);
					parameters = NULL;

				} else {

					printLog("Interpreter_Log.txt", "\nERROR: Producer could not produce a new program execution from the provided command!\n", "a");
					printf("\nERROR: Producer could not produce a new program execution from the provided command: %s\n", command);

				}

			}

			free(result);
			result = NULL;

			free(parameters);
			parameters = NULL;

			printLog("Interpreter_Log.txt", "\n=====> WAITING FOR NEW COMMAND...\n", "a");


			if(inputFile != NULL) {

				if(fgets(command, COMMAND_SIZE - 1, inputFile) == NULL) {
					fclose(inputFile);
					inputFile = NULL;
				}

			} else {
				printf("\nEnter command: ");
				fgets(command, COMMAND_SIZE, stdin);

				printLog("Interpreter_Log.txt", command, "a");

				#ifdef SET_DEBUG
				printf("\nEntered command: %s\n", command);
				#endif
			}
		}

		kill(schedulerPid, SIGKILL);

	} else if(schedulerPid == 0) {

		params = (char**) malloc(4 * sizeof(char*));

		if(params) {

			params[0] = (char*) malloc(50 * sizeof(char));
			params[1] = (char*) malloc(10 * sizeof(char));
			params[2] = (char*) malloc(20 * sizeof(char));

			if(params[0] && params[1] && params[2]) {

				snprintf(params[0], 50, "%s", "scheduler");
				snprintf(params[1], 10, "%d", interpreterPid);
				snprintf(params[2], 20, "%d", commandFifoSegment);

				params[4] = NULL;

				printLog("Interpreter_Log.txt", "\nInitializing Scheduler <PID %d>...", "a");

				execv(argv[1], params);

				printLog("Interpreter_Log.txt", "\nERROR: Scheduler could not be executed! execv failure!\n", "a");
				printf("\nERROR: Scheduler could not be executed! execv failure!\n");

				if(params[0]) {
					free(params[0]);
					params[0] = NULL;
				}

				if(params[1]) {
					free(params[1]);
					params[1] = NULL;
				}

				if(params[2]) {
					free(params[2]);
					params[2] = NULL;
				}

				exit(1);

			} else {
				printf("\nMalloc Error: could not allocate memory for one of the parameters of the scheduler process.\n");
				printLog("Interpreter_Log.txt", "\nMalloc Error: could not allocate memory for one of the parameters of the scheduler process.\n", "a");

				exit(1);
			}
		}

	} else {

		printLog("Interpreter_Log.txt", "\nfork() Error: Child process to execute scheduler could not be created.\n", "a");
		printf("\nfork() Error: Child process to execute scheduler could not be created.\n");
	}

	shmdt(fifoData);
	
	if(log != NULL) {
		fclose(log);
	}
	log = NULL;

	return 0;
}

void deallocateSharedMemory(int x) {

	int k = 0, t = 0;

	printLog("Interpreter_Log.txt", "\nDeallocating Allocated Memory...\n", "a");
	
	if(fifoData) {
		shmdt(fifoData);
	}
	
	if(params[0]) {
		free(params[0]);
		params[0] = NULL;
	}

	if(params[1]) {
		free(params[1]);
		params[1] = NULL;
	}

	if(params[2]) {
		free(params[2]);
		params[2] = NULL;
	}

	exit(0);

}

int parseCommand(char* input, char* result, int* parameters) {

	int i = 0, j = 0, k = 0;
	int inputSize = 0;
	
	char* buffer = NULL;
	
	if(input == NULL) {

		printLog("Interpreter_Log.txt", "\nPARSER ERROR: Input does not exist. Input is NULL.\n", "a");
		printf("PARSER ERROR: Input does not exist. Input is NULL.");
		return -1;
	}

	inputSize = strlen(input) + 2;

	if(parameters == NULL){
		return -1;
	}

	if(result == NULL) {
		return -1;
	}

	buffer = (char*) malloc (inputSize * sizeof(char));
	
	if(buffer == NULL) {

		printLog("Interpreter_Log.txt", "\nAt parseCommand => Malloc Error: buffer could not be allocated to parse a command.\n", "a");
		return -1;
	}

	i = 0;

	// Loads the filename of the program to be executed.
	while(input[i] && input[i] != ' ') {
		buffer[i] = input[i];
		i++;
	}

	buffer[i] = '\n';

	if(strcmp(buffer, "exec\n") != 0) {

		printLog("Interpreter_Log.txt", "\nCommand was not recognized. The command is probably in the wrong format.\n", "a");

		free(buffer);
		buffer = NULL;
		
		return -1;
	}

	j = 0;

	i++;

	while(input[i] && input[i] != ' ') {

		result[j] = input[i];
		
		i++;
		j++;
	}

	i++;

	if(input[i] && input[i] == '(') {

		result[j] = '\0';

		j = 0;
		i++;

		while(input[i] && input[i+1] && input[i+2]) {
		
			if(!(input[i] > 47 && input[i] < 58)) {

				printLog("Interpreter_Log.txt", "\nAt parseCommand => Invalid parameter. Execution command parameters must be integers.\n", "a");
				free(buffer);
				buffer = NULL;

				return -1;
			}

			while(input[i] > 47 && input[i] < 58) {

				buffer[j] = input[i];
				j++;
				i++;
			}

			buffer[j] = '\n';

			if(k >= PARAMETERS_SIZE) {

				printLog("Interpreter_Log.txt", "\nAt parseCommand => Too many parameters were provided.\n", "a");
				free(buffer);
				buffer = NULL;
				return -1;

			}

			parameters[k] = strtol(buffer, NULL, 10);

			k++;
			j = 0;

			if(input[i] == ')') {

				if(input[i+1] && input[i+1] == '\n') {

					printLog("Interpreter_Log.txt", "\nAt parseCommand => Command was successfully parsed: %s\n", "a");
					free(buffer);
					buffer = NULL;

					return k;		

				} else  {

					printLog("Interpreter_Log.txt", "\nAt parseCommand => Invalid command. Commands must end with ) followed by \\n \n", "a");
					free(buffer);
					buffer = NULL;
					return -1;
				}

			} else if(input[i] != ',') {

				printLog("Interpreter_Log.txt", "\nAt parseCommand => Invalid command. Parameters must be separated by virgule (,).\n", "a");
				free(buffer);
				buffer = NULL;
				return -1;
			}

			i++;

			if(input[i] != ' ') {

				printLog("Interpreter_Log.txt", "\nAt parseCommand => Invalid command. A white space must exist between , and a new parameter. \n", "a");
				free(buffer);
				buffer = NULL;
				return -1;
			}

			i++;
		}

	} else if(input[i]) {

		printLog("Interpreter_Log.txt", "\n\nError! Program name must not contain white spaces!\n\n", "a");
		printf("\n\nError! Program name must not contain white spaces!\n\n");
		free(buffer);
		buffer = NULL;
		return -1;
	
	} else {

		printLog("Interpreter_Log.txt", "\nAt parseCommand => Invalid command.\n", "a");
		free(buffer);
		buffer = NULL;
		return -1;
	}

	printLog("Interpreter_Log.txt", "\nAt parseCommand => Endless command. Commands must end with ) followed by \\n \n", "a");
	free(buffer);
	buffer = NULL;
	return -1;

}

