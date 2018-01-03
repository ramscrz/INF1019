/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo VM.c
 * PUC-Rio
 */

#include"VM.h"
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include"Semaphore.h"
#include"PageFrameTable.h"
#include<stdio.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>

typedef struct page {

	MODIFIED_CONTROL_BIT modified;
	FRAMED_CONTROL_BIT framed;
	PAGE_FRAME_INDEX allocatedPageFrame; 
	PAGE_INDEX pageIndex;
	NEW_CONTROL_BIT newPage;
	//unsigned int timeWindowFrequency[TIMEWINDOW_MILLISECS];
} Page;

typedef struct pageTable {
	PAGE_INDEX lastAllocatedPageIndex;
	PAGE_FRAME_INDEX lastAllocatedPageFrameIndex;
	PAGE_FRAME_INDEX lastLostPageFrameIndex;
	PAGE_INDEX lastLostPageIndex;
	PAGE_INDEX lastRequestedPageIndex;
	unsigned int waitingForPageRequest;
	MODIFIED_CONTROL_BIT waitingPageAccessMode;
	pid_t ownerProcessPid;
	unsigned int processId;
	int sharedMemSegment;
	int pageFrameTableSharedMemSegmt;
	int semaphoreId;
	Page page[NUMBER_PAGE_FRAMES];
	
} PageTable;

PageFrameTable* initVM() {

	unsigned int u = 0;

	return initPageFrameTable();

}

int dropPageTable(PageTable *pageTable, pid_t parentProcess) {

	if(pageTable == NULL) {
		return -1;
	}

	printf("\n\tPage Table dropped by process0%d PID %d\n", pageTable->processId, pageTable->ownerProcessPid);

	pageTable->ownerProcessPid = parentProcess;

	return 1;

}

int getLastRequestedPageIndex(PageTable* pageTable, PAGE_INDEX* pageIndex) {

	if(pageTable == NULL) {
		return -1;
	}

	*pageIndex = pageTable->lastRequestedPageIndex;

	return 1;
}

int getPageTableSegment(PageTable* pageTable) {

	if(pageTable == NULL) {
		-1;
	}

	return pageTable->sharedMemSegment;

}

PAGE_INDEX getLostPageIndex(PageTable* pageTable) {

	if(pageTable == NULL) {
		-1;
	}

	return pageTable->lastLostPageIndex;
}

PAGE_FRAME_INDEX getLostPageFrameIndex(PageTable* pageTable) {

	if(pageTable == NULL) {
		-1;
	}

	return pageTable->lastLostPageFrameIndex;
}

int getPageFrameTableSegment(PageTable* pageTable) {

	if(pageTable == NULL) {
		return -1;	
	}

	return pageTable->pageFrameTableSharedMemSegmt;

}

int getPid(PageTable* pageTable) {

	if(pageTable == NULL) {
		return -1;	
	}

	return pageTable->ownerProcessPid;

}

// GM calls this function.
PageTable* getPageTable(int pageTableSegment) {

	PageTable *pageTable = NULL;

	if(pageTableSegment < 0) {

		printf("\n\nERROR: invalid shared memory segment for page request.\n\n");

		return NULL;
	}
		
	pageTable = (PageTable*) shmat(pageTableSegment, 0, 0);

	if(pageTable == NULL) {

		printf("\n\nERROR: shmat could not get shared memory address of a PageTable.\n\n");

		return NULL;
	}

	return pageTable;
	
}

PageFrameTable* getPageFrameTable(PageTable *pageTable) {

	PageFrameTable *pageFrameTable = NULL;

	if(pageTable == NULL) {
		printf("\nERROR: PageTable is NULL-Pointer.\n");
		exit(-1);
	}

	if(pageTable->pageFrameTableSharedMemSegmt < 0) {

		printf("\n\nERROR: invalid shared memory segment for page frame request.\n\n");

		return NULL;
	}
		
	pageFrameTable = (PageFrameTable*) shmat(pageTable->pageFrameTableSharedMemSegmt, 0, 0);

	if(pageFrameTable == NULL) {

		printf("\n\nERROR: shmat could not get shared memory address of a PageFrameTable.\n\n");

		return NULL;
	}

	return pageFrameTable;
	
}

