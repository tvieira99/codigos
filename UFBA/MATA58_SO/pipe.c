/**
 * FORMULA DE BÁSKARA UTILIZANDO PIPES!!!!
 * Thiago Vieira Souza Andrade
 */
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>

int main (int argc, char *argv[]){
	int fd_1[2], fd_2[2], fd_3[2];
	char readbuffer[80];
	pid_t son1,son2,son3;

	pipe(fd_1);
	pipe(fd_2);
	pipe(fd_3);
	//son1 irá fazer (-b+sqrt(delta))/2a
	//son2 irá fazer (-b-sqrt(delta))/2a
	//son3 irá receber os valores e dar -b sqrt(delta) e 2a
	close(fd_1[0]);
	float valorA;
	scanf("%f", &valorA);
	float valor2A = 2 * valorA;
	write(fd_1[1], &valor2A, (sizeof(valor2A))); 	
	
	close(fd_2[0]);
	float valorB;
	scanf("%f", &valorB);
	float valorBneg = -valorB;
	write(fd_2[1], &valorBneg, (sizeof(valorBneg))); 

	close(fd_3[0]);
	float valorC;
	scanf("%f", &valorC);
	float delta = sqrt(valorB*valorB -(4*valorA*valorC));
	write(fd_3[1], &delta, (sizeof(delta))); 
	
	if((son1 = fork()) == -1){
		perror("fork");
		exit(1);
	}

	if(son1 == 0){
		float negB, A2, sqrtDelta;
		close(fd_1[1]);
		read(fd_1[0], readbuffer, sizeof(readbuffer));	
		A2 = atof(readbuffer);

		close(fd_2[1]);
		read(fd_2[0], readbuffer, sizeof(readbuffer));	
		negB = atof(readbuffer);
		
		close(fd_3[1]);
		read(fd_3[0], readbuffer, sizeof(readbuffer));	
		sqrtDelta = atof(readbuffer);

		float valorTotal = (negB+sqrtDelta)/A2;
		printf("%f\n", valorTotal);	
	}
	
	if((son2 = fork()) == -1){
		perror("fork");
		exit(1);
	}

	if(son2 == 0){
		float negB, A2, sqrtDelta;
		close(fd_1[1]);
		read(fd_1[0], readbuffer, sizeof(readbuffer));	
		A2 = atof(readbuffer);

		close(fd_2[1]);
		read(fd_2[0], readbuffer, sizeof(readbuffer));	
		negB = atof(readbuffer);
		
		close(fd_3[1]);
		read(fd_3[0], readbuffer, sizeof(readbuffer));	
		sqrtDelta = atof(readbuffer);

		float valorTotal = (negB-sqrtDelta)/A2;
		printf("%f\n", valorTotal);	
	}
	
	if((son3 = fork()) == -1){
		perror("fork");
		exit(1);
	}

	if(son3 == 0){
	}
}
