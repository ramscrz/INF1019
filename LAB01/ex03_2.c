
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main() {

        int i, status = 0;
        pid_t parentpid = getpid();
		pid_t pidGrandparent = 0;
        pid_t pid = fork();

        if(pid != 0) {
                // Parent
                printf("\nProcesso pai inicializado...\n");
                printf("pid: %d \t pid(child): %d\n\n", parentpid, pid);

				waitpid(pid, &status, 0);
				for(i = 0; i < 100; i++) {
					printf("\nPai <pid: %d> - <pid(parent): - > <value: %d>", parentpid, i);
					sleep(1);
				}             
   
                printf("Processo pai vai finalizar\n\n");
                exit(0);
        } else {
				// Child
				printf("\tFilho foi criado\n");
				printf("\tpid: %d \t pid(parent): %d\n\n", getpid(), parentpid);
				pidGrandparent = parentpid;
				parentpid = getpid();
				pid = fork();

				if(pid != 0) {
					// Child Parent
					waitpid(pid, &status, 0);
					for(i = 100; i < 200; i++) {
						printf("\n\tFilho <pid: %d> - <pid(parent): %d> <value: %d>", 
						getpid(), pidGrandparent, i);
						sleep(2);
					}
					printf("\tProcesso fiho vai finalizar\n\n");
					exit(0);
				} else {
					// Neto
					printf("\t\tNeto foi criado\n");
					printf("\t\tpid: %d \t pid(parent): %d \t pid(grandparent): %d\n\n", 
					getpid(), parentpid, pidGrandparent);

					for(i = 200; i < 300; i++) {
						printf("\n\t\tNeto <pid: %d> - <pid(parent): %d> <value: %d>", getpid(), parentpid, i);
						sleep(3);
					}

					printf("\t\tProcesso neto vai finalizar\n\n");
					exit(0);

				}
        }

        return 0;
}