int assignPageTable(PageTable *pageTable, pid_t ownerProcessPid) {

	if(pageTable == NULL) {
		return -1;
	}

	pageTable->ownerProcessPid = ownerProcessPid;

	printf("\nPage Table assigned to process PID %d", pageTable->ownerProcessPid);

	return 1;
}

// User Process calls this function;
PageTable* createPageTable(unsigned int processId, pid_t ownerProcessPid, PageFrameTable* pageFrameTable) {

	unsigned int u = 0, t = 0;

	PageTable* pageTable = NULL;
	int pageTableSegment = -1;

	if(pageFrameTable == NULL) {
		printf("\nERROR: createPageTable received a NULL PageFrameTable.\n");
		return NULL;
	}

	pageTableSegment = shmget(IPC_PRIVATE, sizeof(PageTable), IPC_CREAT | S_IRUSR | S_IWUSR);

	if(pageTableSegment < 0) {

		pageTableSegment = shmget(IPC_PRIVATE, sizeof(PageTable), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		
		if(pageTableSegment < 0) {
			printf("\n\nERROR: shmget could not allocate a page table (errno: %s).\n\n", strerror(errno));
			return NULL;
		}
	}
		
	pageTable = (PageTable*) shmat(pageTableSegment, 0, 0);

	if(pageTable == NULL) {
		printf("\n\nERROR: shmat could not get shared memory address of a PageTable.\n\n");
		return NULL;
	}

	shmctl(pageTableSegment, IPC_RMID, 0);

	pageTable->ownerProcessPid = ownerProcessPid;
	pageTable->sharedMemSegment = pageTableSegment;
	pageTable->lastRequestedPageIndex = 0;
	pageTable->semaphoreId = setupSemaphore();
	pageTable->pageFrameTableSharedMemSegmt = getSharedMemSegment(pageFrameTable);
	pageTable->processId = processId;
	pageTable->lastLostPageFrameIndex = 0;
	pageTable->lastLostPageIndex = 0;
	pageTable->lastAllocatedPageIndex = 0;
	pageTable->lastAllocatedPageFrameIndex = 0;
	pageTable->waitingForPageRequest = 0;

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		pageTable->page[u].modified = PAGE_NOT_MODIFIED;
		pageTable->page[u].framed = PAGE_NOT_FRAMED;
		pageTable->page[u].allocatedPageFrame = u;
		pageTable->page[u].pageIndex = 0;
		pageTable->page[u].newPage = PAGE_NOT_NEW;

	}

	return pageTable;
}

int getWaitingProcess(PageTable *pageTable) {
	
	if(pageTable == NULL) {
		return -1;
	}
	
	return pageTable->waitingForPageRequest;

}

int unsetWaitingProcess(PageTable *pageTable) {

	if(pageTable == NULL) {
		return -1;
	}
	
	pageTable->waitingForPageRequest = 0;

}

int getWaitingPageAccessMode(PageTable *pageTable, MODIFIED_CONTROL_BIT* mode) {
	
	if(pageTable == NULL || mode == NULL) {
		return -1;
	}
	
	*mode = pageTable->waitingPageAccessMode;

	return 1;

}

int setWaitingPageProcess(PageTable *pageTable, ACCESS_MODE mode) {

	if(pageTable == NULL) {
		return -1;
	}

	pageTable->waitingForPageRequest = 1;

	if(mode == ACCESSMODE_W) {
		pageTable->waitingPageAccessMode = PAGE_MODIFIED;
	} else {
		pageTable->waitingPageAccessMode = PAGE_NOT_MODIFIED;
	}

	return 1;

}

