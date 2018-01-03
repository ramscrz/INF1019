/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo producerconsumer.h
 * PUC-Rio
 */

#ifndef PRODUCERCONSUMER_H
#define PRODUCERCONSUMER_H

#include"globalsettings.h"

struct command {

   char exec[PARAMETERS_SIZE];
   int  parameters[PARAMETERS_SIZE];
   int  nparams;
};

typedef struct command Command;

// Struct representing a FIFO.
struct resources {

   Command newPrograms[FIFO_SIZE];	// Buffer storing FIFO's elements
   int max;                			// Maximum number of stored elements
   int nelem;              			// Current number of stored elements
   int head;               			// Head of the FIFO.

};

typedef struct resources Resources;

Command* createCommand();

void printCommand(Command command);

// Function for the Consumer's activities execution.
int consumer(Resources *data, int producerPid, Command* resource);

// Function for the Producer's activities execution.
int producer(Resources *resource, int consumerPid, char *exec, int *params, unsigned int nparams);

void deallocateCommand(Command *command);

#endif
