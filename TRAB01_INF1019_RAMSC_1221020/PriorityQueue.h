/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo PriorityQueue.h
 * PUC-Rio
 */

#ifndef PRIORITY_QUEUE
#define PRIORITY_QUEUE

#include"producerconsumer.h"
#include<stdio.h>

typedef struct process Process;
typedef struct priorityQueue PriorityQueue;

PriorityQueue* createPriorityQueue(int nquantum); 

PriorityQueue* insertProcessIntoQueue(PriorityQueue *queue, Command command, int pid, int currentQueueId);

PriorityQueue* reinsertIntoQueue(PriorityQueue *queue, Command command, int pid, int currentParam, int currentQueueId);

PriorityQueue* retrieveProcessFromQueue(PriorityQueue *queue, int *currentParamIndex, Command* command, int *pid, int *currentQueueId, int* timeForIO);

int searchAndRemoveProcess(PriorityQueue * queue, int pid);

void printPriorityQueue(PriorityQueue * queue);

void printPriorityQueueBackwards(PriorityQueue * queue);

void fprintPriorityQueueBackwards(char* fileName, PriorityQueue * queue, char* argument);

void fprintPriorityQueue(char* fileName, PriorityQueue * queue, char* argument);

int getQueueSize(PriorityQueue *queue);

int getQueueQuantum(PriorityQueue *queue);

void deallocateQueue(PriorityQueue *pQueue);

#endif