int isPageModified(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, MODIFIED_CONTROL_BIT *modified) {

	if(pageTable == NULL || modified == NULL) {
		return -1;
	}

	*modified = pageTable->page[pageFrameIndex].modified;

	return 1;
}

int isPageFramed(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, FRAMED_CONTROL_BIT *framed) {

	if(pageTable == NULL || framed == NULL) {
		return -1;
	}

	*framed = pageTable->page[pageFrameIndex].framed;

	return 1;
}

int isPageNew(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, NEW_CONTROL_BIT *newPage) {

	if(pageTable == NULL || newPage == NULL) {
		return -1;
	}

	*newPage = pageTable->page[pageFrameIndex].newPage;

	return 1;
}

int setPageModified(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, MODIFIED_CONTROL_BIT newStatus) {

	if(pageTable == NULL) {
		return -1;
	}

	pageTable->page[pageFrameIndex].modified = newStatus;

	return 1;

}

int setPageFramed(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, FRAMED_CONTROL_BIT newStatus) {

	if(pageTable == NULL) {
		return -1;
	}

	pageTable->page[pageFrameIndex].framed = newStatus;

	return 1;
}

int setPageNew(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, NEW_CONTROL_BIT newPage) {

	if(pageTable == NULL) {
		return -1;
	}

	pageTable->page[pageFrameIndex].newPage = newPage;

	return 1;
}

int searchPageFrameByIndex(PageTable* pageTable, PAGE_INDEX pageIndex, PAGE_FRAME_INDEX *pageFrameIndex) {

	int u = 0;

	if(pageTable == NULL) {
		return -1;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		if(pageTable->page[u].pageIndex == pageIndex && pageTable->page[u].framed == PAGE_FRAMED) {
			*pageFrameIndex = u;
			return 1;
		}

	}

	return -1;

}

