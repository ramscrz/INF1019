/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo MemoryManager.h
 * PUC-Rio
 */

#include"VM.h"
#include"Semaphore.h"
#include"PageFrameTable.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<errno.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<sys/wait.h>

#define RUNNING_PROCESS 0
#define STOPPED_PROCESS 1
#define DEAD_PROCESS 2
#define ERASED_PROCESS 3
#define WAITING_PROCESS 4

static unsigned int processStopped[N_USER_PROCESSES];
static unsigned int startTime[N_USER_PROCESSES];
static unsigned int timeLimit[N_USER_PROCESSES];
PageTable* pageTables[N_USER_PROCESSES];
static PageFrameTable * pageFrameTable;
static pid_t pid[N_USER_PROCESSES] = {0};
static pid_t pidLostPageProcess[N_USER_PROCESSES] = {-1};
volatile static sig_atomic_t processUnblocked = 0;
volatile static sig_atomic_t deadProcessPid = -1;
volatile static sig_atomic_t deadProcesses = RUNNING_PROCESS;
volatile static sig_atomic_t pageFault;
volatile static sig_atomic_t interrupted = 0;

void registerPageFaultHandler(int signo, siginfo_t *info, void *context);
void finishedProcessHandler(int signo, siginfo_t *info, void *context);
void processUnblockedHandler(int signo);
void forcedExitHandler(int signo);

