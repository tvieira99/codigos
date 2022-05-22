#include <stdio.h>
#include <stdlib.h>
#include "codigo.h"
#define TAMFILAPROCESSOS 10
#define NUM_PAG_P_PROCESSOS 10
#define TAM_DISCO 200
#define TAM_RAM 50

static unsigned int indexDoDisco = 0;
static process FilaDeProcessos[TAMFILAPROCESSOS];
static int Disco[TAM_DISCO];
static FTEntry FreeFrameList[TAM_RAM];
static unsigned int RAM[TAM_RAM];
static int paginacaoFIFOindex = 0;

//implementing FIFO
void paginacaoFIFO(){
	//pega o ponteiro da pagina que a FTL está gerenciando
	page* paginaASair = FreeFrameList[paginacaoFIFOindex%50].endereco;
	printf("\nEndereço da pagina a ser liberada %p\n", paginaASair);
	FreeFrameList[paginacaoFIFOindex%50].loaded = false;
	FreeFrameList[paginacaoFIFOindex%50].lastTickUsed = 0;
	FreeFrameList[paginacaoFIFOindex%50].endereco = NULL;

	//desmarca a pagina como carregada
	paginaASair->carregada = false;
	Disco[indexDoDisco] = *(paginaASair->dados);
	paginaASair->dados = &Disco[indexDoDisco];
	paginaASair->FTEPointer = NULL;
	indexDoDisco++;
	paginacaoFIFOindex = paginacaoFIFOindex + 1;
}

void paginacaoLRU(){
	printf("liberou uma página\n");
	//pega o ponteiro da pagina que a FTL está gerenciando
	int minContador = 0;
	int ponteiroMaiorContagem = 0;
	for (int i = 0; i < TAM_RAM; i++ ){
		if(FreeFrameList[i].lastTickUsed < minContador){
			minContador = FreeFrameList[i].lastTickUsed;
			ponteiroMaiorContagem = i;
		}
	}
	page* paginaASair = FreeFrameList[ponteiroMaiorContagem].endereco;
	FreeFrameList[ponteiroMaiorContagem].loaded = false;
	FreeFrameList[ponteiroMaiorContagem].lastTickUsed = 0;
	FreeFrameList[ponteiroMaiorContagem].endereco = NULL;

	//desmarca a pagina como carregada
	paginaASair->carregada = false;
	Disco[indexDoDisco] = *(paginaASair->dados);
	paginaASair->dados = &Disco[indexDoDisco];
	paginaASair->FTEPointer = NULL;
	indexDoDisco++;
}

//addPageToRam: Busca uma entrada livre na FrameList, se acha, insere nela
void addPageToRam(page* pagina, unsigned int dados){
	for (int i = 0; i < TAM_RAM; i++){
		if (FreeFrameList[i].loaded == false){
			printf("\ndados: %u", dados);
			RAM[i] = dados;
			pagina->dados = &RAM[i];
			pagina->FTEPointer = &FreeFrameList[i];
			pagina->carregada = true;
			FreeFrameList[i].loaded = true;
			FreeFrameList[i].endereco = pagina;
			FreeFrameList[i].lastTickUsed = 0;
			break;
		}
		if (i == 49){
			paginacaoFIFO();
			i = -1;
		}
	}
}

void initRAM(){
	for (int i = 0; i < TAM_RAM; i++){
		RAM[i] = 0;
	}
}

//inicia FreeFrameList com as FTEntry necessárias

void initFreeFrameList(){
	for (int i = 0; i < TAM_RAM; i++){
		FTEntry newFTEntry;
		newFTEntry.loaded = false;
		newFTEntry.endereco = NULL;
		newFTEntry.lastTickUsed = 0;
		FreeFrameList[i] = newFTEntry;
	}
}

//initNewPage deve retornar o id da nova página não modificada nem carregada, se não foi criada, retorna um valor <0.
void initNewPage(process *processo, int pageId, unsigned int dados){
	page newPage;
	newPage.pageId = pageId;
	newPage.processo = processo;
	processo->pageTable[pageId] = newPage;
	newPage.endereco = 0;
	addPageToRam(&processo->pageTable[pageId], dados);	
}

void runProcess(process* process, int currentTick){
	for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++){
		if (process->pageTable[i].carregada == false){
			printf("\nProcesso %d não pode ser executado porque a página %d não foi carregada", process->processId, process->pageTable[i].pageId);
			return;
		}
		process->pageTable[i].carregada;
	}
	printf("\nProcesso %d foi executado com sucesso", process->processId);
	process->tempoDeExec = process->tempoDeExec + 1;
	//PEGA UMA PÁGINA ALEATÓRIA PARA O PROCESSO ACESSAR
	process->pageTable[rand()%10].FTEPointer->lastTickUsed = currentTick;
}


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
			newProcess.pageTable = malloc(sizeof(page)*NUM_PAG_P_PROCESSOS);
			for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++){
				initNewPage(&newProcess, i, rand());
			}
			Fila[i] = newProcess;
			return i+1;		
		}
	}
}


int main(){
	initFreeFrameList();
	initRAM();
	int process1 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	runProcess(&FilaDeProcessos[0], 0);
	int process2 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	int process3 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	int process4 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	int process5 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	int process6 = initNewProcess(FilaDeProcessos, 0, 0, 0, 0, 0, 0);
	runProcess(&FilaDeProcessos[0],10);
	printf("\nPagina %d do processo 1 existe!",FilaDeProcessos[0].pageTable[2].pageId);
	for (int i = 0; i < TAM_RAM; i++){
			printf("\n Endereco da pagina %p ", FreeFrameList[i].endereco);
	}
	for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++){
		if(!FilaDeProcessos[process1-1].pageTable[i].carregada){
			printf("\nPagina %d não está carregada do processo %d", i, process1);
		}
	}
	for (int i = 0; i < TAM_DISCO; i++){
		if (Disco[i] != 0){
			printf("\nEsse dado está guardado no disco: %u", Disco[i]);
		}
	}
	//SIMULAÇÃO DA RAM DE 200K COM PÁGINAS DE 4K
	//page *Ram = malloc(sizeof(page)*50);
	//SIMULAÇÃO DE DISCO
	//TABELAS DE BLOCOS LIVRES. TEMOS 50 BLOCOS LIVRES GUARDA OS IDs das páginas. SE O VALOR FOR <0. ESTÁ LIVRE.
	//addr *FreeBlockTables = malloc(sizeof(int)*50);

}
