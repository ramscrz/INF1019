/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo producerconsumer.c
 * PUC-Rio
 */

#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include"producerconsumer.h"
#include<string.h>
#include"globalsettings.h"

void deallocateCommand(Command *command) {

	if(command == NULL) {
		return;
	}

	free(command);
	
	command = NULL;

}

Command* createCommand() {

	Command* command = (Command*) malloc (sizeof(Command));
	if(command == NULL) {
		return NULL;
	}

	command->nparams = 0;

	return command;
}

void printCommand(Command command) {

	int i = 0;

	if(command.exec != NULL) {
		printf("\n\tExecute Command: %s", command.exec);
	}

	if(command.parameters != NULL) {
		for(i = 0 ; i < command.nparams ; i++) {
			if(command.parameters[i]) {
				printf("\n\t\tParameter [%d] of Execute Command = %d", i, command.parameters[i]);
			}
		}
	}

	return;
}


// Function for the Consumer's activities execution.
int consumer(Resources *data, int producerPid, Command* resource) {

	int index = 0, i = 0;

	if(data == NULL || resource == NULL) {
		return -1;
	}

	if(data->nelem == 0) {
		#ifdef SET_DEBUG
			printf("\nCONSUMER HAS BEEN STOPPED!\n");
		#endif

		kill(getpid(), SIGSTOP);
	}

	index = data->head - data->nelem;

	if(index < 0) {
		index = data->max + index;
	}

	strcpy(resource->exec, data->newPrograms[index].exec);
	resource->nparams = data->newPrograms[index].nparams;

	for(i = 0; i < resource->nparams; i++) {
		resource->parameters[i] = data->newPrograms[index].parameters[i];
		
		#ifdef SET_DEBUG
			printf("\nCONSUMER Index = %d ; Resource Parameter = %d ; Resources Parameter = %d\n", index, resource->parameters[i], data->newPrograms[index].parameters[i]);
		#endif
	}

	data->nelem--;

	if(data->nelem == data->max - 1) {
		#ifdef SET_DEBUG
			printf("\nPRODUCER IS BEING ACTIVATED...\n");
		#endif
		kill(producerPid, SIGCONT);
	}

	return 1;

}

// Function for the Producer's activities execution.
int producer(Resources* data, int consumerPid, char* exec, int* params, unsigned int nparams) {

	int i = 0;

	if(data == NULL) {
		return -1;
	}

	if(data->newPrograms == NULL) {
		return -1;
	}

	#ifdef SET_DEBUG
		printf("\n\nProducer %d %d\n", data->nelem, data->max);
	#endif

	if(data->nelem == data->max) {
		#ifdef SET_DEBUG
			printf("\n\nPRODUCER HAS BEEN STOPPED!\n\n");
		#endif

		kill(getpid(), SIGSTOP);
	}

	for(i = 0; i < nparams; i++) {
		
		#ifdef SET_DEBUG
			if(data == NULL) printf("\nPRODUCER DATA NULL\n");
			if(data->newPrograms == NULL) printf("\nPRODUCER DATA NEWPROGRAMS NULL\n");
			if(params == NULL) printf("\nPRODUCER PARAMS NULL\n");
			if(data->newPrograms[data->head].parameters == NULL) printf("\nPRODUCER DATA HEAD PARAMETERS NULL\n");
		#endif

		data->newPrograms[data->head].parameters[i] = params[i];

	}

	strcpy(data->newPrograms[data->head].exec, exec);

	i = 0;

	data->newPrograms[data->head].nparams = nparams;

	if(data->head == data->max - 1) {
		data->head = 0;
	} else {
		data->head++;
	}

	data->nelem++;

	if(data->nelem == 1) {
		#ifdef SET_DEBUG
			printf("\n\nCONSUMER IS BEING ACTIVATED...\n");
		#endif

		kill(consumerPid, SIGCONT);
	}

	return 1;

}
