/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo PriorityQueue.c
 * PUC-Rio
 */

#include"PriorityQueue.h"
#include<stdlib.h>
#include<string.h>
#include"globalsettings.h"
#include<stdio.h>

struct process {

	Command *command;
	struct process *next;
	struct process *previous;
	int currentParam;
	int pid;
	int currentQueueId;

};

struct priorityQueue {

	int quantum;
	int nprocesses;
	Process* head;
	Process* tail;

};

PriorityQueue* createPriorityQueue(int nquantum) {

	PriorityQueue* pQueue = (PriorityQueue*) malloc (sizeof(PriorityQueue));

	if(pQueue == NULL) {
		return NULL;
	}

	pQueue->quantum = nquantum;
	pQueue->nprocesses = 0;
	pQueue->head = NULL;
	pQueue->tail = NULL;

	return pQueue;
}

int getQueueSize(PriorityQueue *queue) {

	if(queue == NULL) {
		return -1;
	}

	return queue->nprocesses;

}

int getQueueQuantum(PriorityQueue *queue) {

	if(queue == NULL) {
		return -1;
	}

	return queue->quantum;

}

PriorityQueue* reinsertIntoQueue(PriorityQueue *queue, Command command, int pid, int currentParam, int currentQueueId) {

	int i = 0;

	if(queue == NULL) {
		return NULL;
	}

	if(queue->head == NULL && queue->tail == NULL) {

		queue->head = (Process*) malloc (sizeof(Process));

		if(queue->head == NULL) {
			return NULL;
		}

		queue->head->command = createCommand();

		if(queue->head->command == NULL) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;

			return NULL;
		}

		queue->head->command->nparams = command.nparams;

		if(command.nparams > PARAMETERS_SIZE) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		for(i = 0; i < queue->head->command->nparams; i++) {
			queue->head->command->parameters[i] = command.parameters[i];
		}

		if(strcpy(queue->head->command->exec, command.exec) == NULL) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		queue->head->pid = pid;
		queue->head->currentParam = currentParam;
		queue->head->currentQueueId = currentQueueId;

		queue->head->next = NULL;
		queue->head->previous = NULL;

		queue->tail = queue->head;

		queue->nprocesses = 1;

	} else if(queue->head != NULL && queue->tail != NULL) {

		queue->head->previous = (Process*) malloc (sizeof(Process));

		if(queue->head->previous == NULL) {
			return NULL;
		}

		queue->head->previous->command = createCommand();

		if(queue->head->previous->command == NULL) {
			free(queue->head->previous);
			queue->head->previous = NULL;

			return NULL;
		}

		queue->head->command->nparams = command.nparams;

		if(command.nparams > PARAMETERS_SIZE) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
		}

		for(i = 0; i < queue->head->command->nparams; i++) {
			queue->head->command->parameters[i] = command.parameters[i];
		}

		if(strcpy(queue->head->command->exec, command.exec) == NULL) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		queue->head->previous->pid = pid;
		queue->head->previous->currentParam = currentParam;
		queue->head->previous->currentQueueId = currentQueueId;

		queue->head->previous->previous = NULL;
		queue->head->previous->next = queue->head;

		queue->head = queue->head->previous;

		queue->nprocesses++;

	} else {

		if(queue->head == NULL) {

		} else if(queue->tail == NULL) {
			printf("\n\nERRO 2\n\n");
		}

		return NULL;

	}

	return queue;

}

PriorityQueue* insertProcessIntoQueue(PriorityQueue *queue, Command command, int pid, int currentQueueId) {

	int i = 0;

	if(queue == NULL) {
		return NULL;
	}

	if(queue->head == NULL && queue->tail == NULL) {

		queue->head = (Process*) malloc (sizeof(Process));

		if(queue->head == NULL) {
			return NULL;
		}

		queue->head->command = createCommand();

		if(queue->head->command == NULL) {
			free(queue->head);
			free(queue->tail);
			queue->head = NULL;
			queue->tail = NULL;

			return NULL;
		}

		queue->head->command->nparams = command.nparams;

		if(command.nparams > PARAMETERS_SIZE) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
		}

		for(i = 0; i < queue->head->command->nparams; i++) {
			queue->head->command->parameters[i] = command.parameters[i];
		}

		if(strcpy(queue->head->command->exec, command.exec) == NULL) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		queue->head->pid = pid;
		queue->head->currentParam = 0;
		queue->head->currentQueueId = currentQueueId;

		queue->head->next = NULL;
		queue->head->previous = NULL;

		queue->tail = queue->head;

		queue->nprocesses = 1;

	} else if(queue->head != NULL && queue->tail != NULL) {

		queue->head->previous = (Process*) malloc (sizeof(Process));

		if(queue->head->previous == NULL) {
			return NULL;
		}

		queue->head->previous->command = createCommand();

		if(queue->head->previous->command == NULL) {
			free(queue->head->previous);
			queue->head->previous = NULL;

			return NULL;
		}

		queue->head->command->nparams = command.nparams;

		if(command.nparams > PARAMETERS_SIZE) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		for(i = 0; i < queue->head->command->nparams; i++) {
			queue->head->command->parameters[i] = command.parameters[i];
		}

		if(strcpy(queue->head->command->exec, command.exec) == NULL) {
			free(queue->head);
			queue->head = NULL;
			queue->tail = NULL;
			return NULL;
		}

		queue->head->previous->pid = pid;
		queue->head->previous->currentParam = 0;
		queue->head->currentQueueId = currentQueueId;

		queue->head->previous->previous = NULL;
		queue->head->previous->next = queue->head;

		queue->head = queue->head->previous;

		queue->nprocesses++;

	} else {

		return NULL;

	}

	return queue;

}

