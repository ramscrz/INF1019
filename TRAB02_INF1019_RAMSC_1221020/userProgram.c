/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo userProgram.c
 * PUC-Rio
 */

#include<stdio.h>
#include<unistd.h>
#include<wait.h>
#include"globalsettings.h"
#include"Page.h"
#include"VM.h"
#include"Semaphore.h"
#include"PageFrameTable.h"
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>

#define MAX_NAME_SIZE 80

volatile sig_atomic_t pageFaultSem;
volatile sig_atomic_t blocked;
volatile sig_atomic_t lostPage;
static PageTable *pageTable;
 
int processIndex;

void pageFaultFeedbackHandler(int signo);
void lostPageFrameHandler(int signo);
void pageAllocationTerminatedHandler(int signo);

int main(int argc, char **argv) {

	PageFrameTable * pageFrameTable = NULL;
	int pidGM = 0, sharedMemSgmt = 0, u = 0;
	char processName[MAX_NAME_SIZE], vAddressIndex[5], vOffset[5], vMode = 'R';
	FILE* processFile = NULL;

	int readLines = 0, size = 0, unblockedSem = 0;

	pageTable = NULL;
	processIndex = 0;

	printf("\nUser Process 0%s, reads from file (.log) %s", argv[argc - 3], argv[0]);

	#ifdef DEBUGMODE
	printf("\n\n(User Process 0%s) Number of PARAMETERS %d\n\n", argv[argc - 3], argc);
	#endif

	if(argc != 5) {
		printf("\nThe process did not receive enough parameters to be executed.\n");
		exit(-1);
	}

	if(strcpy(processName, argv[argc - 5]) == NULL) {
		printf("\nFailed to copy the name of the process 0%s.\n", argv[argc - 3]);
	}

	pidGM = strtol(argv[argc - 4], NULL, 10);
	processIndex = strtol(argv[argc - 3], NULL, 10);
	sharedMemSgmt = strtol(argv[argc - 2], NULL, 10);
	unblockedSem = strtol(argv[argc - 1], NULL, 10);

	signal(SIGCONT, pageFaultFeedbackHandler);
	signal(SIGUSR2, lostPageFrameHandler);
	signal(SIGUSR1, pageAllocationTerminatedHandler);

	pageTable = getPageTable(sharedMemSgmt);
	if(pageTable == NULL) {
		printf("\nERROR: process %s could not retrieve a Page Table.\n", processName);
	}

	size = strlen(processName);

	if(size > MAX_NAME_SIZE - 4) {
		kill(pidGM, SIGUSR1);
		exit(-1);
	}

	processName[size - 1] = '.';
	processName[size] = 'l';
	processName[size + 1] = 'o';
	processName[size + 2] = 'g';
	processName[size + 3] = '\0';

	processFile = fopen(processName, "r");
	if(processFile == NULL) {
		printf("\nFile %s could not be opened.\n", processName);
		exit(-1);
	}

	while(fscanf(processFile, "%4s", vAddressIndex) != EOF) {

		if(fscanf(processFile, "%s", vOffset)) {

			if(fscanf(processFile, " %c", &vMode)) {

				PAGE_INDEX pageIndex = (int)strtol(vAddressIndex, NULL, 16);
				OFFSET offset = (OFFSET)strtol(vOffset, NULL, 16);
				ACCESS_MODE mode = ACCESSMODE_R;

				readLines++;

				#ifdef DEBUGMODE
				printf("\nPROCESS 0%s LINE #%d pageIndex: %s <%d %x> ; offset: %s <%d> ; ACCESSMODE: %c <%d>\n",
 								argv[2], readLines, vAddressIndex, pageIndex, pageIndex, vOffset, offset, vMode, mode);
				#endif

				if(vMode == 'W' || vMode == 'w') {
					mode = ACCESSMODE_W;
				} else if(vMode != 'r' && vMode != 'R') {
					printf("\nERROR: invalid mode %c read from log file of process %s.\n", vMode, processName);
					exit(-1);
				}


				int transRet = trans(pageTable, pageIndex, offset, mode);

				if(transRet == -3) {

					if(setWaitingPageProcess(pageTable, mode) > 0) {

						#ifdef DEBUGMODE
						printf("\nREADY TO SEND SIGUSR1 P%s %d %d\n", argv[2], getpid(), getppid());
						#endif

						blocked = 0;
						#ifdef VMGMDETAIL
						printf("\nProcess 0%s requires a page (PAGE INDEX <DEC %04d> <HEX %04x>) not framed in physical memory!\n", 
											argv[2], pageIndex, pageIndex);
						#endif

						kill(getppid(), SIGUSR1);

						#ifdef DEBUGMODE
						printf("\nSIGUSR1 SENT P%s\n", argv[2]);
						#endif

						while(blocked == 0);

						#ifdef DEBUGMODE
						printf("\nProcess 0%s unblocked\n", argv[2]);
						#endif

						printPageTable(pageTable);

						if(transAfterPageFault(pageTable, pageIndex, offset, mode) < 0) {
							printf("\ntransAfterPageFault could not read page for user process!\n");
						}

						#ifdef DEBUGMODE
						printf("\nCONFIRM UNBLOCKED %s\n", argv[2]);
						#endif
//						kill(getppid(), SIGUSR2);

					} else {
						printf("\nERROR: Blocked Process Id could not be set!\n");
					}

				} else if(transRet == 1) {
					updatePageFrameCount(pageTable, pageIndex);
				}

			} else {
				printf("\nERROR: Process 0%d can't read invalid file %s.\n", processIndex, processName);
			}
		} else {
			printf("\nERROR: Process 0%d can't read invalid file %s.\n", processIndex, processName);
		}
	}

	dropPageTable(pageTable, getppid());

	return 0;
}

void pageAllocationTerminatedHandler(int signo) {

	blocked = 1;

}

void pageFaultFeedbackHandler(int signo) {

	blocked = 1;

}

void lostPageFrameHandler(int signo) {

	printf("\n(LFU) Page Process 0%d lost a page (PAGE INDEX: %x) allocated in page frame %d!\n", processIndex, getLostPageIndex(pageTable), getLostPageFrameIndex(pageTable));

}