int updatePageTable(PageTable** pageTables, PageFrameTable* pageFrameTable, unsigned int processId, unsigned int processPid) {

	PAGE_FRAME_INDEX pFIndex = 0;
	PAGE_INDEX newPageIndex = 0;
	MODIFIED_CONTROL_BIT modified = PAGE_NOT_MODIFIED;

	int previousProcessPid = -1, previousProcessId = -1;
	struct timeval now;
	int semId = 0;

	#ifdef DEBUGMODE
	printf("\n\nUPDATE 0\n\n");
	#endif

	#ifdef DEBUGMODE
	printf("\n\nUPDATE 1\n\n");
	#endif

	if(getLastRequestedPageIndex(pageTables[processId], &newPageIndex) < 0) {
		#ifdef DEBUGMODE
		printf("\n\nUPDATE 1.1\n\n");
		#endif
		return -1;
	}

	#ifdef DEBUGMODE
	printf("\n\nUPDATE 2\n\n");
	#endif

	getWaitingPageAccessMode(pageTables[processId], &modified);

	#ifdef DEBUGMODE
	printf("\n\nUPDATE 2.2\n\n");
	#endif

	for(pFIndex = 0; pFIndex < NUMBER_PAGE_FRAMES; pFIndex++) {

		#ifdef DEBUGMODE
		printf("\n\nUPDATE 2 LOOP\n\n");
		#endif

		if(getPageFrameNextProcessPid(pageFrameTable, pFIndex) == processPid) {

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 2.3\n\n");
			#endif

			previousProcessPid = getPageFrameProcessPid(pageFrameTable, pFIndex, &previousProcessId);

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 2.4 %d\n\n", previousProcessId);
			#endif

			if(previousProcessId == -1) {


				#ifdef DEBUGMODE
				printf("\n\nUPDATE 4E\n\n");
				#endif

				framePage(pageTables[processId], newPageIndex, pageTables[processId]->ownerProcessPid, pFIndex, processId);

				#ifdef DEBUGMODE
				printf("\n\nUPDATE 5E\n\n");
				#endif

				#ifdef VMGMDETAIL
				printf("\nPage <Page Index %04d> of Process 0%d has been allocated in frame <Frame Index %d>!\n", newPageIndex, pageTables[processId]->ownerProcessPid, pFIndex);
				#endif
							
				pageTables[processId]->page[pFIndex].newPage = PAGE_NEW;
				pageTables[processId]->page[pFIndex].modified = modified;
				pageTables[processId]->page[pFIndex].pageIndex = newPageIndex;
				pageTables[processId]->page[pFIndex].framed = PAGE_FRAMED;
				pageTables[processId]->lastAllocatedPageFrameIndex = pFIndex;
				pageTables[processId]->lastAllocatedPageIndex = newPageIndex;

				#ifdef DEBUGMODE
				printf("\n\nUPDATE 6\n\n");
				printf("\n\nUPDATE 7\n\n");
				#endif

				return previousProcessPid;

			}

			#ifdef DEBUGMODE
			if(pageTables[previousProcessId]->page[pFIndex].allocatedPageFrame == pFIndex) 
				printf("\n\nM 1\n\n");
			if(pageTables[previousProcessId]->page[pFIndex].framed == PAGE_FRAMED) 
				printf("\n\nM 1\n\n");
			#endif

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 2.5\n\n");
			#endif
			if(pageTables[previousProcessId]->page[pFIndex].allocatedPageFrame == pFIndex
						&& pageTables[previousProcessId]->page[pFIndex].framed == PAGE_FRAMED) {

				#ifdef DEBUGMODE
				printf("\n\nUPDATE 2.6\n\n");
				#endif

				pageTables[previousProcessId]->page[pFIndex].framed = PAGE_NOT_FRAMED;
				pageTables[previousProcessId]->lastLostPageFrameIndex = pFIndex;
				pageTables[previousProcessId]->lastLostPageIndex = pageTables[previousProcessId]->page[pFIndex].pageIndex;

				pageTables[previousProcessId]->page[pFIndex].framed = PAGE_NOT_FRAMED;
				pageTables[previousProcessId]->page[pFIndex].modified = PAGE_NOT_MODIFIED;
				pageTables[previousProcessId]->page[pFIndex].newPage = PAGE_NOT_NEW;

				unframePage(pageTables[previousProcessId], pFIndex);
			}

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 4\n\n");
			#endif

			framePage(pageTables[processId], newPageIndex, pageTables[processId]->ownerProcessPid, pFIndex, processId);

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 5\n\n");
			#endif

			#ifdef VMGMDETAIL
			printf("\nPage <Page Index %04d> of Process 0%d has been allocated in frame <Frame Index %d>!\n", newPageIndex, pageTables[processId]->ownerProcessPid, pFIndex);
			#endif
							
			pageTables[processId]->page[pFIndex].newPage = PAGE_NEW;
			pageTables[processId]->page[pFIndex].modified = modified;
			pageTables[processId]->page[pFIndex].pageIndex = newPageIndex;
			pageTables[processId]->page[pFIndex].framed = PAGE_FRAMED;
			pageTables[processId]->lastAllocatedPageFrameIndex = pFIndex;
			pageTables[processId]->lastAllocatedPageIndex = newPageIndex;

			#ifdef DEBUGMODE
			printf("\n\nUPDATE 6\n\n");
			printf("\n\nUPDATE 7\n\n");
			#endif

			return previousProcessPid;

		}
		
	}

	return -1;
}

