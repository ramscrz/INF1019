#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main() {

	int i, status = 0;
	int value = 0;
	pid_t parentpid = getpid();

	pid_t pid = fork();

	if(pid != 0) {
		//Parent
		printf("\nProcesso pai inicializado...\n");
		printf("pid: %d \t pid(child): %d\n\n", parentpid, pid);
		
		// Parent process waits for the child process to return before resuming execution.
		waitpid(pid, &status, 0);

		for(i = 0; i < 100; i++) {
			value += 5;
			printf("*Pai <%d> - <%d>\n", parentpid, value);
		}

		printf("Processo pai finalizado!\n\n");
		exit(0);
	} else {
		// Child
		printf("\tProcesso filho inicializado...\n");
		printf("\tpid: %d \t pid(parent): %d\n\n", getpid(), parentpid);

		for(i = 0; i < 100; i++) {
			value += 10;
			printf("\t*Filho <%d> - <%d>\n", getpid(), value);
		}

		printf("\tProcesso filho finalizado!\n\n");
		exit(0);
	}
	
	return 0;
}
