#include <stdio.h>
#include <stdlib.h>
#include "codigo.h"
#include <time.h>
#include <unistd.h>
#define TAMFILAPROCESSOS 10
#define NUM_PAG_P_PROCESSOS 10
#define TAM_DISCO 200
#define TAM_RAM 50


bool debug = true;
static unsigned int indexDoDisco = 0;

static process FilaDeProcessos[TAMFILAPROCESSOS];

static DEntry Disco[TAM_DISCO];
static FTEntry FreeFrameList[TAM_RAM];
static DEntry RAM[TAM_RAM];
static int paginacaoFIFOindex = 0;


void debugFunction(){
	printf("\nAtual estado da fila de processos:");
	for (int i = 0; i < TAMFILAPROCESSOS; i++){
		if (FilaDeProcessos[i].processId > 0){
			printf("\n\tProcesso %d está presente na fila de processos", FilaDeProcessos[i].processId);
		}
	}
	printf("\nAtual estado das páginas de cada processo:");
	for(int i = 0; i < TAMFILAPROCESSOS; i++){
		if (FilaDeProcessos[i].processId > 0){
			printf("\nProcesso %d", FilaDeProcessos[i].processId);
			for (int j = 0; j < NUM_PAG_P_PROCESSOS; j++){
				printf("\n\tPágina %d está:\n\t\tcarregada:%i\n\t\tdados:%i\n\t\tFTEPointer:%p", 
				FilaDeProcessos[i].pageTable[j].pageId,
				FilaDeProcessos[i].pageTable[j].carregada,
				FilaDeProcessos[i].pageTable[j].dados->dados,
				FilaDeProcessos[i].pageTable[j].FTEPointer
				);
			}
		}
	}
	printf("\nAtual estado da RAM:");
	printf("\n[");
	for (int i = 0; i < TAM_RAM; i++){
		printf("\n\tDados:%d\n\tUsed:%i", RAM[i].dados, RAM[i].used);
	}
	printf("]");

	printf("\nAtual estado do Disco:");
	printf("\n[");
	for (int i = 0; i < TAM_DISCO; i++){
		printf("\n\tDados:%d\n\tUsed:%i", Disco[i].dados, Disco[i].used);
	}
	printf("]"); 
}

DEntry* addToDisc(int dados){
	for (int i = 0; i < TAM_DISCO; i++){
		if (Disco[i].used == false){
			DEntry newDEntry;
			newDEntry.used = true;
			newDEntry.dados = dados;
			Disco[i] = newDEntry;
			return &Disco[i];
		}
	}
}

//implementing FIFO
void paginacaoFIFO(){
	//pega o ponteiro da pagina que a FTL está gerenciando
	page* paginaASair = FreeFrameList[paginacaoFIFOindex%50].endereco;
	if(debug == true){
		printf("\nEndereço da pagina a ser liberada %p\n", paginaASair);
	}
	FreeFrameList[paginacaoFIFOindex%50].loaded = false;
	FreeFrameList[paginacaoFIFOindex%50].lastTickUsed = 0;

	//desmarca a pagina como carregada
	paginaASair->carregada = false;
	paginaASair->dados = addToDisc(paginaASair->dados->dados);
	paginaASair->FTEPointer = NULL;
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
			DEntry newDEntry;
			newDEntry.dados = dados;
			newDEntry.used = true;
			RAM[i] = newDEntry;
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
		DEntry newDEntry;
		newDEntry.dados = 0;
		newDEntry.used = false;
		RAM[i] = newDEntry;
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
	processo->pageTable[pageId] = newPage;
	newPage.endereco = 0;
	addPageToRam(&processo->pageTable[pageId], dados);	
}

void runProcess(process* process, int currentTick){
	for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++){
		if (process->pageTable[i].carregada == false){
			if(debug == true){
				printf("\nProcesso %d não pode ser executado porque a página %d não foi carregada tentando por a página na ram", process->processId, process->pageTable[i].pageId);
			}
			addPageToRam(&process->pageTable[i], process->pageTable[i].dados->dados);
			runProcess(process, currentTick);
			return;
		}
	}
	int numRandom = rand()%10;
	printf("\nProcesso %d foi executado com sucesso utilizando a página %d", process->processId, numRandom);
	process->tempoDeExec = process->tempoDeExec + 1;

	//PEGA UMA PÁGINA ALEATÓRIA PARA O PROCESSO ACESSAR
	process->pageTable[numRandom].FTEPointer->lastTickUsed = currentTick;
}


