#ifndef SEMF_H
#define SEMF_H

#include<stdlib.h>

union semun {
   int val;
   struct semid_ds *buf;
   ushort * array;
};

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

#endif
