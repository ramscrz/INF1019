// Pipe sendo descritor de arquivo usado para enviar.
// Você escreve na pipe e lê da pipe como se a pipe fosse um arquivo
// A saída do ls ir para a entrada do grep

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {

	int fd[2];
	int pid = 0, i = 0;

	char *progOneArgs[] = {"/bin/grep", "test", NULL};
	char *progTwoArgs[] = {"/bin/ls", NULL};

	if(pipe(fd) < 0) {

		printf("\nErro na abertura de pipe.\n");
		exit(-1);

	}

	if(fork() > 0) {

		close(fd[1]);

		if(dup2(fd[0], 0) == -1) {
			perror("\nErro em dup2() para redirecionar a entrada stdin\n");
			return -1;
		}

		execv(progOneArgs[0], progOneArgs);

	} else {

		close(fd[0]);

		if(dup2(fd[1], 1) == -1) {
			perror("\nErro em dup2() para redirecionar a saída stdout\n");
			return -2;
		}

		execv(progTwoArgs[0], progTwoArgs);

	}

	return 0;
}
