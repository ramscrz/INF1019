/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo globalsettings.c
 * PUC-Rio
 */

#include<stdio.h>
#include"globalsettings.h"
#include<string.h>
#include<stdlib.h>

void printLog(char* logFileName, char* message, char* argument) {

	FILE* logFile;

	logFile = fopen(logFileName, argument);

	if(!logFile) {
		printf("\n\tERROR: Scheduler Log File could not be accessed.\n");
		exit(1);
	} else {

		fprintf(logFile, message, NULL);

		fclose(logFile);
	}

	return;

}