void printPageTable(PageTable * pageTable) {
	
	unsigned int u = 0;

	#ifdef PRINTPAGETABLE
	printf("\n\n\t======= PageTable =======\n");
	printf("\n\tOwner Process PID: %d\n", pageTable->ownerProcessPid);
	printf("\n\tLast Requested Page Index: %d\n", pageTable->lastRequestedPageIndex);
	printf("\n\tOwner Process ID: %d\n\n", pageTable->processId);

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		if(pageTable->page[u].framed == PAGE_FRAMED) {
			printf("\n---------------------------------------------------------------------");
			printf("\n\t\tModified? %d", pageTable->page[u].modified);
			printf("\n\t\tFramed? %d", pageTable->page[u].framed);
			printf("\n\t\tAllocated Page Frame: %d", pageTable->page[u].allocatedPageFrame);
			printf("\n\t\tPage Index: %d", pageTable->page[u].pageIndex);
			printf("\n\t\tIs this a newly framed page? %d", pageTable->page[u].newPage);
			printf("\n---------------------------------------------------------------------\n\n");
		}
	}
	#endif

	return;

}

// The user process waits 1 second, then it verifies the MODIFIED BIT. If it's set to MODIFIED,
// then the user process waits 1 more second before calling validadeNewPage.

// This function must be called by the user process after pageFault has been solved by GM.
int validateNewPage(PageTable *pageTable, PAGE_INDEX pageIndex, ACCESS_MODE mode, OFFSET offset) {

	PAGE_FRAME_INDEX pageFrameIndex = 0;
	PageFrameTable *pageFrameTable = NULL;
	int u = 0;

	if(pageTable == NULL) {
		return -1;
	}

	if(searchPageFrameByIndex(pageTable, pageIndex, &pageFrameIndex) < 0) {
		return -1;
	}

	pageTable->page[pageFrameIndex].pageIndex = pageIndex;
	pageTable->page[pageFrameIndex].allocatedPageFrame = pageFrameIndex;

	if(mode == ACCESSMODE_W) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, w\n", pageTable->processId, pageFrameIndex, offset, pageFrameIndex, offset);
		pageTable->page[pageFrameIndex].modified = PAGE_MODIFIED;
	} else if(mode == ACCESSMODE_R) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, r\n", pageTable->processId, pageFrameIndex, offset, pageFrameIndex, offset);
		pageTable->page[pageFrameIndex].modified = PAGE_NOT_MODIFIED;
	} else {

		printf("\nERROR: Invalid ACCESS MODE value found.\n");
		printf("\nP%d, %d%d, INVALID_ACCESS_MODE", 	pageTable->ownerProcessPid, 
														pageTable->page[pageFrameIndex].pageIndex, 
														offset);
	}

	pageTable->page[pageFrameIndex].framed = PAGE_FRAMED;
	pageTable->page[pageFrameIndex].newPage = PAGE_NOT_NEW;

	return 1;

}

// Retorno -1 indica que um sinal SIGUSR1 precisa ser enviado ao GM.
int trans(PageTable* pageTable, PAGE_INDEX pageIndex, OFFSET offset, ACCESS_MODE mode) {

	PAGE_FRAME_INDEX pageFrameIndex = 0;

	#ifdef DEBUGMODE
	printf("\nTRANS 0\n");
	#endif

	if(pageTable == NULL) {
		return -1;
	}

	#ifdef DEBUGMODE
	printf("\nTRANS 1\n");
	#endif

	#ifdef DEBUGMODE
	printf("\nTRANS 2\n");
	#endif

	if(pageTable == NULL) {
		return -2;
	}

	#ifdef DEBUGMODE
	printf("\nTRANS 3\n");
	#endif

	pageTable->lastRequestedPageIndex = pageIndex;

	#ifdef DEBUGMODE
	printf("\nTRANS 4\n");
	#endif

	if(searchPageFrameByIndex(pageTable, pageIndex, &pageFrameIndex) < 0) {
		#ifdef DEBUGMODE
		printf("\nTRANS 5\n");
		#endif
		
		return -3;
	}

	#ifdef DEBUGMODE
	printf("\nTRANS 6\n");
	#endif

	if(pageTable->page[pageFrameIndex].newPage == PAGE_NEW) {
		pageTable->page[pageFrameIndex].newPage = PAGE_NOT_NEW;
	}

	if(mode == ACCESSMODE_W) {
		pageTable->page[pageFrameIndex].modified = PAGE_MODIFIED;
	}

	#ifdef DEBUGMODE
	printf("\nTRANS 7\n");
	#endif

	if(mode == ACCESSMODE_W) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, w\n", pageTable->processId, pageFrameIndex, offset, pageFrameIndex, offset);
	} else if(mode == ACCESSMODE_R) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, r\n", pageTable->processId, pageFrameIndex, offset, pageFrameIndex, offset);
	}

	#ifdef DEBUGMODE
	printf("\nTRANS 8\n");

	printf("\nTRANS 9\n");
	#endif

	return 1;

}

