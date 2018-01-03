#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define TEXT_SIZE 400

int main() {

	int fd[2];
	int pid = 0, i = 0, nRx = 0, nTx = 0;
	
	char *messages[] = {"Aoi", "Kuroi", "Shiroi", "Shotsu", "Kotoba", "Daigakusei"};

	char receivedText[TEXT_SIZE], message[TEXT_SIZE];

	if(pipe(fd) < 0) {

		printf("\nErro na aberture de pipe.\n");
		exit(-1);

	}

	if(fork() > 0) {

		close(fd[1]);

		for(i = 0; i < 6; i++) {
			nRx = read(fd[0], receivedText, strlen(messages[i]) + 1);
			printf("\n%d dados lidos: %s\n", nRx, receivedText);
		}

		while(i < 11) {

			nRx = read(fd[0], receivedText, TEXT_SIZE);
			printf("\n%d dados lidos: %s\n\n", nRx, receivedText);
			i++;
		}

	} else {

		close(fd[0]);

		for(i = 0; i < 6; i++) {
			nTx = write(fd[1], messages[i], strlen(messages[i]) + 1);
			printf("\n%d dados escritos: %s\n\n", nTx, messages[i]);
		}

		while(i < 11) {

			scanf("%[^\n]%*c", message);

			nTx = write(fd[1], message, strlen(message) + 1);
			printf("\n%d dados escritos: %s\n", nTx, message);

			i++;
		}

	}

	close(fd[0]); close(fd[1]);

	return 0;
}
