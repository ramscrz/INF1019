#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

pid_t multiSearchValue(int iter, int nchildren, int n, int* values, int value) {

	int i = 0;
	int start = 0;
	int end = n;
	int resto = 0;

	if(iter > 0) {
	
		pid_t parentpid = getpid();

		pid_t pid = fork();

		if(pid != 0) {

			n = n - (n % nchildren);
			multiSearchValue(iter - 1, nchildren, n, values, value);
			waitpid(pid, 0, 0);
		} else {
			// Child
			resto = n % nchildren;

			start = n/nchildren * (iter - 1);
			end = start + n/nchildren + resto;
			printf("Filho < PID %d > de pai < PID %d > procurando valor %d entre posições %d e %d.\n", 
						getpid(), parentpid, value, start, end - 1);

			for(i = start; i < end; i++) {
				if(values[i] == value) {
					printf("Filho < PID %d > Encontrou na posição %d do vetor.\n", getpid(), i);
				} else {
					printf("Filho < PID %d > Não encontrou ocorrência do valor.\n", getpid());
				}
			}

			printf("\tProcesso filho %d finalizado!\n\n", iter);
			exit(0);		
		}
	}

}

int main() {

	int values[] = {1, 6, 5, 6, 3, 5, 6, -4, 5, 6, 2, 5, 100, 26, 6, 5};
	int values2[] = {6, 2, 6, 1, 4};
	int values3[] = {6, 2, 6, 1, 6, 4};
	int values4[] = {6, 2, 6, 1, 6, 4, 6};
	int values5[] = {6, 2, 6, 1, 6, 4, 6, 21};

	printf("\n\n<< Primeiro teste >>\n\n");
	multiSearchValue(4, 4, 16, values, 6);
	printf("\n\n<< Segundo teste >>\n\n");
   multiSearchValue(4, 4, 16, values, 5);
	printf("\n\n<< Terceiro teste >>\n\n");
	multiSearchValue(5, 5, 5, values2, 6);
	printf("\n\n<< Quarto teste >>\n\n");
	multiSearchValue(4, 4, 6, values3, 6);
	printf("\n\n<< Quinto teste >>\n\n");
	multiSearchValue(4, 4, 7, values4, 6);
	printf("\n\n<< Sexto teste >>\n\n");
	multiSearchValue(4, 4, 8, values5, 6);

	return 0;

}