PriorityQueue* retrieveProcessFromQueue(PriorityQueue *queue, int *currentParamIndex, Command* command, int * pid, int *currentQueueId, int* timeForIO) {

	int i = 0, sum = 0;

	if(queue == NULL) {
		return NULL;
	}

	if(command == NULL) {
		return NULL;
	}

	if(pid == NULL) {
		return NULL;
	}

	if(queue->tail == NULL) {
		return NULL;
	}

	if(queue->nprocesses > 0 && queue->tail != NULL) {

		*pid = queue->tail->pid;
		*command = *(queue->tail->command);

		*currentParamIndex = queue->tail->currentParam;
		*currentQueueId = queue->tail->currentQueueId;

		sum = 0;

		*timeForIO = 100000;

		for(i = 0; i < command->nparams; i++) {

			sum += command->parameters[i];

			if(sum > *currentParamIndex) {
				*timeForIO = sum - (*currentParamIndex);
				i = command->nparams;
			}

		}


		if(queue->tail->command != NULL) {

			deallocateCommand(queue->tail->command);
		}

		queue->tail->command = NULL;
		queue->tail = queue->tail->previous;

		if(queue->tail == NULL) {
			queue->head->command = NULL;
			free(queue->head);
			queue->head = NULL;
		} else {
			free(queue->tail->next);

			queue->tail->next = NULL;
		}

		queue->nprocesses--;

		return queue;

	} else {
		return NULL;
	}

}

void printPriorityQueue(PriorityQueue * queue) {

	Process * p;

	if(queue == NULL) {
		return;
	}

	p = queue->tail;

	int processCounter = 0;
	int i = 0;

	printf("\n\tQuantum: %d", queue->quantum);
	printf("\n\tNumber of Processes: %d\n", queue->nprocesses);

	while(p != NULL && queue->nprocesses > 0) {
		printf("\n\t---------------------------------");
		printf("\n\n\tProcess in position %d on the Queue", processCounter);
		printf("\n\tProcess PID: %d", p->pid);
		printf("\n\tCurrent Parameter INDEX: %d", p->currentParam);
		printf("\n\tCurrent Queue ID: %d", p->currentQueueId);
		printf("\n\tNumber of Parameters: %d", p->command->nparams);

		if(p->command != NULL) {

			if(p->command->exec != NULL) {
				printf("\n\tProcess Execute Command: %s\n", p->command->exec);
			}

			if(p->command->parameters) {
				for(i = 0; i < p->command->nparams; i++) {
					printf("\n\t\tProcess Parameter [%d] = %d", i, p->command->parameters[i]);
				}
			}
		}

		p = p->previous;
	}

	return;
}

int searchAndRemoveProcess(PriorityQueue * queue, int pid) {

	Process * p, *previous, *next;
	int removedPid = 1;

	if(queue == NULL) {
		return 0;
	}

	if(queue->head == NULL) {
		return 0;
	}

	if(queue->tail == NULL) {
		return 0;
	}

	if(queue->nprocesses == 1) {

		if(queue->head->pid == pid) {

			if(queue->head->command != NULL) {
				deallocateCommand(queue->head->command);
			}

			queue->head->command = NULL;

			if(queue->head != NULL) {
				free(queue->head);
			}

			queue->head = NULL;
			queue->tail = NULL;

			queue->nprocesses = 0;

			return pid;
		}

		return 0;
	}

	p = queue->tail;

	while(p) {
  
		if(p->pid == pid) {

			if(queue->nprocesses > 0 && queue->tail != NULL) {

     			removedPid = p->pid;
      		deallocateCommand(p->command);
      		p->command = NULL;
      		previous = p->previous;
				next = p->next;

				free(p);
				p = NULL;


      		if(previous == NULL) {

					if(queue->head->command != NULL) {
						deallocateCommand(queue->head->command);
					}

					queue->head->command = NULL;
         		queue->head = next;
					queue->head->previous = NULL;

      		} else if(next == NULL) {

					if(queue->tail->command != NULL) {
						deallocateCommand(queue->tail->command);
					}

					queue->tail->command = NULL;
					queue->tail = previous;
					queue->tail->next = NULL;

				} else {

					previous->next = next;
					
					next->previous = previous;

				}

      		queue->nprocesses--;

				return removedPid;

			} else {
				return 0;
			}
		}

		p = p->previous;
	}

   return 0;
}

