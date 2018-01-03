/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo PageFrameTable.h
 * PUC-Rio
 */

#include<signal.h>
#include <sys/types.h>
#include <unistd.h>
#include"PageFrameTable.h"
#include"Semaphore.h"
#include"VM.h"
#include"Page.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<errno.h>

typedef struct pageFrame {

	pid_t processPid;
	PAGE_INDEX pageIndex;
	FRAMED_CONTROL_BIT framed;
	int processOwnerId;
	int nextProcessPid;
	unsigned int timeWindowAccum;
	unsigned int timeWindowFrequency[TIMEWINDOW];
} PageFrame;

struct pageFrameTable {
	unsigned int waitingPageProcessId;
	MODIFIED_CONTROL_BIT waitingPageModifiedMode;
	PageFrame pageFrames[NUMBER_PAGE_FRAMES];
	int sharedMemSegment;
	int semaphoreId, semaphoreDeadId, accessCountSemId;
	unsigned int timeCounter;
};

PageFrameTable* initPageFrameTable() {
	
	unsigned int u = 0, k = 0;

	PageFrameTable* pageFrameTable = NULL;
	int pageFrameTableSegment = -1;

	pageFrameTableSegment = shmget(IPC_PRIVATE, sizeof(PageFrameTable), IPC_CREAT | S_IRUSR | S_IWUSR);

	if(pageFrameTableSegment < 0) {

		shmdt(&pageFrameTableSegment);
		shmctl(pageFrameTableSegment, IPC_RMID, 0);

		pageFrameTableSegment = shmget(IPC_PRIVATE, sizeof(PageFrameTable), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		
		if(pageFrameTableSegment < 0) {

			shmdt(&pageFrameTableSegment);
			shmctl(pageFrameTableSegment, IPC_RMID, 0);

			printf("\n\nERROR: shmget could not allocate a page Frame table (errno: %s).\n\n", strerror(errno));

			return NULL;
		}
	}
		
	pageFrameTable = (PageFrameTable*) shmat(pageFrameTableSegment, 0, 0);

	if(pageFrameTable == NULL) {

		shmctl(pageFrameTableSegment, IPC_RMID, 0);
		shmdt(&pageFrameTableSegment);

		printf("\n\nERROR: shmat could not get shared memory address of a PageFrameTable.\n\n");

		return NULL;
	}

	shmctl(pageFrameTableSegment, IPC_RMID, 0);

	pageFrameTable->sharedMemSegment = pageFrameTableSegment;
	pageFrameTable->semaphoreId = setupSemaphore();
	pageFrameTable->semaphoreDeadId = setupSemaphore();
	pageFrameTable->accessCountSemId = setupSemaphore();
	pageFrameTable->waitingPageProcessId = 0;
	pageFrameTable->waitingPageModifiedMode = PAGE_MODIFIED;
	pageFrameTable->timeCounter = 0;

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		pageFrameTable->pageFrames[u].processPid = 0; 
		pageFrameTable->pageFrames[u].pageIndex = 0;
		pageFrameTable->pageFrames[u].timeWindowAccum = 0;
		pageFrameTable->pageFrames[u].framed = PAGE_NOT_FRAMED;
		pageFrameTable->pageFrames[u].timeWindowAccum = 0;
		pageFrameTable->pageFrames[u].processOwnerId = -1;
		pageFrameTable->pageFrames[u].nextProcessPid = -1;

		for(k = 0; k < TIMEWINDOW; k++) {
			pageFrameTable->pageFrames[u].timeWindowFrequency[k] = 0;
		}
	}

	return pageFrameTable; 

}

int getTimeCounter(PageFrameTable *pageFrameTable, suseconds_t *frameTimeBase) {

	if(pageFrameTable == NULL || frameTimeBase == NULL) {
		return -1;
	}

	*frameTimeBase = pageFrameTable->timeCounter;

	return 1;
}

void lockCountAccess(PageFrameTable *pageFrameTable) {

	int pageFrameTSegmt = 0;

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoP(pageFrameTable->accessCountSemId);

	return;
}

void unlockCountAccess(PageFrameTable *pageFrameTable) {

	int pageFrameTSegmt = 0;

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoV(pageFrameTable->accessCountSemId);

	return;
}

