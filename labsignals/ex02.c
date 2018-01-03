#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>

void divisionByZeroHandler(int signal) {

	printf("\nAttempt to divide by 0! Floating Point Error!\n");
	printf("This arithmetic operation will be ignored!\n\n");

	exit(1);
}

int main() {

	int first = 0, second = 0, option = 0;
	double firstR = 0.0, secondR = 0.0;

	printf("\n\nType 1 to enable custom SIGFPE Handler.\n");
	printf("Type anything different than 1 to use default handler for SIGFPE.\n\n");

	printf("Choose: ");
	scanf("%d", &option);
	printf("\n");

	if(option == 1) {
		signal(SIGFPE, divisionByZeroHandler);
	}

	option = 0;

	printf("\n\nType 1 for arithmetic operations between integers.\n");
	printf("Type 2 for arithmetic operations between real number.\n\n");
	
	while(option != 1 && option != 2) {
		printf("Choose: ");
		scanf("%d", &option);
		printf("\n");
	}

	if(option == 1) {
		printf("Enter Number A: ");
		scanf("%d", &first);
		printf("Enter Number B: ");
		scanf("%d", &second);

		printf("\n%d + %d = \n", first, second);
		printf("%d\n", first + second);
		printf("\n%d - %d = \n", first, second);
		printf("%d\n", first - second);
		printf("\n%d * %d = \n", first, second);
		printf("%d\n", first * second);
		printf("\n%d / %d = \n", first, second);
		printf("%d\n", first / second);

	} else if(option == 2) {
		printf("Enter Number A: ");
		scanf("%lf", &firstR);
		printf("Enter Number B: ");
		scanf("%lf", &secondR);

		printf("\n%lf + %lf = \n", firstR, secondR);
		printf("%lf\n", firstR + secondR);
		printf("\n%lf - %lf = \n", firstR, secondR);
		printf("%lf\n", firstR - secondR);
		printf("\n%lf * %lf = \n", firstR, secondR);
		printf("%lf\n", firstR * secondR);
		printf("\n%lf / %lf = \n", firstR, secondR);
		printf("%lf\n", firstR / secondR);
	}
	
	return 0;
}
