#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main() {

	int status = 0;
	pid_t parentpid = getpid();

	pid_t pid = fork();

	if(pid != 0) {
		// Parent
		printf("\nProcesso pai inicializado...\n");
		printf("pid: %d \t pid(child): %d\n\n", parentpid, pid);

		// Parent process waits for the child process to return before resuming execution.
		waitpid(pid, &status, 0);
		printf("Processo pai finalizado!\n\n");
		exit(0);
	} else {
		// Child
		printf("\tProcesso filho inicializado...\n");
		printf("\tpid: %d \t pid(parent): %d\n", getpid(), parentpid);
		printf("\tProcesso filho finalizado!\n\n");
		exit(0);
	}

	return 0;
}


