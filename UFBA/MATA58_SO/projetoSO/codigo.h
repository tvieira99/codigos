#ifndef guard_codigo
#define guard_codigo

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Endereco {
	int nPag;
	int offsetDados;
}
addr;

//ESTRUTURA DA PÁGINA, NÃO VOU PROTEGER CONTRA EXECUÇÃO OU NÃO
typedef struct Pagina{
	int pageId;
	int processId;	
	int contador;
	bool carregada;
	bool modificada;
	int* dados;
	int endereco;
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
	page* pages;
} process;
#endif