#include<stdio.h>

int main() {

	int i = 0;
	char * japanNumbers[20] = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十",
									 "十一", "十二", "十三", "十四", "十五", "十六", "十七", 
									 "十八", "十九", "二十"};

	printf("\n\nPrograma executado a partir de outro programa de comandos SHELL.\n\n");

	for(i = 0; i < 20; i++) {

		printf("\t\t\t %s \t %d \n", japanNumbers[i], i+1);

	}

	printf("\n\n");

	return 0;
}

