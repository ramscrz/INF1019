#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

static void printMatrix(int rows, int cols, double M[rows][cols]) {

	int r, c = 0;

	for(r = 0; r < rows; r++) {
		printf("\n| ");
		for(c = 0; c < cols - 1; c++) {
			printf("\t %.5f", M[r][c]);
		}
		printf("\t %.5f \t|", M[r][c]);
	}

}


static void somaMatrizes(int row, int col, double A[][col], double B[][col]) {

	int i = 0;
	int sum = 0;
	pid_t pidparent = getpid();
	pid_t pid = 0;

	row -= 1;

	if(row >= 0) {
	
		pid = fork();

		if(pid != 0) {
        	// Parent
        	waitpid(pid, 0, 0);
		} else {
			// Child
			somaMatrizes(row, col, A, B);
			printf("\nFILHO <PID %d> de PAI <PID %d>: Resultado da Matriz Soma LINHA %d :\n | ", getpid(), pidparent, row);
			for(i = 0; i < col; i++) {
				printf("\t %.5f", A[row][i] + B[row][i]);
			}
			printf("\t|\n");
			exit(0);
		}
	}
}

int main() {

	double M[][3] = {{1, 3.9, 2}, {2, -3, 2.5}, {90, -11, 3.6}};

	double N[][3] = {{23, -45, 8.2}, {2, 6, 2.9}, {-9.5, 34, 83.5}};

	double A[][4] = {{1, 3.9, 2, 5}, {2, -3, 2.5, 5.4}, {-8, 11, 16.7, -52}, {90, -11, 3.6, 22}};

	double B[][4] = {{23, -45, 8.2, 75.5}, {2, 6, 2.9, 3}, {-8.9, 1.1, 167, -5.2}, {-9.5, 34, 83.5, 11}};

	double C[][4] = {{10, 10, 10, 10}, {10, 10, 10, 10}, {10, 10, 10, 10}, {10, 10, 10, 10}};

	double D[][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};

	printMatrix(3, 3, M);
	printf("\n\n + \n\n");
	printMatrix(3, 3, N);
	printf("\n\n");
	somaMatrizes(3, 3, M, N);
	
	printf("\n\n");
	printMatrix(4, 4, C);
   printf("\n\n + \n\n");
   printMatrix(4, 4, C);
   printf("\n\n");
   somaMatrizes(4, 4, C, C);
	
	printf("\n\n");
   printMatrix(4, 4, D);
   printf("\n\n + \n\n");
   printMatrix(4, 4, C);
   printf("\n\n");
   somaMatrizes(4, 4, D, C);

	printf("\n\n");
   printMatrix(4, 4, A);
   printf("\n\n + \n\n");
   printMatrix(4, 4, B);
   printf("\n\n");
   somaMatrizes(4, 4, A, B);
	printf("\n\n");

	return 0;
}