//initNewProcess deve retornar o id do novo processo criado. retorna <0 caso o processo não tenha sido criado.

int initNewProcess(int deadline, int tempoDeChegada, int tempoDeExec, int prioridade, int quantum, int sobrecarga) {
	for (int i = 0; i < TAMFILAPROCESSOS; i++){
		if(FilaDeProcessos[i].processId == 0){ 
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
			FilaDeProcessos[i] = newProcess;
			return i+1;		
		}
	}
}

void sortByTempoDeChegada()
{
	int size = sizeof FilaDeProcessos / sizeof *FilaDeProcessos;

	int i, j;
	struct Processos temp;

	for(i = 0; i < size - 1; i++) {
		for(j = i + 1; j < size; j++)
		{
			if (FilaDeProcessos[i].tempoDeChegada > FilaDeProcessos[j].tempoDeChegada) 
			{
				temp = FilaDeProcessos[i];
				FilaDeProcessos[i] = FilaDeProcessos[j];
				FilaDeProcessos[j] = temp;

			}
		}
	}

}

void sortByTempoExec()
{
	int size = sizeof FilaDeProcessos / sizeof *FilaDeProcessos;

	int i, j;
	struct Processos temp;

	sortByTempoDeChegada();

	for(i = 0; i < size - 1; i++) {
		for(j = i + 1; j < size; j++)
		{
			if (FilaDeProcessos[i].tempoDeChegada == FilaDeProcessos[j].tempoDeChegada) 
			{
				if (FilaDeProcessos[i].tempoDeExec > FilaDeProcessos[j].tempoDeExec) {
					temp = FilaDeProcessos[i];
					FilaDeProcessos[i] = FilaDeProcessos[j];
					FilaDeProcessos[j] = temp;
				}
			}
		}
	}

}


void systemEscalonatorExecFIFOandSJF() 
{
	int size = sizeof FilaDeProcessos / sizeof *FilaDeProcessos;

	for(int i = 0; i < size; i++) 
	{
		//sleep(FilaDeProcessos[i].tempoDeExec);
		for (int j = 0; j < FilaDeProcessos[i].tempoDeExec; ++j) 
		{
			putchar('#'); 
  	}
		if (FilaDeProcessos[i].tempoDeExec != 0) 
		{
			printf("\n"); 
			int tempodeEspera = (FilaDeProcessos[0].tempoDeChegada - 0) + FilaDeProcessos[0].tempoDeExec;
			//int tempoDeExecEspera = FilaDeProcessos[i].tempoDeExec - FilaDeProcessos[i].entrada;
		}
		//int tempoDeExecTotal += tempoDeExecEspera;
	}
}

void sjf()
{
	printf("\n================ SJF =================\n");
	sortByTempoExec();
	systemEscalonatorExecFIFOandSJF();
};

void fifo()
{
	printf("\n================ FIFO =================\n");
	
	sortByTempoDeChegada();
	systemEscalonatorExecFIFOandSJF();
};


int main(){
	srand(time(0));
	initFreeFrameList();
	initRAM();
	int process1 = initNewProcess(0, 0, 2, 0, 0, 0);
	runProcess(&FilaDeProcessos[0], 0);
	int process2 = initNewProcess(0, 3, 5, 4, 2, 0);
	int process3 = initNewProcess(0, 3, 9, 2, 0, 0);
	int process4 = initNewProcess(0, 1, 6, 1, 0, 0);
	int process5 = initNewProcess(0, 2, 2, 8, 0, 0);
	int process6 = initNewProcess(0, 8, 1, 0, 0, 0);
	if(debug == true){
		debugFunction();
	}
	runProcess(&FilaDeProcessos[0],10);
	debugFunction();
	fifo();
	sjf();
}
