#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main() {

	char command[200];

	scanf("%s", command);

	if(strcmp(command, "Hello! I'm a!") == 0) {
		exit(3);
	}

	return 0;
}
