#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/wait.h>

#define FIFO "SuperFifo"

int main (void) {

	int fpFIFO, pid, npid;
   char ch;
	char mensagemOne[] = "Anata no namae wa, nan desu ka?\n";
	char mensagemTwo[] = "Watashi wa Rafaeru to moushimasu.\n";

	if (access(FIFO, F_OK) == -1) {

		if (mkfifo(FIFO, S_IRUSR | S_IWUSR) != 0) {

			fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
			return -1;
		}
	}

	pid = fork();

	if(pid > 0) {

		if ((fpFIFO = open (FIFO, O_RDONLY)) < 0) {
			fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
			return -2;
		}

		npid = fork();

		if(npid > 0) {

			waitpid(npid, 0, 0);

		} else if(npid == 0) {

			waitpid(pid, 0, 0);

			if ((fpFIFO = open (FIFO, O_WRONLY)) < 0) {
				fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
				return -2;
			}

			write(fpFIFO, mensagemTwo, strlen(mensagemTwo));

			close (fpFIFO);

			exit(0);

		}

   	puts("Começando a ler...");

   	while(read(fpFIFO, &ch, sizeof(ch)) > 0)
      	printf("%c", ch);

   	puts ("Fim da leitura");
		close (fpFIFO);

	} else if(pid == 0) {

		if ((fpFIFO = open (FIFO, O_WRONLY)) < 0) {
			fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
			return -2;
		}

		write(fpFIFO, mensagemOne, strlen(mensagemOne));

		close (fpFIFO);

	}

   return 0;
}
