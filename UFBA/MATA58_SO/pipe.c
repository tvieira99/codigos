/*
 * FORMULA DE BÁSKARA UTILIZANDO PIPES!!!!
 * Thiago Vieira Souza Andrade
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>

int main (int argc, char *argv[]){
	int fd1[2],fd2[2],fd3[3];	
	pid_t son1;
	pipe(fd1);
	pipe(fd2);
	pipe(fd3);
	//son1 irá fazer (-b+sqrt(delta))/2a
	//son2 irá fazer (-b-sqrt(delta))/2a
	
	son1 = fork();	
	if(son1 == -1){
		perror("fork");
		exit(1);
	}

	if(son1 == 0){
		float num1, num2, num3;
		close(fd1[1]);
		read(fd1[0], &num1, (sizeof(float)));
		close(fd1[0]);
		
		close(fd2[1]);
		read(fd2[0], &num2, (sizeof(float)));
		close(fd2[0]);
		
		close(fd3[1]);
		read(fd3[0], &num3, (sizeof(float)));
		close(fd3[0]);

		float x1 = (num2 - num3)/num1;
		printf("%f\n",x1);
		exit(0);
	} else {
		float num1, num2, num3;
		scanf("%f %f %f", &num1, &num2, &num3);
		
		float a2 = num1 * 2;
		float negB = -num2;
		float deltPart1 = num2*num2;
		float deltPart2 = 4*num1*num3;
		float de = deltPart1 - deltPart2;
		float delta = sqrtf(de);
		
		close(fd1[0]);
		write(fd1[1], &a2, (sizeof(float)));
		close(fd1[1]);

		close(fd2[0]);
		write(fd2[1], &negB, (sizeof(float)));
		close(fd2[1]);

		close(fd3[0]);
		write(fd3[1], &delta, (sizeof(float)));
		close(fd3[1]);
		
		float x2 = (negB + delta)/a2;
		printf("%f\n",x2);
	}
}
