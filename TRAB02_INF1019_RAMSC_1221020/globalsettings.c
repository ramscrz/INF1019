/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo globalsettings.c
 * PUC-Rio
 */

#include<stdio.h>
#include"globalsettings.h"
#include<string.h>
#include<stdlib.h>
#include<time.h>

int printCurrentTime(int enable) {

	time_t timed;
	struct tm * timeData;
	
	time(&timed);
	timeData = localtime(&timed);

	if(enable > 0) {
		printf("\n\tCurrent Time: %s\n", asctime(timeData));
	}

	return timeData->tm_sec;

}


