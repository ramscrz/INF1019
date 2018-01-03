/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo globalsettings.h
 * PUC-Rio
 */

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

// Uncomment the macro DEBUGMODE below to enable debug information
// to be printed on terminal during execution.
// #define DEBUGMODE

// Uncomment the macro PRINTPAGETABLE below to enable the Page Table Content
// to be printed on terminal during execution.
//#define PRINTPAGETABLE

// Uncomment the macro PRINTPAGEFRAMETABLE below to enable the Page Frame Table Content
// to be printed on terminal during execution.
//#define PRINTPAGEFRAMETABLE

// Uncomment the macro VMGMDETAIL below to enable some moderate detailed information
// to be printed on terminal during execution.
//#define VMGMDETAIL


#define N_USER_PROCESSES 4 			// Number of processes to be executed.
#define NUMBER_PAGE_FRAMES 256	// Number of Page Frames 
#define TIMEWINDOW 30						// Size of the LFU time window (unit of time, not real time).

typedef unsigned char PAGE_FRAME_INDEX;
typedef int PAGE_INDEX;
typedef int OFFSET;

int printCurrentTime(int enable);

#endif