void lockDeadProcess(PageTable *pageTable) {

	PageFrameTable *pageFrameTable = NULL;
	int pageFrameTSegmt = 0;

	if(pageTable == NULL) {
		printf("\nCould not lock page frame table because page table is NULL.\n");
		return;
	}

	pageFrameTSegmt = getPageFrameTableSegment(pageTable);

	if(pageFrameTSegmt < 0) {
		printf("\nInvalid Shared Memory Segment retrieved in unlockPageFrameTableAccess.\n");
		return;
	}

	pageFrameTable = (PageFrameTable*) shmat(pageFrameTSegmt, 0, 0);

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoP(pageFrameTable->semaphoreDeadId);

	return;
}

void lockPageFrameTableAccessFromPageTable(PageTable *pageTable) {

	PageFrameTable *pageFrameTable = NULL;
	int pageFrameTSegmt = 0;

	if(pageTable == NULL) {
		printf("\nCould not lock page frame table because page table is NULL.\n");
		return;
	}

	pageFrameTSegmt = getPageFrameTableSegment(pageTable);

	if(pageFrameTSegmt < 0) {
		printf("\nInvalid Shared Memory Segment retrieved in unlockPageFrameTableAccess.\n");
		return;
	}

	pageFrameTable = (PageFrameTable*) shmat(pageFrameTSegmt, 0, 0);

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoP(pageFrameTable->semaphoreId);

	return;
}

void lockPageFrameTableAccess(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoP(pageFrameTable->semaphoreId);

	return;
}

void unlockPageFrameTableAccessFromPageTable(PageTable *pageTable) {

	PageFrameTable *pageFrameTable = NULL;
	int pageFrameTSegmt = 0;

	if(pageTable == NULL) {
		printf("\nCould not lock page frame table because page table is NULL.\n");
		return;
	}

	pageFrameTSegmt = getPageFrameTableSegment(pageTable);

	if(pageFrameTSegmt < 0) {
		printf("\nInvalid Shared Memory Segment retrieved in unlockPageFrameTableAccess.\n");
		return;
	}

	pageFrameTable = (PageFrameTable*) shmat(pageFrameTSegmt, 0, 0);

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoV(pageFrameTable->semaphoreId);

	return;
}

void unlockPageFrameTableAccess(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoV(pageFrameTable->semaphoreId);

	return;
}

void unlockDeadProcess(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		printf("\nCould not lock page frame table because page frame table is NULL.\n");
		return;
	}

	semaforoV(pageFrameTable->semaphoreDeadId);

	return;
}

int getPageFTableSegment(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->sharedMemSegment;
}

PageFrameTable* getPageFTable(PageTable *pageTable) {

	PageFrameTable *pageFrameTable = NULL;
	int pageFrameTSegmt = 0;

	if(pageTable == NULL) {
		return NULL;
	}

	pageFrameTSegmt = getPageFrameTableSegment(pageTable);

	if(pageFrameTSegmt < 0) {
		return NULL;
	}

	pageFrameTable = (PageFrameTable*) shmat(pageFrameTSegmt, 0, 0);

	return pageFrameTable;
}

int getPageFrameTableSemaphoreId(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->semaphoreId;
}

void printFramedPages(PageFrameTable *pageFrameTable) {
	unsigned int u = 0;

	if(pageFrameTable == NULL) {
		return;
	}

	#ifdef PRINTPAGEFRAMETABLE
	printf("\n\n\t======= PageFrameTable =======\n");

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		if(pageFrameTable->pageFrames[u].framed == PAGE_FRAMED) {
			printf("\n-------------------------------");
			printf("\n\t\t[PAGEFRAME] Framed? %d", pageFrameTable->pageFrames[u].framed);
			printf("\n\t\t[PAGEFRAME] Process in Page Frame: %d", pageFrameTable->pageFrames[u].processPid);
			
			printf("\n\t\t[PAGEFRAME] Page Index: %d", pageFrameTable->pageFrames[u].pageIndex);
		}
	}
	#endif

	return;
}

