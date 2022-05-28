#include "codigo.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

void debugFunction() {
  printf("\nAtual estado do Disco:");
  printf("\n[");
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    printf("\nProcesso %d: \n\texited:%i\n\tTempo de exec:%d\n\t",
           FilaDeProcessos[i].processId, FilaDeProcessos[i].exited,
           FilaDeProcessos[i].tempoDeExec);
  }
  printf("]");
  printf("\nAtual estado das páginas de cada processo:");
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    if (FilaDeProcessos[i].processId > 0) {
      printf("\nProcesso %d", FilaDeProcessos[i].processId);
      for (int j = 0; j < NUM_PAG_P_PROCESSOS; j++) {
        printf("\n\tPágina %d "
               "está:\n\t\tcarregada:%i\n\t\tdados:%i\n\t\tFTEPointer:%p",
               FilaDeProcessos[i].pageTable[j].pageId,
               FilaDeProcessos[i].pageTable[j].carregada,
               FilaDeProcessos[i].pageTable[j].dados->dados,
               FilaDeProcessos[i].pageTable[j].FTEPointer);
      }
    }
  }
  printf("\nAtual estado da RAM:");
  printf("\n[");
  for (int i = 0; i < TAM_RAM; i++) {
    printf("\n\tDados:%d\n\tUsed:%i", RAM[i].dados, RAM[i].used);
  }
  printf("]");

  printf("\nAtual estado do Disco:");
  printf("\n[");
  for (int i = 0; i < 30; i++) {
    printf("\n\tDados:%d\n\tUsed:%i", Disco[i].dados, Disco[i].used);
  }
  printf("]");
}

DEntry *addToDisc(int dados) {
  for (int i = 0; i < TAM_DISCO; i++) {
    if (Disco[i].used == false) {
      DEntry newDEntry;
      newDEntry.used = true;
      newDEntry.dados = dados;
      Disco[i] = newDEntry;
      return &Disco[i];
    }
  }
}

// implementing FIFO
void paginacaoFIFO() {
  // pega o ponteiro da pagina que a FTL está gerenciando
  page *paginaASair = FreeFrameList[paginacaoFIFOindex % 50].endereco;
  if (debug) {
    printf("\nEndereço da pagina a ser liberada %p\n", paginaASair);
  }
  FreeFrameList[paginacaoFIFOindex % 50].loaded = false;
  FreeFrameList[paginacaoFIFOindex % 50].lastTickUsed = 0;

  // desmarca a pagina como carregada
  paginaASair->carregada = false;
  paginaASair->dados = addToDisc(paginaASair->dados->dados);
  paginaASair->FTEPointer = NULL;
  paginacaoFIFOindex = paginacaoFIFOindex + 1;
}

void paginacaoLRU() {
  printf("liberou uma página\n");
  // pega o ponteiro da pagina que a FTL está gerenciando
  int minContador = 0;
  int ponteiroMaiorContagem = 0;
  for (int i = 0; i < TAM_RAM; i++) {
    if (FreeFrameList[i].lastTickUsed < minContador) {
      minContador = FreeFrameList[i].lastTickUsed;
      ponteiroMaiorContagem = i;
    }
  }
  page *paginaASair = FreeFrameList[ponteiroMaiorContagem].endereco;
  FreeFrameList[ponteiroMaiorContagem].loaded = false;
  FreeFrameList[ponteiroMaiorContagem].lastTickUsed = 0;

  // desmarca a pagina como carregada
  paginaASair->carregada = false;
  Disco[indexDoDisco] = *(paginaASair->dados);
  paginaASair->dados = &Disco[indexDoDisco];
  paginaASair->FTEPointer = NULL;
  indexDoDisco++;
}

DEntry *addToRam(DEntry *EndRam, page *pagina, int dados) {
  DEntry newDEntry;
  newDEntry.used = true;
  newDEntry.dados = dados;
  *(EndRam) = newDEntry;
  return EndRam;
}

// addPageToRam: Busca uma entrada livre na FrameList, se acha, insere nela
void addPageToRam(page *pagina, unsigned int dados) {
  for (int i = 0; i < TAM_RAM; i++) {
    if (FreeFrameList[i].loaded == false) {
      DEntry *refDisc = NULL;
      if (pagina->carregada == false && pagina->dados != NULL) {
        refDisc = pagina->dados;
      }
      if (debug) {
        printf("\npágina %d, carregada: %i, endDosDados:%p, refDisc: %p",
               pagina->pageId, pagina->carregada, pagina->dados, refDisc);
      }
      pagina->dados = addToRam(&RAM[i], pagina, dados);

      pagina->FTEPointer = &FreeFrameList[i];
      pagina->carregada = true;
      FreeFrameList[i].loaded = true;
      FreeFrameList[i].endereco = pagina;
      FreeFrameList[i].lastTickUsed = 0;

      // LIMPA O ESPAÇO NO DISCO QUE FICOU ALOCADO PARA UMA PÁGINA QUE VAI SER
      // RECARREGADA
      if (refDisc != NULL) {
        refDisc->dados = 0;
        refDisc->used = false;
      }
      break;
      return;
    }
    if (i == 49) {
      paginacaoFIFO();
      i = -1;
    }
  }
}

