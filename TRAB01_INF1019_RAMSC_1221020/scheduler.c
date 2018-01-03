/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo scheduler.c
 * PUC-Rio
 */

#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include"producerconsumer.h"
#include"PriorityQueue.h"
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<unistd.h>
#include"globalsettings.h"
#include<fcntl.h>
#include<time.h>

#define F1_QUANTUM 1
#define F2_QUANTUM 2
#define F3_QUANTUM 4

#define F1 0
#define F2 1
#define F3 2
#define QUARANTINE 3

#define QUARANTINE_QUANTUM 3

#define DID_NOT_EXCEEDED_QUANTUM_TIME 0
#define EXCEEDED_QUANTUM_TIME 1
#define REAPED 2

int currentPid = 0;
PriorityQueue * priorityQueues[4];

void finishedQuarantineHandler(int signo);
int reinsertBlockedProcess();
void w4IOHandler(int signo);
void finishedProcessHandler(int signo);
void terminatesProgram(int signo);
int printCurrentTime();

static volatile int mutex = 0;
static volatile int processRunning = 0;
static volatile int waitSignal = 0;

int main(int argc, char **argv) {

	Resources *resources;
	Command* runningProgram;
	int interpreterPid = 0, fifoSegment = 0, schedulerPid = 0, removedPid = 0;
	int quantum = 0;
	int currentQueue = -1;
	int queueId = -1;
	int currentParamIndex = 0;
	int timeForIO = 0;

	int quarantineTime = 0;
	int futureQuarantineTime = 0;

	int startTime = 0, endTime = 0;

	int f1Counter = 0, f2Counter = 0, f3Counter = 0;
	Command currentCommand;
	int pid = 0;

	char** arguments;	
	char extraInfo[50];

	int k = 0, i = 0, j = 0;
	
	struct sigaction sigchld;

	FILE* log = fopen("scheduler_Log.txt", "w");

	if(log == NULL) {
		printf("\n\tERROR: Scheduler Log File could not be accessed.\n");
		exit(1);
	}

	fclose(log);
	log = NULL;

	sigchld.sa_handler = finishedProcessHandler;
	sigemptyset(&sigchld.sa_mask);
	sigchld.sa_flags = SA_NOCLDSTOP;

	if(sigaction(SIGCHLD, &sigchld, 0) == -1) {
		printLog("scheduler_Log.txt", "\n\tERROR: sigaction failed for signal SIGCHLD!\n", "a");
		printf("\n\tERROR: sigaction failed for signal SIGCHLD!\n");
		exit(1);
	}

	schedulerPid = getpid();

	printLog("scheduler_Log.txt", "\n\tScheduler Initialized!\n", "a");
	
	#ifdef SET_DEBUG
		printf("\n\tScheduler Initialized! %d\n", schedulerPid);
	#endif

	interpreterPid = strtol(argv[1], NULL, 10);
	fifoSegment = strtol(argv[2], NULL, 10);

	printLog("scheduler_Log.txt", "\n\tLoading shared resources exec and parameters...\n", "a");

	#ifdef SET_DEBUG
		printf("\n\tLoading shared resources exec and parameters...\n");
	#endif

	resources = (Resources*) shmat(fifoSegment, 0, 0);
	
	if(resources == NULL) {
		printLog("scheduler_Log.txt", "\n\tError: Producer-Consumer FIFO could be found as shared memory resource.\n", "a");
		printf("\n\tError: Producer-Consumer FIFO could be found as shared memory resource.\n");
		shmdt(resources);
		exit(1);
	}

	printLog("scheduler_Log.txt", "\n\tScheduler Initialized!\n", "a");

	#ifdef SET_DEBUG
		printf("\n\tScheduler Initialized!\n");
	#endif

	printLog("scheduler_Log.txt", "\n\tCreating Priority Queues F1, F2, F3 and the Quarantine Queue...\n", "a");

	#ifdef SET_DEBUG
		printf("\n\tCreating Priority Queues F1, F2, F3 and the Quarantine Queue...\n");
	#endif

	priorityQueues[F1] 			= createPriorityQueue(F1_QUANTUM);
	priorityQueues[F2] 			= createPriorityQueue(F2_QUANTUM);
	priorityQueues[F3] 			= createPriorityQueue(F3_QUANTUM);
	priorityQueues[QUARANTINE] = createPriorityQueue(QUARANTINE_QUANTUM);
	
	if(priorityQueues[F1] == NULL || priorityQueues[F2] == NULL || priorityQueues[F3] == NULL) {
		printLog("scheduler_Log.txt", "\n\tError: Failed to create Priority Queues.\n", "a");
		printf("\n\tError: Failed to create Priority Queues.\n");
		exit(1);
	}

	if(priorityQueues[QUARANTINE] == NULL) {

		printLog("scheduler_Log.txt", "\n\tFailed to create Quarantine Queue.\n", "a");
		printf("\n\tFailed to create Quarantine Queue.\n");
		deallocateQueue(priorityQueues[F1]);
		deallocateQueue(priorityQueues[F2]);
		deallocateQueue(priorityQueues[F3]);
		exit(1);
	}

	//signal(SIGALRM, finishedQuarantineHandler);
	signal(SIGUSR1, w4IOHandler);

	while(1) {

		if(resources->nelem > 0) {

			#ifdef SET_DEBUG
				printf("\n\tConsumer attempt to retrieve new processes...\n");
			#endif

			runningProgram = NULL;

			runningProgram = createCommand();

			consumer(resources, interpreterPid, runningProgram);

			arguments = (char**) malloc ((runningProgram->nparams + 3) * sizeof(char*));

			arguments[0] = (char*) malloc (strlen(runningProgram->exec) * sizeof(char));

			strcpy(arguments[0], runningProgram->exec);

			#ifdef SET_DEBUG
				printf("\n\tArgument[0] = %s\n", arguments[0]);
			#endif

			for(k = 1; k < runningProgram->nparams + 1; k++) {

			arguments[k] = (char*) malloc (10 * sizeof(char));

			snprintf(arguments[k], 10, "%d", runningProgram->parameters[k-1]);

				#ifdef SET_DEBUG
				printf("\n\tArgument[%d] = %s\n", k, arguments[k]);
				#endif
			
			}

			arguments[k] = (char*) malloc (15 * sizeof(char));

			snprintf(arguments[k], 15, "%d", schedulerPid);

			#ifdef SET_DEBUG
				printf("\n\tArgument[%d] = %s\n", k, arguments[k]);
			#endif

			arguments[k + 1] = NULL;

			pid = fork();

			if(pid > 0) {

				kill(pid, SIGSTOP);
	
				printLog("scheduler_Log.txt", "\n\tInserting new process into Priority Queue F1 (Queue of ready processes)...\n", "a");

				#ifdef SET_DEBUG
					printf("\n\tInserting new process into Priority Queue F1 (Queue of ready processes)... %d %d\n", pid, getpid());
				#endif

				if(insertProcessIntoQueue(priorityQueues[F1], *runningProgram, pid, F1) != NULL) {

					#ifdef SET_DEBUG
						printf("\n\n\t==>Process PID <%d> was inserted into Queue 1\n\n", pid);
					#endif

					#ifdef SET_DEBUG
						printPriorityQueue(priorityQueues[F1]);
						printPriorityQueue(priorityQueues[F2]);
						printPriorityQueue(priorityQueues[F3]);
					#endif

				}

			} else if(pid == 0) {

				printLog("scheduler_Log.txt", "\n\tChild Process created!", "a");

				#ifdef SET_DEBUG
				printf("\nChild Process created! PID = %d / Parent's PID = %d\n", getpid(), getppid());			
				printf("\n\tProcess <PID %d ; %s> is about to start executing... \n", getpid(), runningProgram->exec);
				#endif

				execv(runningProgram->exec, arguments);

				printLog("scheduler_Log.txt", "\nERROR: execv failed! Exiting Child Process...\n", "a");
				#ifdef SET_DEBUG
				printf("\nERROR: execv failed! Exiting Child Process PID %d\n", getpid());
				#endif

				exit(1);
	
			} else {

				printLog("scheduler_Log.txt", "\nError: Scheduler could not create child process!\n", "a");
				printf("\nError: Scheduler could not create child process!\n");

				deallocateQueue(priorityQueues[F1]);
				deallocateQueue(priorityQueues[F2]);
				deallocateQueue(priorityQueues[F3]);
				deallocateQueue(priorityQueues[QUARANTINE]);
				deallocateCommand(runningProgram);
				runningProgram = NULL;

				exit(1);
			}

			deallocateCommand(runningProgram);
		}

		while(getQueueSize(priorityQueues[QUARANTINE]) > 0 && (quarantineTime >= QUARANTINE_QUANTUM)) {

			if(reinsertBlockedProcess() != 1) {
				printf("\nERROR! FAILED TO REINSERT BLOCKED PROCESS!\n");
				exit(1);
			}

			quarantineTime -= QUARANTINE_QUANTUM;

			if(futureQuarantineTime == 1) {
				quarantineTime += 1;
			} else if(futureQuarantineTime == 2) {
				quarantineTime += 2;
			} else if(futureQuarantineTime == 3) {
				quarantineTime += 2;
				if(quarantineTime >= QUARANTINE_QUANTUM) {
					quarantineTime++;
				}
			}

			futureQuarantineTime = 0;
		}

		if(currentQueue == -1) {
			f1Counter = getQueueSize(priorityQueues[F1]);
			f2Counter = getQueueSize(priorityQueues[F2]);
			f3Counter = getQueueSize(priorityQueues[F3]);

			if(f1Counter != 0) {
				currentQueue = F1;
			} else if(f2Counter != 0) {
				currentQueue = F2;
			} else if(f3Counter !=0) {
				currentQueue = F3;
			} else {

				currentQueue = -1;

				if(getQueueSize(priorityQueues[QUARANTINE]) > 0) {
					sleep(1);
					quarantineTime++;
				}
			}

		} else if(currentQueue == F1 && f1Counter == 0) {

			f2Counter = getQueueSize(priorityQueues[F2]);
			f3Counter = getQueueSize(priorityQueues[F3]);

			if(f2Counter != 0) {
				currentQueue = F2;
			} else if(f3Counter != 0) {
				currentQueue = F3;
			} else {
				currentQueue = -1;
			}

		} else if(currentQueue == F2 && f2Counter == 0) {

			f3Counter = getQueueSize(priorityQueues[F3]);

			if(f3Counter != 0) {
				currentQueue = F3;
			} else {
				currentQueue = -1;
			}

		} else if(currentQueue == F3 && f3Counter == 0) {

			f1Counter = getQueueSize(priorityQueues[F1]);
			f2Counter = getQueueSize(priorityQueues[F2]);
			f3Counter = getQueueSize(priorityQueues[F3]);

			if(f1Counter != 0) {
				currentQueue = F1;
			} else if(f2Counter != 0) {
				currentQueue = F2;
			} else if(f3Counter !=0) {
				currentQueue = F3;
			} else {
				currentQueue = -1;
			}

			printLog("scheduler_Log.txt", "\n\tRound complete! Starting a new round...\n", "a");
			#ifdef SET_DEBUG
			printf("\n\tRound complete! Starting a new round...\n");
			#endif

		}

		if(currentQueue == F1) {
			
			#ifdef SET_DEBUG
			printf("\n\tProcess from Priority Queue F1 was selected! %d\n", f1Counter);
			printPriorityQueue(priorityQueues[F1]);
			#endif

			if(retrieveProcessFromQueue(priorityQueues[F1], &currentParamIndex, &currentCommand, &currentPid, &queueId, &timeForIO) != NULL) {

				printLog("scheduler_Log.txt", "\n\tProcess from Priority Queue F1 was selected!\n", "a");				
				#ifdef SCHEDULER_EXEC
				printf("\n\tProcess <PID %d> was correctly retrieved from Priority Queue F1!\n", currentPid);
				#endif

				quantum = getQueueQuantum(priorityQueues[F1]);
				f1Counter--;
			}

		} else if(currentQueue == F2) {

			#ifdef SET_DEBUG
				printf("\n\tProcess from Priority Queue F2 was selected! %d\n", f2Counter);
				printPriorityQueue(priorityQueues[F2]);
			#endif

			if(retrieveProcessFromQueue(priorityQueues[F2], &currentParamIndex, &currentCommand, &currentPid, &queueId, &timeForIO) != NULL) {

				printLog("scheduler_Log.txt", "\n\tProcess from Priority Queue F2 was selected!\n", "a");
				#ifdef SCHEDULER_EXEC
				printf("\n\tProcess <PID %d> was correctly retrieved from Priority Queue F2! %d\n", currentPid, f2Counter);
				#endif

				quantum = getQueueQuantum(priorityQueues[F2]);
				f2Counter--;

			}

		} else if(currentQueue == F3) {
			
			#ifdef SET_DEBUG
			printf("\n\tProcess from Priority Queue F3 was selected! %d\n", f3Counter);
			printPriorityQueue(priorityQueues[F3]);
			#endif

			if(retrieveProcessFromQueue(priorityQueues[F3], &currentParamIndex, &currentCommand, &currentPid, &queueId, &timeForIO) != NULL) {

				printLog("scheduler_Log.txt", "\n\tProcess from Priority Queue F3 was selected!\n", "a");
				#ifdef SCHEDULER_EXEC			
				printf("\n\tProcess <PID %d> was correctly retrieved from Priority Queue F3!\n", currentPid);
				#endif

				quantum = getQueueQuantum(priorityQueues[F3]);
				f3Counter--;
			}

		}

		if(currentQueue == F1 || currentQueue == F2 || currentQueue == F3) {

			snprintf(extraInfo, 10, "%d", currentPid);

			printLog("scheduler_Log.txt", "\nAbout to execute scheduled process...\n", "a");
			printLog("scheduler_Log.txt", extraInfo, "a");
			
			printf("\nAbout to execute scheduled process <PID %d> from Priority Queue F%d!\n", currentPid, currentQueue + 1);

			processRunning = EXCEEDED_QUANTUM_TIME;

			#ifdef SET_DEBUG	
				printPriorityQueue(priorityQueues[F1]);
				printPriorityQueue(priorityQueues[F2]);
				printPriorityQueue(priorityQueues[F3]);
			#endif

			if(timeForIO < quantum) {
				currentParamIndex += timeForIO;
			} else {
				currentParamIndex += quantum;
			}

			if(timeForIO <= quantum) {
				waitSignal = 1;
				while(waitSignal != 1);
			} else {
				waitSignal = 0;
				while(waitSignal != 0);
			}

			#ifdef SET_DEBUG
			printf("\n\ntimeForIO %d ; waitSignal %d ; quantum %d \n\n", timeForIO, waitSignal, quantum);
			#endif

			startTime = printCurrentTime();

			kill(currentPid, SIGCONT);

			sleep(quantum);

			while(waitSignal != 0);

			kill(currentPid, SIGSTOP);

			endTime = printCurrentTime();

			if(getQueueSize(priorityQueues[QUARANTINE]) > 0) {
				if((endTime - startTime) >= 0) {
					quarantineTime += (endTime - startTime);
				} else {
					quarantineTime += (startTime - endTime);
				}
			}

			if(quarantineTime > QUARANTINE_QUANTUM && getQueueSize(priorityQueues[QUARANTINE]) == 1) {
				quarantineTime = 3;
			}

			if(processRunning == EXCEEDED_QUANTUM_TIME) {
				// O tempo de quantum não foi suficiente para a corrente rajada de CPU desse processo.
	
				printLog("scheduler_Log.txt", "\nProcess exceeded the respective priority queue's quantum time.\n", "a");
				#ifdef SCHEDULER_EXEC
				printf("\n\tProcess <PID %d> exceeded the respective priority queue's quantum time.\n\n", currentPid);
				#endif

				if(currentQueue == F3 || currentQueue == F2) {

					printLog("scheduler_Log.txt", "\nProcess exceeded the respective priority queue's quantum time.\n", "a");
					#ifdef SCHEDULER_EXEC
					printf("\n\tInserting Process PID = %d in Priority Queue F3 Queue...\n\n", currentPid);
					#endif

					reinsertIntoQueue(priorityQueues[F3], currentCommand, currentPid, currentParamIndex, F3);

				} else if(currentQueue == F1) {

					printLog("scheduler_Log.txt", "\nProcess exceeded the respective priority queue's quantum time.\n", "a");
					#ifdef SCHEDULER_EXEC
					printf("\n\tInserting Process PID = %d in Priority Queue F2 Queue...\n\n", currentPid);
					#endif

					reinsertIntoQueue(priorityQueues[F2], currentCommand, currentPid, currentParamIndex, F2);
	
				} else {

					deallocateQueue(priorityQueues[F1]);
					deallocateQueue(priorityQueues[F2]);
					deallocateQueue(priorityQueues[F3]);
					deallocateQueue(priorityQueues[QUARANTINE]);

					exit(1);
				}

			} else if(processRunning == DID_NOT_EXCEEDED_QUANTUM_TIME) {
				// O processo parou antes de exceder o tempo de quantum da respectiva fila de prioridade.

				printLog("scheduler_Log.txt", "\nProcess did not exceed the respective priority queue's quantum time.\n", "a");
				#ifdef SCHEDULER_EXEC
				printf("\n\tProcess <PID %d> did not exceed the respective priority queue's quantum time.\n", currentPid);
				#endif

				if(currentQueue == F1 || currentQueue == F2) {

					printLog("scheduler_Log.txt", "\n\tInserting Process PID = %d in Quarantine F1 Queue...\n", "a");

					#ifdef SCHEDULER_EXEC
					printf("\n\tInserting Process PID = %d in Quarantine F1 Queue...\n", currentPid);
					#endif

					reinsertIntoQueue(priorityQueues[QUARANTINE], currentCommand, currentPid, currentParamIndex, F1);

					if(quarantineTime > 0 && quarantineTime < QUARANTINE_QUANTUM) {
						futureQuarantineTime += (QUARANTINE_QUANTUM - quarantineTime);
					}

					#ifdef SET_DEBUG
					printf("\n\tQuarantine Time = %d\n", quarantineTime);
					printf("\tFuture Quarantine Time = %d\n", futureQuarantineTime);
					#endif
					//printPriorityQueue(priorityQueues[QUARANTINE]);
					//printPriorityQueue(priorityQueues[F1]);
				} else if(currentQueue == F3) {

					printLog("scheduler_Log.txt", "\n\tInserting Process PID = %d in Quarantine F2 Queue...\n", "a");

					#ifdef SCHEDULER_EXEC
					printf("\n\tInserting Process PID = %d in Quarantine F2 Queue...\n\n", currentPid);
					#endif

					reinsertIntoQueue(priorityQueues[QUARANTINE], currentCommand, currentPid, currentParamIndex, F2);

					if(quarantineTime > 0 && quarantineTime < QUARANTINE_QUANTUM) {
						futureQuarantineTime += (QUARANTINE_QUANTUM - quarantineTime);
					}

					#ifdef SET_DEBUG
					printf("\n\tQuarantine Time = %d\n", quarantineTime);
					printf("\tFuture Quarantine Time = %d\n", futureQuarantineTime);
					#endif
					//printPriorityQueue(priorityQueues[QUARANTINE]);
					//printPriorityQueue(priorityQueues[F1]);
				} else {

					deallocateQueue(priorityQueues[F1]);
					deallocateQueue(priorityQueues[F2]);
					deallocateQueue(priorityQueues[F3]);
					deallocateQueue(priorityQueues[QUARANTINE]);
					
					exit(1);
				}

			} else if(processRunning == REAPED) {

				snprintf(extraInfo, 10, "%d", currentPid);
				printLog("scheduler_Log.txt", extraInfo, "a");
				printLog("scheduler_Log.txt", "\n\tProcess reaped and removed from the priority queues.\n", "a");
               
				printf("\n\tProcess PID %d reaped and removed from the priority queues.\n", currentPid);	
			}

			#ifdef SET_DEBUG
			printf("\n\t===== QUEUE F1 =====\n");
			printPriorityQueue(priorityQueues[F1]);
			printf("\n\t===== QUEUE F2 =====\n");
			printPriorityQueue(priorityQueues[F2]);
			printf("\n\t===== QUEUE F3 =====\n");
			printPriorityQueue(priorityQueues[F3]);
			printf("\n\t===== QUEUE QUARANTINE =====\n");
			printPriorityQueue(priorityQueues[QUARANTINE]);
			#endif

		}

	}

	return 0;
}

