#include <stdio.h>
#include <stdlib.h>
//paginas

//initNewProcess deve retornar o id do novo processo criado. retorna <0 caso o processo não tenha sido criado.
int initNewProcess() {
	
}


typedef struct Pagina{
	int processId;	
	int contador;
	int dado;
} page;

//processos
typedef struct Processos {
	int tempoDeChegada;
	int tempoDeExec;
	int deadline;
	int prioridade;
	int quantum;
	int sobrecarga;
	int processId;
	page* pages;
} process;

int main(){
	page *Ram = malloc(sizeof(page)*50);
	page *Disc = malloc(sizeof(page)*200);
}
