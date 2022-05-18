#include <stdio.h>
#include <stdlib.h>
#include "codigo.h"
#define TAMFILAPROCESSOS 10
//initNewProcess deve retornar o id do novo processo criado. retorna <0 caso o processo não tenha sido criado.



int initNewProcess(process* Fila, int deadline, int tempoDeChegada, int tempoDeExec, int prioridade, int quantum, int sobrecarga) {
	for (int i = 0; i < TAMFILAPROCESSOS; i++){
		if(Fila[i].processId == 0){ 
			process newProcess;
			newProcess.processId = i+1;
			newProcess.deadline = deadline;
			newProcess.sobrecarga = sobrecarga;
			newProcess.tempoDeChegada = tempoDeChegada;
			newProcess.tempoDeExec = tempoDeExec;
			newProcess.prioridade = prioridade;
			Fila[i] = newProcess;
			return i+1;		
		}
	}
}


int main(){
	process FilaDeProcessos[TAMFILAPROCESSOS];
	int process1 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	int process2 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	printf("%d\n",FilaDeProcessos[1].processId);
	//SIMULAÇÃO DA RAM DE 200K COM PÁGINAS DE 4K
	//page *Ram = malloc(sizeof(page)*50);
	//SIMULAÇÃO DE DISCO
	//page *Disc = malloc(sizeof(page)*200);
	//TABELAS DE BLOCOS LIVRES. TEMOS 50 BLOCOS LIVRES GUARDA OS IDs das páginas. SE O VALOR FOR <0. ESTÁ LIVRE.
	//addr *FreeBlockTables = malloc(sizeof(int)*50);

}
