#include<stdio.h>
#include<sys/stat.h>

int main (void) {

	FILE *pArq;
	char ch;

	if ((pArq = fopen("minhaFifo", "r")) == NULL) {

		puts ("Erro ao abrir a FIFO para escrita"); return -1;
	}

	while ((ch = fgetc(pArq)) != EOF) putchar (ch);

	fclose (pArq);
	return 0;
}