void initRAM() {
  for (int i = 0; i < TAM_RAM; i++) {
    DEntry newDEntry;
    newDEntry.dados = 0;
    newDEntry.used = false;
    RAM[i] = newDEntry;
  }
}

void initFilaDeProcessos() {
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    process newProcess;
    newProcess.exited = false;
    newProcess.pageTable = NULL;
    newProcess.deadline = 0;
    newProcess.processId = 0;
    newProcess.sobrecarga = 0;
    newProcess.tempoDeChegada = 0;
    newProcess.tempoDeExec = 0;
    newProcess.prioridade = 0;
    FilaDeProcessos[i] = newProcess;
  }
}

// inicia FreeFrameList com as FTEntry necessárias

void initFreeFrameList() {
  for (int i = 0; i < TAM_RAM; i++) {
    FTEntry newFTEntry;
    newFTEntry.loaded = false;
    newFTEntry.endereco = NULL;
    newFTEntry.lastTickUsed = 0;
    FreeFrameList[i] = newFTEntry;
  }
}

// initNewPage deve retornar o id da nova página não modificada nem carregada,
// se não foi criada, retorna um valor <0.
void initNewPage(process *processo, int pageId, int dados) {
  page newPage;
  newPage.pageId = pageId;
  newPage.endereco = 0;
  newPage.carregada = false;
  newPage.dados = NULL;
  processo->pageTable[pageId] = newPage;
  addPageToRam(&processo->pageTable[pageId], dados);
}

void runProcess(process *process,
                int currentTick) { // Não chamar runProcess em processos
                                   // fechados (exited == 1)
  for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++) {
    if (process->pageTable[i].carregada == false) {
      if (debug) {
        printf("\nProcesso %d não pode ser executado porque a página %d não "
               "foi carregada tentando por a página na ram",
               process->processId, process->pageTable[i].pageId);
      }
      addPageToRam(&process->pageTable[i], process->pageTable[i].dados->dados);
      runProcess(process, currentTick);
      return;
    }
  }
  int numRandom = rand() % 10;
  printf("\nProcesso %d foi executado com sucesso utilizando a página %d",
         process->processId, numRandom);
  process->tempoDeExec = process->tempoDeExec - 1;
  process->pageTable[numRandom].FTEPointer->lastTickUsed = currentTick;

  if (process->tempoDeExec <= 0) {
    process->exited = true;
  }

  // PEGA UMA PÁGINA ALEATÓRIA PARA O PROCESSO ACESSAR
}

// initNewProcess deve retornar o id do novo processo criado. retorna <0 caso o
// processo não tenha sido criado.

int initNewProcess(int tempoDeChegada, int tempoDeExec, int deadline,
                   int prioridade) {
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    if (FilaDeProcessos[i].processId == 0) {
      process newProcess;
      newProcess.exited = false;
      newProcess.processId = i + 1;
      newProcess.deadline = deadline;
      newProcess.tempoDeChegada = tempoDeChegada;
      newProcess.tempoDeExec = tempoDeExec;
      newProcess.prioridade = prioridade;

      newProcess.pageTable = malloc(sizeof(page) * NUM_PAG_P_PROCESSOS);
      for (int i = 0; i < NUM_PAG_P_PROCESSOS; i++) {
        initNewPage(&newProcess, i, rand());
      }

      FilaDeProcessos[i] = newProcess;
      return i + 1;
    }
  }
}