void w4IOHandler(int signo) {

	kill(currentPid, SIGSTOP);
	kill(currentPid, SIGUSR2);

	waitSignal = 0;

	processRunning = DID_NOT_EXCEEDED_QUANTUM_TIME;

	printf("\n\tI/O signal (w4IOHandler received!)\n");

	return;

}

void finishedProcessHandler(int signo) {

	int status;
	int pid = wait(&status);

	waitSignal = 0;

	processRunning = REAPED;

	#ifdef SCHEDULER_EXEC
	printf("\n\tReap process %d\n", pid);
	#endif

	return;
}

int reinsertBlockedProcess() {

	int lcurrentPid, lcurrentParamIndex, lcurrentQueue, ltimeForIO;
	Command lcurrentCommand;
	FILE* LOG;

	#ifdef SET_DEBUG
	printf("\n\n\tQuarantine finished.\n\n");
	#endif

	PriorityQueue * pQueue = retrieveProcessFromQueue(priorityQueues[QUARANTINE], &lcurrentParamIndex, &lcurrentCommand, &lcurrentPid, &lcurrentQueue, &ltimeForIO);

	if(pQueue != NULL) {
	
		printLog("scheduler_Log.txt", "\t\nReinserting process in Priority Queue...\n", "a");

		printf("\nReinserting process <PID %d> in Priority Queue F%d!", lcurrentPid, lcurrentQueue + 1);
		printCurrentTime();

		if(reinsertIntoQueue(priorityQueues[lcurrentQueue], lcurrentCommand, lcurrentPid, lcurrentParamIndex, lcurrentQueue) == NULL) {
			return -1;
		}

		return 1;

	} else {

		printLog("scheduler_Log.txt", "\n\tQuarantine is empty!\n", "a");
		#ifdef SET_DEBUG
		printf("\n\n\tQuarantine is empty!\n\n");
		#endif
	
	}

	return -1;

}

int printCurrentTime() {

	time_t timed;
	struct tm * timeData;
	
	time(&timed);
	timeData = localtime(&timed);
	printf("\n\tCurrent Time: %s\n", asctime(timeData));

	return timeData->tm_sec;

}