MAP_T searchLFUFramePage(PageTable *pageTable, PageFrameTable* pageFrameTable, pid_t *processPid, PAGE_FRAME_INDEX *pageFrameIndex, PAGE_INDEX *pageIndex, PageTable **pageTables) {

	unsigned int min = 0;
	unsigned int u = 0;

	if(NUMBER_PAGE_FRAMES < 1 || pageFrameTable == NULL || pageTable == NULL || pageTables == NULL) {
		printf("\n\n\n\n\n\ERRO AQUI 1\n\n\n\n\n");
		return MAP_T_ERROR;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		if(pageFrameTable->pageFrames[u].framed == PAGE_NOT_FRAMED && pageFrameTable->pageFrames[u].nextProcessPid < 0) {
			#ifdef DEBUGMODE
			printf("\n\n\nOVERWRITE %d\n\n\n", u);
			#endif
			*pageIndex = pageFrameTable->pageFrames[u].pageIndex;
			*processPid = pageFrameTable->pageFrames[u].processPid;
			*pageFrameIndex = u;
			return MAP_T_OVERWRITE;
		}

	}

	int found = 0;

	min = -1;

	#ifdef DEBUGMODE
	printf("\n\n\n\nFAILED TO OVERWRITE\n\n\n\n");
	#endif

	MODIFIED_CONTROL_BIT notModified = PAGE_MODIFIED;

	for(u = 1; u < NUMBER_PAGE_FRAMES; u++) {
		#ifdef DEBUGMODE
		printf("\n\nSEARCH LFU timeWindow %d FRAMED %d nextPid %d\n\n", pageFrameTable->pageFrames[u].timeWindowAccum, pageFrameTable->pageFrames[u].framed == PAGE_FRAMED, pageFrameTable->pageFrames[u].nextProcessPid);
		#endif
		if((pageFrameTable->pageFrames[u].timeWindowAccum < min || min == -1) && pageFrameTable->pageFrames[u].framed == PAGE_FRAMED && pageFrameTable->pageFrames[u].nextProcessPid < 0) {

			found = 1;
			min = pageFrameTable->pageFrames[u].timeWindowAccum;
			*pageIndex = pageFrameTable->pageFrames[u].pageIndex;
			*processPid = pageFrameTable->pageFrames[u].processPid;
			*pageFrameIndex = u;

			if(isPageModified(pageTables[pageFrameTable->pageFrames[u].processOwnerId], u, &notModified) < 0) {
				notModified = PAGE_MODIFIED;
			}

		} else if(pageFrameTable->pageFrames[u].timeWindowAccum == min && pageFrameTable->pageFrames[u].framed == PAGE_FRAMED && notModified == PAGE_MODIFIED && pageFrameTable->pageFrames[u].nextProcessPid < 0) {

			if(isPageModified(pageTables[pageFrameTable->pageFrames[u].processOwnerId], u, &notModified) < 0) {
				notModified = PAGE_MODIFIED;
			}

			if(notModified == PAGE_NOT_MODIFIED) {
				found = 1;
				*pageIndex = pageFrameTable->pageFrames[u].pageIndex;
				*processPid = pageFrameTable->pageFrames[u].processPid;
				*pageFrameIndex = u;
				notModified = PAGE_NOT_MODIFIED;
			}
		}
	}

	if(found == 0) {
		printf("\n\nERROR: LFU could not select a page to be replaced!\n\n");
		return MAP_T_ERROR;
	}

	return MAP_T_REPLACE;

}

int selectPageFrame(PageFrameTable *pageFrameTable, PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, int processPid) {

	MODIFIED_CONTROL_BIT modified = PAGE_MODIFIED;

	if(pageFrameTable == NULL || processPid < 0) {
		return -1;
	}

	#ifdef DEBUGMODE
	printf("\n\nSELECT PAGE FRAME CALLED %d\n\n", processPid);
	#endif

	pageFrameTable->pageFrames[pageFrameIndex].nextProcessPid = processPid;
	
	if(pageTable == NULL) {
		return 1;
	}

	isPageModified(pageTable, pageFrameIndex, &modified);

	if(modified == PAGE_NOT_MODIFIED) {
		return 1;
	} else {
		return 2;
	}
}

int getPageFrameNextProcessPid(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX pFIndex) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->pageFrames[pFIndex].nextProcessPid;
}

int getPageFrameProcessPid(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX pFIndex, unsigned int *previousProcessId) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	*previousProcessId = pageFrameTable->pageFrames[pFIndex].processOwnerId;

	return pageFrameTable->pageFrames[pFIndex].processPid;
}

