/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo PageFrameTable.h
 * PUC-Rio
 */

#ifndef PAGE_FRAME_TABLE_H
#define PAGE_FRAME_TABLE_H

#include "Page.h"
#include"globalsettings.h"
#include<sys/types.h>

typedef enum {

	MAP_T_OVERWRITE,
	MAP_T_REPLACE,
	MAP_T_ERROR

} MAP_T;

PageFrameTable* initPageFrameTable();

int getPageFTableSegment(PageFrameTable *pageFrameTable);

PageFrameTable* getPageFTable(PageTable *pageTable);

MAP_T searchLFUFramePage(PageTable *pageTable, PageFrameTable* pageFrameTable, pid_t *processPid, PAGE_FRAME_INDEX *pageFrameIndex, PAGE_INDEX *pageIndex, PageTable **pageTables);

int framePage(PageTable* pageTable, PAGE_INDEX pageIndex, pid_t processPid, PAGE_FRAME_INDEX frameIndex, unsigned int processId);

void updateCountAccess(PageTable* pageTable, PAGE_INDEX pageIndex);

int getTimeCounter(PageFrameTable *pageFrameTable, time_t *frameTimeBase);

void printFramedPages(PageFrameTable *pageFrameTable);

void lockPageFrameTableAccessFromPageTable(PageTable *pageTable);

void lockPageFrameTableAccess(PageFrameTable *pageFrameTable);

int selectPageFrame(PageFrameTable *pageFrameTable, PageTable *pageTable, PAGE_FRAME_INDEX pageFrameIndex, int processPid);

void incrementTimeUnit(PageFrameTable* pageFrameTable);

void updatePageFrameCount(PageTable* pageTable, PAGE_INDEX pageIndex);

int getPageFrameNextProcessPid(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX pFIndex);

int getPageFrameProcessPid(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX pFIndex, unsigned int *previousProcessId);

void unlockPageFrameTableAccessFromPageTable(PageTable *pageTable);

void unlockPageFrameTableAccess(PageFrameTable *pageFrameTable);

void lockDeadProcess(PageTable *pageTable);

void unlockDeadProcess(PageFrameTable *pageFrameTable);

int getWaitingPageProcessId(PageFrameTable *pageFrameTable);

int getWaitingPageModifiedMode(PageFrameTable *pageFrameTable);

int setWaitingPageProcessId(PageTable *pageTable, unsigned int processId, ACCESS_MODE mode);

int unframePage(PageTable* pageTable, PAGE_FRAME_INDEX frameIndex);

int incrementPageAccessCount(PageFrameTable* pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value);

int decrementPageAccessCount(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value);

int destroyPageFrameTable(PageFrameTable *pageFrameTable);

int setPageAccessCount(PageFrameTable *pageFrameTable, PAGE_FRAME_INDEX frameIndex, unsigned int value);

int getSharedMemSegment(PageFrameTable *pageFrameTable);

#endif