void printPriorityQueueBackwards(PriorityQueue * queue) {

	Process * p;

	if(queue == NULL) {
		return;
	}

	p = queue->head;

	int processCounter = 0;
	int i = 0;

	printf("\nQuantum: %d", queue->quantum);
	printf("\nNumber of Processes: %d\n", queue->nprocesses);
   
	while(p) {
		printf("\n\t---------------------------------");
		printf("\n\tProcess in position %d on the Queue", processCounter);
		printf("\n\tProcess PID: %d", p->pid);
		printf("\n\tCurrent Parameter INDEX: %d", p->currentParam);
		printf("\n\tCurrent Queue ID: %d", p->currentQueueId);
		printf("\n\tNumber of Parameters: %d", p->command->nparams);

		if(p->command != NULL) {

			if(p->command->exec != NULL) {
				printf("\n\tProcess Execute Command: %s\n", p->command->exec);
			}

			if(p->command->parameters != NULL) {
				for(i = 0; i < p->command->nparams; i++) {
					printf("\n\t\tProcess Parameter [%d] = %d", i, p->command->parameters[i]);
				}
			}
		}
      
		p = p->next;
	}

	return;
}


void fprintPriorityQueueBackwards(char* fileName, PriorityQueue * queue, char* argument) {

	Process * p;
	FILE *file;

   if(fileName == NULL) {
      return;
   }

   if(argument == NULL) {
      return;
   }

	if(queue == NULL) {
		return;
	}

	file = fopen(fileName, argument);

	if(file == NULL) {
		return;
	}	

	p = queue->head;

	int processCounter = 0;
	int i = 0;

	fprintf(file, "\nQuantum: %d", queue->quantum);
	fprintf(file, "\nNumber of Processes: %d\n", queue->nprocesses);

	while(p) {
		fprintf(file, "\n\t---------------------------------");
		fprintf(file, "\n\tProcess in position %d on the Queue", processCounter);
		fprintf(file, "\n\tProcess PID: %d", p->pid);
		fprintf(file, "\n\tCurrent Parameter INDEX: %d", p->currentParam);
		fprintf(file, "\n\tCurrent Queue ID: %d", p->currentQueueId);

		if(p->command != NULL) {

			if(p->command->exec != NULL) {
				fprintf(file, "\n\tProcess Execute Command: %s\n", p->command->exec);
			}

			if(p->command->parameters != NULL) {
				for(i = 0; i < p->command->nparams; i++) {
					fprintf(file, "\n\t\tProcess Parameter [%d] = %d", i, p->command->parameters[i]);
				}
			}
		}

		p = p->next;
	}

	fclose(file);
	file = NULL;

	return;
}

void fprintPriorityQueue(char* fileName, PriorityQueue * queue, char* argument) {

	Process * p;
	FILE *file;

	if(fileName == NULL) {
		return;
	}

	if(argument == NULL) {
		return;
	}

	if(queue == NULL) {
		return;
	}

	file = fopen(fileName, argument);

	if(file == NULL) {
		return;
	}

	p = queue->tail;

	int processCounter = 0;
	int i = 0;

	fprintf(file, "\nQuantum: %d", queue->quantum);
	fprintf(file, "\nNumber of Processes: %d\n", queue->nprocesses);

	while(p) {
		fprintf(file, "\n\t---------------------------------");
		fprintf(file, "\n\tProcess in position %d on the Queue", processCounter);
		fprintf(file, "\n\tProcess PID: %d", p->pid);
		fprintf(file, "\n\tCurrent Parameter INDEX: %d", p->currentParam);
		fprintf(file, "\n\tCurrent Queue ID: %d", p->currentQueueId);

		if(p->command != NULL) {

			if(p->command->exec != NULL) {
				fprintf(file, "\n\tProcess Execute Command: %s\n", p->command->exec);
			}

			if(p->command->parameters != NULL) {
				for(i = 0; i < p->command->nparams; i++) {
					fprintf(file, "\n\t\tProcess Parameter [%d] = %d", i, p->command->parameters[i]);
				}
			}
		}

		p = p->previous;
	}

	fclose(file);
	file = NULL;

	return;

}

void deallocateQueue(PriorityQueue *pQueue) {

	while(pQueue->head) {

		pQueue->head = pQueue->head->next;

		if(pQueue->head != NULL) {

			if(pQueue->head->previous->command != NULL) {
				deallocateCommand(pQueue->head->previous->command);
			}

			pQueue->head->previous->command = NULL;

			free(pQueue->head->previous);
			pQueue->head->previous = NULL;
		}
	}

	return;
}