int main(int argc, char** argv) {

	PageTable* pageTable = NULL;

	pid_t pidGM = getpid();

	int processIndex = 0, u = 0, i = 0, k = 0, livingProcessesSegment = 0;
	int *livingProcesses = NULL;

	char **params;

	struct sigaction sigCHLD, sigPageFault, sigUnblock;

	FILE* userProcessesFile = NULL;

	char userProcessName[N_USER_PROCESSES][100];

	int deathCounter = 0;

	int pageFaultSem;
	int unblockedSem;

	if(N_USER_PROCESSES > NUMBER_PAGE_FRAMES) {
		printf("\n\nERROR: There are more user processes than page frames!\n\n");
		exit(1);
	}

	pageFaultSem = setupSemaphore();
	unblockedSem = setupSemaphore();

	#ifdef DEBUGMODE
	printf("\n\nSemaphore ID: pageFaultSem %d unblockedSem %d\n\n", pageFaultSem, unblockedSem);
	#endif
	
	pageFault = 0;

	printCurrentTime(1);

	if(argc != 3) {
		printf("\nERROR: insufficient parameters! Run: ./<Program Name> <Name of the file containing the file name for each user process (total of %d user processes).> <Name of the user program executable file>\n\n", N_USER_PROCESSES);
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	signal(SIGINT, forcedExitHandler);

	memset(&sigPageFault, 0, sizeof(struct sigaction));
	sigPageFault.sa_flags = SA_SIGINFO|SA_RESTART;
	sigPageFault.sa_sigaction = registerPageFaultHandler;
	sigemptyset(&sigPageFault.sa_mask);

	if(sigaction(SIGUSR1, &sigPageFault, 0) == -1) {
		printf("\n\tERROR: sigaction failed for signal SIGCHLD <PROCESS FINISHED>!\n");
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	memset(&sigUnblock, 0, sizeof(struct sigaction));
	sigUnblock.sa_flags = SA_SIGINFO|SA_RESTART;
	sigUnblock.sa_handler = processUnblockedHandler;
	sigemptyset(&sigUnblock.sa_mask);

	if(sigaction(SIGUSR2, &sigUnblock, 0) == -1) {
		printf("\n\tERROR: sigaction failed for signal SIGCHLD <PROCESS FINISHED>!\n");
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	#ifdef DEBUGMODE
	printf("\nPARENT PID = %d\n", getpid());
	#endif

	pageFrameTable = initVM();

	if(pageFrameTable == NULL) {
		printf("\nERROR: GM could not allocate PageFrameTable.\n");
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	for(u = 0; u < N_USER_PROCESSES; u++) {
		pageTables[u] = NULL;
	}

	for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {
		processStopped[processIndex] = RUNNING_PROCESS;
	}

	livingProcessesSegment = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);

	if(livingProcessesSegment < 0) {

		shmctl(livingProcessesSegment, IPC_RMID, 0);
		
		livingProcessesSegment = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		
		if(livingProcessesSegment < 0) {

			printf("\n\nERROR: shmget could not allocate a living process counter (errno: %s).\n\n", strerror(errno));
			delSemValue(unblockedSem);
			delSemValue(pageFaultSem);
			exit(-1);
		}
	}

	livingProcesses = (int*) shmat(livingProcessesSegment, 0, 0);
	if(livingProcesses == NULL) {
		shmctl(livingProcessesSegment, IPC_RMID, 0);
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	shmctl(livingProcessesSegment, IPC_RMID, 0);

	*livingProcesses = 0;

	userProcessesFile = fopen(argv[1], "r");
	if(userProcessesFile == NULL) {
		printf("\nERROR: file %s containing the name of the file for each user process (total of %d user processes) does not exist.\n", 
							argv[1], N_USER_PROCESSES);
		delSemValue(unblockedSem);
		delSemValue(pageFaultSem);
		exit(-1);
	}

	for(i = 0; i < N_USER_PROCESSES; i++) {	

		if(fgets(userProcessName[i], 100, userProcessesFile) == NULL) {
			fclose(userProcessesFile);
			printf("\nERROR: GM could not read process name from file %s.\n", argv[1]);
			delSemValue(unblockedSem);
			delSemValue(pageFaultSem);
			exit(-1);
		}
	}

	for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {
			pageTables[processIndex] = createPageTable(processIndex, 0, pageFrameTable);
			if(pageTables[processIndex] == NULL) {
				printf("\nERROR: GM could not allocate PageTable for all the processes\n");
				delSemValue(unblockedSem);
				delSemValue(pageFaultSem);
				exit(-1);
			}
	}

	printFramedPages(pageFrameTable);

	printf("\n=============== Start of Simulation: =================\n");
	printCurrentTime(1);

	for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {

		pid[processIndex] = fork();

		if(pid[processIndex] > 0) {

			(*livingProcesses)++;

		} else if(pid[processIndex] == 0) {

			pid[processIndex] = getpid();

			assignPageTable(pageTables[processIndex], pid[processIndex]);

			printf("\nSetting up process 0%d, PID = %d (%d)\n", processIndex, pid[processIndex], getPid(pageTables[processIndex]));

			printPageTable(pageTables[processIndex]);

			params = (char**) malloc(6 * sizeof(char*));

			for(u = 0; u < 4; u++) {
				params[u] = NULL;
			}

			if(params) {

				params[0] = (char*) malloc(61 * sizeof(char));
				params[1] = (char*) malloc(16 * sizeof(char));
				params[2] = (char*) malloc(6 * sizeof(char));
				params[3] = (char*) malloc(16 * sizeof(char));
				params[4] = (char*) malloc(16 * sizeof(char));

				if(params[0] != NULL && params[1] != NULL && params[2] != NULL 
						&& params[3] != NULL && params[4] != NULL && params[5] != NULL) {

					int sharedMemSgmt = getPageTableSegment(pageTables[processIndex]);

					if(sharedMemSgmt >= 0) {

						snprintf(params[0], 60, "%s", userProcessName[processIndex]);
						snprintf(params[1], 15, "%d", pidGM);
						snprintf(params[2], 5, "%d", processIndex);
						snprintf(params[3], 15, "%d", sharedMemSgmt);
						snprintf(params[4], 15, "%d", unblockedSem);

						params[5] = NULL;

						execv(argv[2], params);

						printf("\nERROR: execv failed to execute process #%d (%s).\n", processIndex, userProcessName[processIndex]);

						removeProcessFromPageFrame(pageTables[processIndex]);
						erasePageTable(pageTables[processIndex]);
						pageTables[processIndex] = NULL;

						(*livingProcesses)--;

						exit(-1);
					}

				} else {

					printf("\nERROR: at least one of the parameters of P0%d (%s) could not be allocated.\n", 
										processIndex, userProcessName[processIndex]);

					for(u = 0; u < 4; u++) {
						if(params[u] != NULL) {
							free(params[u]);
							params[u] = NULL;
						}
					}

					exit(-1);
				}

			} else {


				printf("\nERROR: parameters of process %d (%s) could not be allocated.\n", 
								processIndex, userProcessName[processIndex]);
				exit(-1);

			}

		} else {

			for(u = 0; u < processIndex; u++) {
				kill(pid[u], SIGKILL);
			}

			printf("\nError at fork(): process <INDEX %d> could not be created.\n", processIndex);
			delSemValue(unblockedSem);
			delSemValue(pageFaultSem);
			exit(-1);

		}
	}

	while((*livingProcesses) > 0) {

		for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {

			if(processStopped[processIndex] != ERASED_PROCESS) {
				if(getPid(pageTables[processIndex]) == getpid()) {
					#ifdef DEBUGMODE					
					printf("\nDEATH COUNTER UP! %d PROCESS %d\n", deathCounter, processIndex);
					#endif
					processStopped[processIndex] = ERASED_PROCESS;
					deathCounter++;
				}
			}
		}

		//if(pageFault > 0) {
			#ifdef DEBUGMODE
			//printf("\npageFault counter %d\n", pageFault);
			#endif
			for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {
				
				if(getWaitingProcess(pageTables[processIndex]) == 1) {

					kill(pid[processIndex], SIGSTOP);

					PAGE_FRAME_INDEX pageFrameIndex = 0;
					PAGE_INDEX pageIndex = 0;
					pid_t procssPid = 0;
					int index = -1, time = 0;
					MAP_T map = MAP_T_OVERWRITE;

					pageFault--;

					unsetWaitingProcess(pageTables[processIndex]);

					map = searchLFUFramePage(pageTables[processIndex], pageFrameTable, &procssPid,
	 																	&pageFrameIndex, &pageIndex, pageTables);

				
					#ifdef DEBUGMODE
					if(map == MAP_T_ERROR) {
						printf("\n\n\nERROR: PAGE FRAME SELECTED: %d ; pageIndex = %d\n\n\n", pageFrameIndex, pageIndex);
					}
					#endif

					if(map == MAP_T_ERROR) {

						printf("\nLFU Error!\n");

					} else if(map == MAP_T_REPLACE) {

						#ifdef DEBUGMODE
						printf("\n\nprocssPid %d\n\n", procssPid);
						#endif

						for(u = 0; u < N_USER_PROCESSES; u++) {

							#ifdef DEBUGMODE
							printf("\n\nCandidate Process PID %d\n\n", pid[u]);
							#endif

							if(pid[u] == procssPid) {
								#ifdef DEBUGMODE
								printf("\n\nPID of Process owner of a page selected to be replaced: %d\n\n", pid[u]);
								#endif							
								index = u;
							}

						}

						#ifdef DEBUGMODE
						printf("\nPROCESS STOPPED 7 %d %d \n", processIndex, index);
						#endif

						if(index >= 0) {
							time = selectPageFrame(pageFrameTable, pageTables[index], pageFrameIndex, pid[processIndex]);
							if(time > 0) {

								#ifdef DEBUGMODE
								printf("\nPROCESS STOPPED 8.1 %d %d \n", processIndex, index);
								#endif

								processStopped[processIndex] = WAITING_PROCESS;
								startTime[processIndex] = printCurrentTime(1);
								timeLimit[processIndex] = time;
								pidLostPageProcess[processIndex] = pid[index];

								if(time == 2) {
									printf("\n(LFU Save Modified Page) Process 0%d PageFault Penalty Time Interval: %ds\n", processIndex, time);
								} else {
									printf("\n(LFU Overwrite) Process 0%d PageFault Penalty Time Interval: %ds\n", processIndex, time);
								}

								#ifdef VMGMDETAIL
								printf("\n(LFU) Process 0%d PageFault Penalty Time Interval: START TIME = %ds ; Duracao: %ds\n", processIndex, startTime[processIndex], time);
								printf("\n(LFU) Page Process 0%d lost a page (PAGE INDEX: %x) allocated in page frame %d!\n", processIndex, getLostPageIndex(pageTables[index]), getLostPageFrameIndex(pageTables[index]));
								#endif

							} else {
								printf("\nERROR: updatePageTable failed in MemoryManager.\n");
								kill(pid[processIndex], SIGKILL);
							}
						}

					} else if(map == MAP_T_OVERWRITE) {

						// SIMPLESMENTE ALOCA A PÁGINA.						
						time = selectPageFrame(pageFrameTable, NULL, pageFrameIndex, pid[processIndex]);
						if(time > 0) {
							#ifdef DEBUGMODE
							printf("\nPROCESS STOPPED 8.2 %d %d %d\n", processIndex, index, pid[processIndex]);
							#endif

							processStopped[processIndex] = WAITING_PROCESS;
							startTime[processIndex] = printCurrentTime(1);
							timeLimit[processIndex] = time;
							pidLostPageProcess[processIndex] = -1;

							printf("\n(Overwrite Empty Frame) Process 0%d PageFault Penalty Time Interval: %ds\n", processIndex, time);

							#ifdef VMGMDETAIL
							printf("\n(Overwrite) Process 0%d PageFault Penalty Time Interval: START TIME = %ds ; Duracao: %ds\n", processIndex, startTime[processIndex], time);
							#endif
						} else {
							printf("\nERROR: updatePageTable failed in MemoryManager.\n");
							kill(pid[processIndex], SIGKILL);
						}
					}
				}
			}

		//}

		for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {

			if(processStopped[processIndex] == WAITING_PROCESS) {

				int endTime = printCurrentTime(0);
				// Calculate Time from startTime until now.
				// If at least 2 seconds have passed, then unblock the process.
				if((endTime - startTime[processIndex]) >= timeLimit[processIndex]) {

					int previousOwnerPid = updatePageTable(pageTables, pageFrameTable, processIndex, pid[processIndex]);

					if(previousOwnerPid > 0) {
						kill(previousOwnerPid, SIGUSR2);
					}

					#ifdef VMGMDETAIL
					printf("\nProcess %d unblocked! PID %d\n", processIndex, pid[processIndex]);
					printCurrentTime(1);
					#endif

					//processUnblocked = 0;

					kill(pid[processIndex], SIGCONT);

	//				while(processUnblocked == 0);		

					processStopped[processIndex] = RUNNING_PROCESS;

//					semaforoV(unblockedSem);

				} else if((endTime - startTime[processIndex]) < 0) {
					if((endTime - startTime[processIndex]) + 60 >= timeLimit[processIndex]) {

						int previousOwnerPid = updatePageTable(pageTables, pageFrameTable, processIndex, pid[processIndex]);

						if(previousOwnerPid > 0) {
							kill(previousOwnerPid, SIGUSR2);
						}

						#ifdef VMGMDETAIL
						printf("\nProcess %d unblocked! PID %d\n", processIndex, pid[processIndex]);
						printCurrentTime(1);
						#endif

						//processUnblocked = 0;

						kill(pid[processIndex], SIGCONT);

						#ifdef DEBUGMODE
						printf("\n\n\n\nHITOTSU 2\n\n\n\n");
						#endif
						//kill(pid[processIndex], SIGUSR1);
		//				while(processUnblocked == 0);

						#ifdef DEBUGMODE
						printf("\n\n\n\nHITOTSU 3\n\n\n\n");
						#endif

						processStopped[processIndex] = RUNNING_PROCESS;

			//			semaforoV(unblockedSem);
					}
				}

			}

		}

		if(deathCounter == N_USER_PROCESSES) {

			#ifdef DEBUGMODE
			printf("\nDeath Counter FULL!\n");
			#endif

			for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {

				if(pageTables[processIndex] != NULL) {

					#ifdef DEBUGMODE
					printf("\n\n\nDEAD!\n\n\n");
					#endif

					erasePageTable(pageTables[processIndex]);
					pageTables[processIndex] = NULL;

					printf("\nProcess P%d finished executing!\n", processIndex);
					(*livingProcesses)--;

					#ifdef DEBUGMODE
					printf("\n#LivingProcesses = %d\n", *livingProcesses);
					#endif

					deadProcesses = ERASED_PROCESS;
				}
			}
		}

		if(interrupted == 1) {
			for(u = 0; u < processIndex; u++) {
				kill(pid[u], SIGKILL);
			}

			*livingProcesses = 0;
		}

		// LFU Time Window Counter
		incrementTimeUnit(pageFrameTable);

	}

	printf("\n=============== End of Simulation: ===============\n");
	printCurrentTime(1);

	for(processIndex = 0; processIndex < N_USER_PROCESSES; processIndex++) {
		
		if(pageTables[processIndex] != NULL) {

			removeProcessFromPageFrame(pageTables[processIndex]);
			erasePageTable(pageTables[processIndex]);
			pageTables[processIndex] = NULL;
			
			printf("\nProcess P%d removed!\n", processIndex);
		} else {
			//printf("\nProcess P%d has already been removed.\n", processIndex);
		}
	}

	semaforoP(unblockedSem);
	delSemValue(unblockedSem);
	semaforoP(pageFaultSem);
	delSemValue(pageFaultSem);	

	shmdt(livingProcesses);
	livingProcesses = NULL;

	destroyPageFrameTable(pageFrameTable);

	shmdt(pageFrameTable);
	pageFrameTable = NULL;

	return 0;
}

void registerPageFaultHandler(int signo, siginfo_t *info, void *context) {

	char *message = "\n\nPageFault received!\n\n";
	int u = 0;

	pageFault++;

	write(STDOUT_FILENO, message, strlen(message)-1);
	
	return;
}

void finishedProcessHandler(int signo, siginfo_t *info, void *context) {

	int status = 0;
	char *message = "\n\n\n\nDEAD PROCESS %d\n\n\n\n";
	
	deadProcesses = DEAD_PROCESS;

	deadProcessPid = info->si_pid;

	#ifdef DEBUGMODE
	write(STDOUT_FILENO, message, strlen(message)-1);
	#endif
	
	return;

}

void processUnblockedHandler(int signo) {
	
	char *message = "\n\n\n\nPROCESS UNLOCKED SIGNAL RECEIVED!%d\n\n\n\n";

	if(processUnblocked == 1) printf("\n\nALREADY 1\n\n");

	processUnblocked = 1;

	#ifdef DEBUGMODE
	write(STDOUT_FILENO, message, strlen(message)-1);
	#endif

	return;

}

void forcedExitHandler(int signo) {

	interrupted = 1;

	return;
}
