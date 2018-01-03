#include<stdio.h>
#include<sys/stat.h>

int main (void) {

	FILE *pArq;

	if ((pArq = fopen("minhaFifo", "w")) == NULL) {

		puts ("Erro ao abrir a FIFO para escrita"); return -1;

	}

	fputs ("Melancia sem caroço", pArq);

	fclose (pArq);
	return 0;
}
