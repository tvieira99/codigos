#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#define N 20
 
sem_t homem, mulher, banheiro;
int conta_homem, conta_mulher;
void mulher_quer_entrar();
void mulher_sai();
void mulher_usa_banheiro();
void adiciona_mulher();
void homem_quer_entrar();
void homem_sai();
void homem_usa_banheiro();
void adiciona_homem();
 
int main() {
    conta_mulher = 0;
    conta_homem = 0;
    pthread_t thread[N];
 
    /* Inicializando os semaforos */
    int i, res_parc, res = 0;
    res_parc = sem_init(&homem, 0, 1);
    res += res_parc;
    res_parc = sem_init(&mulher, 0, 1);
    res += res_parc;
    res_parc = sem_init(&banheiro, 0, 1);
    res += res_parc;
    if (res != 0) {
        perror("Falha na inicializacao dos semaforos\n");
        exit(EXIT_FAILURE);
    }
 
    /* Inicializando threads */
    for (res = 0, i = 0; i < N; i++) {
        usleep(rand() % 300000);
        if (rand() % 2 != 0)
            res_parc = pthread_create(&thread[i], NULL,
                    (void*) adiciona_mulher, (int*) (i + 1));
        else
            res_parc = pthread_create(&thread[i], NULL, (void*) adiciona_homem,
                    (int*) (i + 1));
        res += res_parc;
    }
    if (res != 0) {
        perror("Erro em criacao de threads\n");
        exit(EXIT_FAILURE);
    }
    /* Juntando threads */
    for (i = 0; i < N; i++) {
        res_parc = pthread_join(thread[i], NULL);
        res += res_parc;
    }
    if (res != 0) {
        perror("Erro em join threads\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void mulher_usa_banheiro() {
    usleep(rand() % 500000);
}
 
void adiciona_mulher() {
    usleep(rand() % 400000);
    mulher_quer_entrar();
    mulher_usa_banheiro();
    mulher_sai();
    pthread_exit();
}

void homem_usa_banheiro() {
    usleep(rand() % 300000);
}
 
void adiciona_homem() {
    usleep(rand() % 400000);
    homem_quer_entrar();
    homem_usa_banheiro();
    homem_sai();
    pthread_exit();
}
 
void mulher_quer_entrar() {
	//semaforo mulher valor
	int sm_val;
	//semaforo banheiro valor
	int sb_val;
	sem_getvalue(&mulher, &sm_val);	
	sem_getvalue(&banheiro, &sb_val);
	//Se banheiro estiver ocupado e for por uma mulher
	if(sb_val <= 0){
		if(sm_val <= 0){
			sem_wait(&banheiro);
			sem_wait(&mulher);	
		}
	} else { //Se ele não estiver ocupado
		sem_wait(&banheiro);
		sem_wait(&mulher);
	}
	conta_mulher = conta_mulher + 1;
}
 
void mulher_sai() {
		sem_post(&banheiro);
		sem_post(&mulher);
		conta_mulher = conta_mulher - 1;
}

void homem_quer_entrar() {
	//semaforo homem valor
	int sh_val;
	//semaforo banheiro valor
	int sb_val;
	sem_getvalue(&homem, &sh_val);	
	sem_getvalue(&banheiro, &sb_val);
	//Se banheiro estiver ocupado e for por um homem 
	if(sb_val <= 0){
		if(sh_val <= 0){
			sem_wait(&banheiro);
			sem_wait(&homem);	
		}
	} else { //Se ele não estiver ocupado
		sem_wait(&banheiro);
		sem_wait(&homem);
	}
	conta_homem = conta_homem + 1;
}
 
void homem_sai() {
		sem_post(&banheiro);
		sem_post(&homem);
		conta_homem = conta_homem - 1;
}
 

