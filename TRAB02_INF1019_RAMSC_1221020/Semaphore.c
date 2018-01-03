/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo Semaphore.c
 * PUC-Rio
 */

#include <sys/sem.h>
#include <unistd.h>
#include <ctype.h>
#include "Semaphore.h"

union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};

int setupSemaphore() {
	int semId = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
	
	if(setSemValue(semId) < 0) {
		return -1;
	}

	return semId;
}

int setSemValue(int semId) {
	union semun semUnion;
	semUnion.val = 1;

	if(semId < 0) {
		return -1;
	}

	return semctl(semId, 0, SETVAL, semUnion);
}

int delSemValue(int semId) {
	union semun semUnion;

	if(semId < 0) {
		return -1;
	}

	return semctl(semId, 0, IPC_RMID, semUnion);

}

int semaforoP(int semId) {
	struct sembuf semB;
	semB.sem_num = 0;
	semB.sem_op = -1;
	semB.sem_flg = SEM_UNDO;
	semop(semId, &semB, 1);
	return 0;
}

int semaforoV(int semId) {
	struct sembuf semB;
	semB.sem_num = 0;
	semB.sem_op = 1;
	semB.sem_flg = SEM_UNDO;
	semop(semId, &semB, 1);
	return 0;
}