int ExecuteRoundRobin(int quantum, int sobrecarga) {
  bool breakflag = false;
  unsigned int ponteiro = 0;
  unsigned int actualTick = 0;
  unsigned int contadorDeTempo = 0;
  int lastProcess = -1;
  while (true) {
    ponteiro = ponteiro % TAMFILAPROCESSOS;
    // CHECA SE A FILA ESTÁ VAZIA
    for (int i = 0; i < TAMFILAPROCESSOS; i++) {
      if (FilaDeProcessos[i].exited == true ||
          FilaDeProcessos[i].processId == 0) {
        if (i == 9) {
          printf("TERMINADO");
          breakflag = true;
          return actualTick;
        }
        continue;
      }
      break;
    }
    if (breakflag) {
      break;
    }
    // O CÓDIGO ACIMA TENTA TERMINAR A FILA SEMPRE QUE RODA. PODE IGNORAR
    if (FilaDeProcessos[ponteiro].processId > 0 &&
        FilaDeProcessos[ponteiro].exited ==
            false) { // VERIFICA SE O PROCESSO ESTÁ ALOCADO
      if (FilaDeProcessos[ponteiro].tempoDeChegada <=
          actualTick) { // VERIFICA SE O PROCESSO JA PODE SER RODADO
        if (FilaDeProcessos[ponteiro].processId == lastProcess) {
          contadorDeTempo++;
        } else {
          if (lastProcess != -1) {
            printf("\nsobrecarga");
            actualTick = actualTick + sobrecarga;
          }
        }
        // O CODIGO ACIMA CONTA O TEMPO QUE CADA PROCESSO AINDA TEM NO
        // PROCESSADOR, TAMBÉM TOMA CONTA DA SOBRECARGA
        runProcess(&FilaDeProcessos[ponteiro], actualTick);
        printf("\nProcesso %d; Tick:%d", FilaDeProcessos[ponteiro].processId,
               actualTick);
        lastProcess = FilaDeProcessos[ponteiro].processId;
        if ((contadorDeTempo == quantum - 1) ||
            (FilaDeProcessos[ponteiro].exited == true)) {
          contadorDeTempo = 0;
          ponteiro++;
        }
        actualTick++;
      } else {
        if (ponteiro == lastProcess - 1) {
          printf("\nProcesso %d ainda não pode ser rodado, estamos no tick:%d, "
                 "e ele está em: %d,ult. process:%d",
                 FilaDeProcessos[ponteiro].processId, actualTick,
                 FilaDeProcessos[ponteiro].tempoDeChegada, lastProcess);
          actualTick++;
        }
        ponteiro++;
      }
    } else {
      if (ponteiro == lastProcess - 1) {
        printf("\ncond 2 Processo %d não pode ser rodado, estamos no tick:%d, "
               "e ele está em: %d,ult. process:%d",
               FilaDeProcessos[ponteiro].processId, actualTick,
               FilaDeProcessos[ponteiro].tempoDeChegada, lastProcess);
        actualTick++;
      }
      ponteiro++;
    }
  }
}

int ExecuteEDF(int sobrecarga) {
  bool breakflag = false;
  unsigned int ponteiro = 2;
  unsigned int actualTick = 0;
  int lastProcess = -1;
  while (true) {
    ponteiro = 0;
    // CHECA SE A FILA ESTÁ VAZIA
    for (int i = 0; i < TAMFILAPROCESSOS; i++) {
      if (FilaDeProcessos[i].exited == true ||
          FilaDeProcessos[i].processId == 0) {
        if (i == 9) {
          //printf("\nEDF Terminado");
          breakflag = true;
          return actualTick;
        }
        continue;
      }
      break;
    }
    if (breakflag) {
      break;
    }
    // O CÓDIGO ACIMA TENTA TERMINAR A FILA SEMPRE QUE RODA. PODE IGNORAR
    for (int i = 0; i <TAMFILAPROCESSOS; i++) {
      if(FilaDeProcessos[ponteiro].exited ==  true){ ponteiro = i; continue;}
      if (FilaDeProcessos[i].processId > 0 &&
          FilaDeProcessos[i].exited ==
              false) { // VERIFICA SE O PROCESSO ESTÁ ALOCADO
        //printf("\nProcesso %d sendo testado\n\tExited?:%i", FilaDeProcessos[i].processId, FilaDeProcessos[i].exited);
        if (FilaDeProcessos[i].tempoDeChegada <=
            actualTick) { // VERIFICA SE O PROCESSO JA PODE SER RODADO
          if (FilaDeProcessos[i].deadline <
              FilaDeProcessos[ponteiro].deadline) {
            ponteiro = i;
          }
        }
      }
    }
    printf("\nProcesso %d foi selecionado", ponteiro+1);
    if (FilaDeProcessos[ponteiro].processId > 0 &&
        FilaDeProcessos[ponteiro].exited ==
            false) { // VERIFICA SE O PROCESSO ESTÁ ALOCADO
      printf("\nProcesso %dfoi selecionado",
             FilaDeProcessos[ponteiro].processId);
      if (FilaDeProcessos[ponteiro].tempoDeChegada <=
          actualTick) { // VERIFICA SE O PROCESSO JA PODE SER RODADO
        if (lastProcess != FilaDeProcessos[ponteiro].processId) {
          actualTick = actualTick + sobrecarga;
        }
        runProcess(&FilaDeProcessos[ponteiro], actualTick);
        actualTick++;
      }
    }
  }
}


int main() {
  // srand(time(0));
  initFreeFrameList();
  initFilaDeProcessos();
  initRAM();
  int process1 = initNewProcess(0, 1, 2, 0);
  int process2 = initNewProcess(0, 3, 5, 4);
  int process3 = initNewProcess(0, 3, 9, 2);
  int process4 = initNewProcess(0, 1, 6, 1);
  int process5 = initNewProcess(0, 2, 2, 8);
  int process6 = initNewProcess(0, 8, 1, 0);
  ExecuteEDF(5);
  // int process2 = initNewProcess(0, 0, 0, 0, 0, 0);
  // int process3 = initNewProcess(0, 0, 0, 0, 0, 0);
  // int process4 = initNewProcess(0, 0, 0, 0, 0, 0);
  // int process5 = initNewProcess(0, 0, 0, 0, 0, 0);
  // int process6 = initNewProcess(0, 0, 0, 0, 0, 0);
  if (debug) {
    debugFunction();
  }
}
