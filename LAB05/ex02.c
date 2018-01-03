#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>

int main() {

	int fIn, fOut, i = 0;
	char tmp;
	
	if( (fIn = open("input", O_RDONLY | O_CREAT,0666)) == -1) {

		perror("\nErro em open()\n");
		return -1;

	}

	if( (fOut = open("output", O_WRONLY | O_CREAT | O_TRUNC,0666)) == -1) {

		perror("\nErro em open()\n");
		return -2;

	}

	close(0);
	
	if(dup(fIn) == -1) {

		perror("\nErro em dup() para redirecionar a entrada stdin\n");
		return -3;

	}

	if(dup2(fOut, 1) == -1) {

		perror("\nErro em dup2() para redirecionar a saída stdout\n");
		return -4;
	}

	do {
		scanf("%c", &tmp);
		printf("%c", tmp);
	} while(tmp != '\n');

	close(fIn);
	close(fOut);

	return 0;
}

