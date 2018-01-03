/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo Semaphore.h
 * PUC-Rio
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

int setupSemaphore();

int setSemValue(int semId);

int delSemValue(int semId);

int semaforoP(int semId);

int semaforoV(int semId);

#endif