int framePage(PageTable* pageTable, PAGE_INDEX pageIndex, pid_t processPid, PAGE_FRAME_INDEX frameIndex, unsigned int processId) {

	PageFrameTable* pageFrameTable = NULL;
	unsigned int u = 0;

	if(frameIndex < 0 || frameIndex >= NUMBER_PAGE_FRAMES) {
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);
	if(pageFrameTable == NULL) {
		return -1;
	}

	#ifdef DEBUGMODE
	printf("\n\n\n\n FRAME PAGE CALLED %d\n\n\n\n", frameIndex);
	#endif

	pageFrameTable->pageFrames[frameIndex].processPid = processPid;
	pageFrameTable->pageFrames[frameIndex].pageIndex = pageIndex;
	pageFrameTable->pageFrames[frameIndex].timeWindowAccum = 0;
	pageFrameTable->pageFrames[frameIndex].framed = PAGE_FRAMED;
	pageFrameTable->pageFrames[frameIndex].processOwnerId = processId;

	#ifdef DEBUGMODE
	printf("\nPROCESS PID %d FRAMED: %d %d %d %d\n", processPid, pageFrameTable->pageFrames[frameIndex].processPid, pageFrameTable->pageFrames[frameIndex].pageIndex, pageFrameTable->pageFrames[frameIndex].timeWindowAccum, pageFrameTable->pageFrames[frameIndex].framed);
	#endif

	for(u = 0; u < TIMEWINDOW; u++) {
		pageFrameTable->pageFrames[frameIndex].timeWindowFrequency[u] = 0;
		pageFrameTable->pageFrames[frameIndex].timeWindowAccum = 0;
	}

	pageFrameTable->pageFrames[frameIndex].nextProcessPid = -1;

	#ifdef PRINTPAGEFRAMETABLE
	printf("\n--------------------- START PAGE FRAME ---------------------\n");
	printFramedPages(pageFrameTable);
	printf("\n--------------------- END PAGE FRAME ---------------------\n");
	#endif

	shmdt(pageFrameTable);

	return 1;

}

int unframePage(PageTable* pageTable, PAGE_FRAME_INDEX frameIndex) {

	PageFrameTable* pageFrameTable = NULL;

	if(frameIndex < 0 || frameIndex >= NUMBER_PAGE_FRAMES || pageTable == NULL) {
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		return -1;
	}

	pageFrameTable->pageFrames[frameIndex].processPid = 0; 
	pageFrameTable->pageFrames[frameIndex].pageIndex = 0;
	pageFrameTable->pageFrames[frameIndex].timeWindowAccum = 0;
	pageFrameTable->pageFrames[frameIndex].framed = PAGE_NOT_FRAMED;
	pageFrameTable->pageFrames[frameIndex].processOwnerId = -1;
	pageFrameTable->pageFrames[frameIndex].processOwnerId = -1;
	pageFrameTable->pageFrames[frameIndex].nextProcessPid = -1;

	shmdt(pageFrameTable);

	return 1;
}

void incrementTimeUnit(PageFrameTable* pageFrameTable) {

	int presence = 0, u = 0;

	if(pageFrameTable == NULL) {
		return;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {
		if(pageFrameTable->pageFrames[u].timeWindowFrequency[pageFrameTable->timeCounter] > 0) {
			presence = 1;
		}
	}

	if(presence == 1) {
		for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

			if(pageFrameTable->timeCounter < TIMEWINDOW - 1) {
				pageFrameTable->pageFrames[u].timeWindowAccum -= pageFrameTable->pageFrames[u].timeWindowFrequency[pageFrameTable->timeCounter+1];
				pageFrameTable->pageFrames[u].timeWindowFrequency[pageFrameTable->timeCounter+1] = 0;
			} else {
				pageFrameTable->pageFrames[u].timeWindowAccum -= pageFrameTable->pageFrames[u].timeWindowFrequency[0];
				pageFrameTable->pageFrames[u].timeWindowFrequency[0] = 0;
			}		
		}

		if(pageFrameTable->timeCounter < TIMEWINDOW - 1) {
			pageFrameTable->timeCounter++;
		} else {
			pageFrameTable->timeCounter = 0;
		}

		//printf("\n\nTime Counter = %d\n\n", pageFrameTable->timeCounter);
	}

	return;
}

int incrementPageAccessCount(PageFrameTable* pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value) {

	if(frameIndex < 0 || frameIndex >= NUMBER_PAGE_FRAMES || pageFrameTable == NULL) {
		return -1;
	}

	if(pageFrameTable->pageFrames[frameIndex].timeWindowAccum + value >= TIMEWINDOW) {
		return -1;
	}

	if(pageFrameTable->pageFrames[frameIndex].timeWindowAccum + value < 0) {
		return -1;
	}

	pageFrameTable->pageFrames[frameIndex].timeWindowAccum += value;

	return 1;

}

int decrementPageAccessCount(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value) {

	if(frameIndex < 0 || frameIndex >= NUMBER_PAGE_FRAMES || pageFrameTable == NULL) {
		return -1;
	}

	if(pageFrameTable->pageFrames[frameIndex].timeWindowAccum - value >= TIMEWINDOW) {
		return -1;
	}

	if(pageFrameTable->pageFrames[frameIndex].timeWindowAccum - value < 0) {
		return -1;
	}

	pageFrameTable->pageFrames[frameIndex].timeWindowAccum -= value;

	return 1;

}