int transAfterPageFault(PageTable* pageTable, PAGE_INDEX pageIndex, OFFSET offset, ACCESS_MODE mode) {

	if(pageTable == NULL) {
		return -1;
	}

	if(mode == ACCESSMODE_W) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, w\n", pageTable->processId, pageTable->lastAllocatedPageFrameIndex, offset, pageTable->lastAllocatedPageFrameIndex, offset);
	} else if(mode == ACCESSMODE_R) {
		printf("\nP%d, <HEX %04x%04x> <DEC %04d %04d>, r\n", pageTable->processId, pageTable->lastAllocatedPageFrameIndex, offset, pageTable->lastAllocatedPageFrameIndex, offset);
	}

	return 1;
}

int getElapsedMilliseconds(suseconds_t start, suseconds_t* interval) {

	struct timeval now;

	if(gettimeofday(&now, NULL) == 0) {

		*interval = (now.tv_usec - start) / 1000;

		return 1;
	}

	printf("\nERROR: getElapsedMilliseconds => %s\n", strerror(errno));

	return -1;

}

int unlockPageTable(PageTable *pageTable) {

	if(pageTable == NULL) {
		return -1;
	}

	semaforoV(pageTable->semaphoreId);
	
	return 1;
}

int lockPageTable(PageTable *pageTable) {

	if(pageTable == NULL) {
		return -1;
	}

	semaforoP(pageTable->semaphoreId);
	
	return 1;
}

int lockRequestPage(PageTable *pageTable) {

	PageFrameTable * pageFrameTable = NULL;

	if(pageTable == NULL) {
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	lockPageFrameTableAccess(pageFrameTable);

	semaforoP(pageTable->semaphoreId);
	if(pageTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	shmdt(pageFrameTable);

	return 1;
}

int unlockRequestPage(PageTable *pageTable) {

	PageFrameTable * pageFrameTable = NULL;

	if(pageTable == NULL) {
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	unlockPageFrameTableAccess(pageFrameTable);

	semaforoV(pageTable->semaphoreId);
	if(pageTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	shmdt(pageFrameTable);

	return 1;
}

int removeProcessFromPageFrame(PageTable *pageTable) {

	unsigned int u = 0;
	PageFrameTable* pageFrameTable = NULL;

	if(pageTable == NULL) {
		return -1;
	}

	semaforoP(pageTable->semaphoreId);

	if(pageTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	pageFrameTable = getPageFrameTable(pageTable);

	if(pageFrameTable == NULL) {
		semaforoV(pageTable->semaphoreId);
		return -1;
	}

	for(u = 0; u < NUMBER_PAGE_FRAMES; u++) {

		if(pageTable->page[u].framed == PAGE_FRAMED) {
			unframePage(pageTable, pageTable->page[u].allocatedPageFrame);
		}

		pageTable->page[u].framed = PAGE_NOT_FRAMED;

	}

	semaforoV(pageTable->semaphoreId);

	return 1;

}

int erasePageTable(PageTable *pageTable) {

	if(pageTable == NULL) {
		return -1;
	}

	semaforoP(pageTable->semaphoreId);

	delSemValue(pageTable->semaphoreId);

	shmdt(pageTable);

	return 1;

}
