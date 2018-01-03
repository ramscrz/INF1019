/* Trabalho 01: Escalonamento em Múltiplos Níveis com Feedback (MLF)
 * INF1019 - Sistemas de Computação
 * 22/10/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo globalsettings.h
 * PUC-Rio
 */

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#define FIFO_SIZE  50
#define PARAMETERS_SIZE 250
#define COMMAND_SIZE 500

// Uncomment the line below to enable detailed information
// about the scheduler's execution to be printed on terminal during execution.
//#define SCHEDULER_EXEC

// Uncomment the line below to enable debug information to be 
// printed on terminal during execution.
// #define SET_DEBUG

void printLog(char* logFileName, char* message, char* argument);

#endif