int destroyPageFrameTable(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	semaforoP(pageFrameTable->semaphoreDeadId);
	semaforoP(pageFrameTable->accessCountSemId); 
	semaforoP(pageFrameTable->semaphoreId);

	delSemValue(pageFrameTable->semaphoreDeadId);
	delSemValue(pageFrameTable->accessCountSemId);
	delSemValue(pageFrameTable->semaphoreId);

	return 1;

}

int setPageAccessCount(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value) {

	if(frameIndex < 0 || frameIndex >= NUMBER_PAGE_FRAMES || pageFrameTable == NULL) {
		return -1;
	}

	if(value >= TIMEWINDOW || value < 0) {
		return -1;
	}

	pageFrameTable->pageFrames[frameIndex].timeWindowAccum = value;

	return 1;

}

int getSharedMemSegment(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->sharedMemSegment;
}

int getWaitingPageProcessId(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->waitingPageProcessId;
}

int getWaitingPageModifiedMode(PageFrameTable *pageFrameTable) {

	if(pageFrameTable == NULL) {
		return -1;
	}

	return pageFrameTable->waitingPageModifiedMode;
}


int setWaitingPageProcessId(PageTable *pageTable, unsigned int processId, ACCESS_MODE mode) {

	PageFrameTable *pageFrameTable = NULL;

	if(pageTable == NULL) {
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		return -1;
	}

	pageFrameTable->waitingPageProcessId = processId;

	if(mode == ACCESSMODE_W) {
		pageFrameTable->waitingPageModifiedMode = PAGE_MODIFIED;
	} else {
		pageFrameTable->waitingPageModifiedMode = PAGE_NOT_MODIFIED;
	}

	shmdt(pageFrameTable);

	return 1;

}

void updatePageFrameCount(PageTable* pageTable, PAGE_INDEX pageIndex) {

	PageFrameTable *pageFrameTable = NULL;
	int pageFrameIndex = 0, u = 0;

	if(pageTable == NULL) {
		return;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		return;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {
		if(pageFrameTable->pageFrames[u].pageIndex == pageIndex) {
			pageFrameIndex = u;
		}
	}

	if(pageFrameIndex <= 0) {
		return;
	}

	pageFrameTable->pageFrames[pageFrameIndex].timeWindowAccum++;
	pageFrameTable->pageFrames[pageFrameIndex].timeWindowFrequency[pageFrameTable->timeCounter]++;

	shmdt(pageFrameTable);

	return;
}

// Deprecated;
void updateCountAccess(PageTable* pageTable, PAGE_INDEX pageIndex)/*, NEW_CONTROL_BIT new)*/ {

	PageFrameTable *pageFrameTable = NULL;
	suseconds_t interval = 0;
	int ret = 0, u = 0, k = 0;
	unsigned long int timeLapse = 0, frameTimeBase = 0;
	int pageFrameIndex = 0;

	if(pageTable == NULL) {
		return;
	}

	lockCountAccess(pageFrameTable);

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		unlockCountAccess(pageFrameTable);
		return;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {
		if(pageFrameTable->pageFrames[u].pageIndex == pageIndex) {
			pageFrameIndex = u;
		}
	}

	if(pageFrameIndex <= 0) {
		unlockCountAccess(pageFrameTable);
		return;
	}

	frameTimeBase = pageFrameTable->timeCounter;

	if(frameTimeBase >= 0) {

		if(pageFrameTable->timeCounter < TIMEWINDOW) {
			for(k = 0; k < NUMBER_PAGE_FRAMES; k++) {
				if(k != pageFrameIndex && pageFrameTable->pageFrames[k].nextProcessPid != -1) {
					unsigned int currVal = pageFrameTable->pageFrames[k].timeWindowFrequency[frameTimeBase];
					pageFrameTable->pageFrames[k].timeWindowAccum -= currVal;
					pageFrameTable->pageFrames[k].timeWindowFrequency[frameTimeBase] = 0;
				}
			}

			pageFrameTable->pageFrames[pageFrameIndex].timeWindowFrequency[frameTimeBase] = 1;
			pageFrameTable->pageFrames[pageFrameIndex].timeWindowAccum++;

			if(pageFrameTable->timeCounter >= TIMEWINDOW - 1) {
				pageFrameTable->timeCounter = 0;
			} else {
				pageFrameTable->timeCounter++;
			}

		}
	}

	unlockCountAccess(pageFrameTable);

	return;
}
