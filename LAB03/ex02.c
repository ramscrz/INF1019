#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>

int main(int argc, char * argv[]) {

	pid_t pid, parentpid;
	int segment, status;
	char* p;
	char* const parameters[] = {"\t", "8180", NULL};
	char *key = "8180";

	parentpid = getpid();

	segment = shmget(8180, 500 * sizeof(char), IPC_CREAT | S_IRUSR | S_IWUSR);

	if(segment < 0) {
		exit(2);
	}

	p = (char *) shmat(segment, 0, 0);

	if(p < 0) {
		exit(2);
	}

	do {

		printf("Aguarda nova Mensagem do Dia...");

		printf("\n\nMensagem do Dia: ");
		fgets(p, 499, stdin);
		printf("\n\n");

		if(strcmp(p, "q\n") != 0) {

			pid = fork();

			if(pid > 0) {
				//Parent
				printf("\nProcesso pai inicializado...\n");
				printf("pid: %d \t pid(child): %d\n\n", parentpid, pid);
				printf("Pai recebe mensagem do dia: %s\n", p);
				waitpid(pid, &status, 0);
			} else if(pid == 0) {
				//Child
				printf("\tProcesso filho inicializado...\n");
				printf("\tpid: %d \t pid(parent): %d\n", getpid(), parentpid);
				execv("DailyMessage", parameters);
				shmdt(p);
				exit(0);
			} else {
				// Error!
				shmdt(p);
				shmctl(segment, IPC_RMID, 0);
				exit(1);
			}
		}

	} while(strcmp(p, "q\n") != 0);

	printf("\n\nProcesso pai finalizado!\n\n");

	shmdt(p);
	shmctl(segment, IPC_RMID, 0);

	return 0;
}
