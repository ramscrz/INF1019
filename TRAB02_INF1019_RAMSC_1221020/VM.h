/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo VM.h
 * PUC-Rio
 */

#ifndef VM_H
#define VM_H

#include"Page.h"
#include"globalsettings.h"
#include<sys/types.h>

PageFrameTable* initVM();

int getPid(PageTable* pageTable);

int getLastRequestedPageIndex(PageTable* pageTable, PAGE_INDEX* pageIndex);

int getPageTableSegment(PageTable* pageTable);

int getPageFrameTableSegment(PageTable* pageTable);

// GM calls this function.
PageTable* getPageTable(int pageTableSegment);

PageFrameTable* getPageFrameTable(PageTable *pageTable);

PAGE_INDEX getLostPageIndex(PageTable* pageTable);

PAGE_FRAME_INDEX getLostPageFrameIndex(PageTable* pageTable);

int assignPageTable(PageTable *pageTable, pid_t ownerProcessPid);

int dropPageTable(PageTable *pageTable, pid_t parentProcess);

void printPageTable(PageTable * pageTable);

// User Process calls this function;
PageTable* createPageTable(unsigned int processId, pid_t ownerProcessPid, PageFrameTable* pageFrameTable);

int unlockPageTable(PageTable *pageTable);

int lockPageTable(PageTable *pageTable);

int isPageModified(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, MODIFIED_CONTROL_BIT *modified);

int unsetWaitingProcess(PageTable *pageTable);

int isPageFramed(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, FRAMED_CONTROL_BIT *framed);

int isPageNew(PageTable* pageTable, PAGE_FRAME_INDEX pageFrameIndex, NEW_CONTROL_BIT *newPage);

int setWaitingPageProcess(PageTable *pageTable, ACCESS_MODE mode);

int getWaitingPageAccessMode(PageTable *pageTable, MODIFIED_CONTROL_BIT* mode);

int getWaitingProcess(PageTable *pageTable);

int setPageModified(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, MODIFIED_CONTROL_BIT newStatus);

int setPageFramed(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, FRAMED_CONTROL_BIT newStatus);

int setPageNew(PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, NEW_CONTROL_BIT newPage);

int searchPageFrameByIndex(PageTable* pageTable, PAGE_INDEX pageIndex, PAGE_FRAME_INDEX *pageFrameIndex);

int updatePageTable(PageTable** pageTables, PageFrameTable* pageFrameTable, unsigned int processId, unsigned int processPid);

int lockRequestPage(PageTable *pageTable);

int unlockRequestPage(PageTable *pageTable); 

// Deprecated.
int validateNewPage(PageTable *pageTable, PAGE_INDEX pageIndex, ACCESS_MODE mode, OFFSET offset);

// Retorno -1 indica que um sinal SIGUSR1 precisa ser enviado ao GM.
int trans(PageTable* pageTable, PAGE_INDEX pageIndex, OFFSET offset, ACCESS_MODE mode);

int transAfterPageFault(PageTable* pageTable, PAGE_INDEX pageIndex, OFFSET offset, ACCESS_MODE mode);

int getElapsedMilliseconds(suseconds_t start, suseconds_t* interval);

int removeProcessFromPageFrame(PageTable *pageTable);

int erasePageTable(PageTable *pageTable);

#endif
