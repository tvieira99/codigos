#ifndef guard_codigo
#define guard_codigo

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct DiscEntry {
	bool used;
	int dados;
}
DEntry;

typedef struct FreeTableEntry {
	struct Pagina* endereco;
	bool loaded;
	int lastTickUsed;
} FTEntry; 

//ESTRUTURA DA PÁGINA, NÃO VOU PROTEGER CONTRA EXECUÇÃO OU NÃO

typedef struct Pagina{
	int pageId;
	bool carregada;
	struct DiscEntry* dados;
	int endereco; //Campo mock-up. Ainda não tem uso
	struct FreeTableEntry* FTEPointer;
} page;

//ESTRUTURA DO PROCESSO.
typedef struct Processos {
	int tempoDeChegada;
	int tempoDeExec;
	int deadline;
	int prioridade;
	int quantum;
	int sobrecarga;
	int processId;
	page* pageTable;
} process;

void addPageToRam();
void initNewPage(process *processo, int pageId, unsigned int dados);
int initNewProcess(int deadline, int tempoDeChegada, int tempoDeExec, int prioridade, int quantum, int sobrecarga);
#endif
