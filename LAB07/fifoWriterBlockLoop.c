#include<unistd.h>
#include<stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

#define OPENMODE (O_WRONLY)
#define FIFO "minhaFifo"

int main (void) {

	int fpFIFO;
	char mensagem[101];

	if(access(FIFO, F_OK) == -1) {

		if (mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0) {
			fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
			return -1;
		}
	}

	puts ("Abrindo FIFO");

	if ((fpFIFO = open (FIFO, OPENMODE)) < 0) {
		fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
		return -2;
	}

	while(1) {

		if(fgets (mensagem, 100, stdin) != NULL) {
			write(fpFIFO, mensagem, strlen(mensagem));
		}

	}

	return 0;
}
