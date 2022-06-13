#include "codigo.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define TAMFILAEXECUCAO 30
#define TAMFILAPROCESSOS 10
#define NUM_PAG_P_PROCESSOS 10
#define TAM_DISCO 200
#define TAM_RAM 50
#define TAMCHARTDRAWERARRAY 500

bool debug = false;
static unsigned int indexDoDisco = 0;

static process FilaDeProcessos[TAMFILAPROCESSOS];

static DEntry Disco[TAM_DISCO];
static FTEntry FreeFrameList[TAM_RAM];
static DEntry RAM[TAM_RAM];
static int paginacaoFIFOindex = 0;
static tuple_TP_chart ChartDrawerrArray[TAMCHARTDRAWERARRAY];

void debugFunction() {
  printf("\nAtual estado do Disco:");
  printf("\n[");
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    printf("\nProcesso %d: \n\texited:%i\n\tTempo de exec:%d\n\t\n\tEndereço "
           "do processo:%p",
           FilaDeProcessos[i].processId, FilaDeProcessos[i].exited,
           FilaDeProcessos[i].tempoDeExec, &FilaDeProcessos[i]);
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
int initChartDrawerArray() {
  tuple_TP_chart aux;
  aux.proc = 0;
  aux.tick = 0;
  for (int i = 0; i < TAMCHARTDRAWERARRAY; i++) {
    ChartDrawerrArray[i] = aux;
  }
}
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

bool alocado(int ponteiro) {
  return ((FilaDeProcessos[ponteiro].processId > 0) &&
          (FilaDeProcessos[ponteiro].exited == false));
}

bool disponivel(int ponteiro, int tick) {
  return (alocado(ponteiro) &&
          FilaDeProcessos[ponteiro].tempoDeChegada <= tick);
}

bool estaNaFila(process *ArrayTeste[], process *Processo, int inicioFila,
                int fimFila) {
  for (int i = inicioFila; i < fimFila; i++) {
    if (Processo->processId == ArrayTeste[i]->processId) {
      return true;
    }
  }
  return false;
}

void add_to_drawerArray(int tick, int process) {
  for (int i = 0; i < TAMCHARTDRAWERARRAY; i++) {
    if (ChartDrawerrArray[i].proc == 0) {
      tuple_TP_chart aux;
      aux.proc = process;
      aux.tick = tick;
      ChartDrawerrArray[i] = aux;
      return;
    }
  }
}

int ExecuteRoundRobin(int quantum, int sobrecarga) {
  unsigned int ponteiro = 0;
  unsigned int actualTick = 0;
  unsigned int contadorDeTempo = 0;
  int lastProcess = -1;
  process **FilaDeExecucao = malloc(sizeof(process *) * TAMFILAEXECUCAO);
  int inicioFila = 0;
  int fimFila = 0;
  while (true) {
    ponteiro = ponteiro % TAMFILAPROCESSOS;
    inicioFila = inicioFila % TAMFILAEXECUCAO;
    fimFila = fimFila % TAMFILAEXECUCAO;
    // CHECA SE A FILA ESTÁ VAZIA
    for (int i = 0; i < TAMFILAPROCESSOS; i++) {
      if (!alocado(i)) {
        if (i == 9) {
          printf("\nTERMINADO");
          return actualTick;
        }
        continue;
      }
      break;
    }

    // O CÓDIGO ACIMA TENTA TERMINAR A FILA SEMPRE QUE RODA. PODE IGNORAR
    for (int intPonteiro = 0; intPonteiro < TAMFILAPROCESSOS; intPonteiro++) {
      if (disponivel(intPonteiro, actualTick) &&
          !estaNaFila(FilaDeExecucao, &FilaDeProcessos[intPonteiro], inicioFila,
                      fimFila)) {
        { // VERIFICA SE O PROCESSO JA PODE SER RODADO
          FilaDeExecucao[fimFila] = &FilaDeProcessos[intPonteiro];
          fimFila++;
        }
      }
    }

    if (disponivel(FilaDeExecucao[inicioFila]->processId - 1,
                   actualTick)) { // UM HACKZINHO processId - 1  = indexNa fila
                                  // de processos
      runProcess(FilaDeExecucao[inicioFila], actualTick);
      add_to_drawerArray(actualTick, FilaDeExecucao[inicioFila]->processId);
      // printf("\n incioDaFila:%d %i", inicioFila, (contadorDeTempo));
      if (FilaDeExecucao[inicioFila]->exited ||
          (contadorDeTempo == quantum - 1)) {
        if ((contadorDeTempo == quantum - 1) &&
            !(FilaDeExecucao[inicioFila]->exited)) {
          printf("\nColocando %d no final da fila",
                 FilaDeExecucao[inicioFila]->processId);
          FilaDeExecucao[fimFila] =
              &FilaDeProcessos[(FilaDeExecucao[inicioFila]->processId - 1)];
          fimFila++;
        }
        contadorDeTempo = -1;
        inicioFila++;
        actualTick = actualTick + sobrecarga;
      }
      actualTick++;
      contadorDeTempo++;
    }

    if (debug) {
      printf("\ndps da op: inicioFila: %d; fimFila: %d tick: %d\nFila de "
             "Execucao: [",
             inicioFila, fimFila, actualTick);
      for (int u = inicioFila; u < fimFila; u++) {
        printf("%d, ", FilaDeExecucao[u]->processId);
      }
      printf("]");
    }
  }
}

int ExecuteEDF(int sobrecarga) {
  unsigned int ponteiro = 2;
  unsigned int actualTick = 0;
  int lastProcess = -1;
  while (true) {
    ponteiro = 0;
    // CHECA SE A FILA ESTÁ VAZIA
    for (int i = 0; i < TAMFILAPROCESSOS; i++) {
      if (!alocado(i)) {
        if (i == 9) {
          printf("\nTERMINADO");
          return actualTick;
        }
        continue;
      }
      break;
    }
    // O CÓDIGO ACIMA TENTA TERMINAR A FILA SEMPRE QUE RODA. PODE IGNORAR
    for (int i = 0; i < TAMFILAPROCESSOS; i++) {
      if (FilaDeProcessos[ponteiro].exited == true) {
        ponteiro = i;
        continue;
      }
      if (disponivel(i, actualTick)) { // VERIFICA SE O PROCESSO ESTÁ ALOCADO
        if (FilaDeProcessos[i].deadline < FilaDeProcessos[ponteiro].deadline) {
          ponteiro = i;
        }
      }
    }

    printf("\nProcesso %d foi selecionado", ponteiro + 1);
    if (alocado(ponteiro)) { // VERIFICA SE O PROCESSO ESTÁ ALOCADO
      printf("\nProcesso %dfoi selecionado",
             FilaDeProcessos[ponteiro].processId);
      if (FilaDeProcessos[ponteiro].tempoDeChegada <=
          actualTick) { // VERIFICA SE O PROCESSO JA PODE SER RODADO
        if (actualTick == 0) {
          lastProcess = FilaDeProcessos[ponteiro].processId;
        }
        if (lastProcess != FilaDeProcessos[ponteiro].processId) {
          actualTick = actualTick + sobrecarga;
          lastProcess = FilaDeProcessos[ponteiro].processId;
        }
        runProcess(&FilaDeProcessos[ponteiro], actualTick);
        add_to_drawerArray(actualTick, FilaDeProcessos[ponteiro].processId);
        actualTick++;
      }
    }
  }
}

void sortByTempoDeChegada() {
  int i, j;
  struct Processos temp;

  for (i = 0; i < TAMFILAPROCESSOS - 1; i++) {
    for (j = i + 1; j < TAMFILAPROCESSOS; j++) {
      if (FilaDeProcessos[i].tempoDeChegada >
          FilaDeProcessos[j].tempoDeChegada) {
        temp = FilaDeProcessos[i];
        FilaDeProcessos[i] = FilaDeProcessos[j];
        FilaDeProcessos[j] = temp;
      }
    }
  }
}

void sortByTempoExec() {
  int size = (sizeof FilaDeProcessos) / (sizeof *FilaDeProcessos);
  int i, j;
  struct Processos temp;
  sortByTempoDeChegada();
  for (i = 0; i < size - 1; i++) {
    for (j = i + 1; j < size; j++) {
      if (FilaDeProcessos[i].tempoDeChegada ==
          FilaDeProcessos[j].tempoDeChegada) {
        if (FilaDeProcessos[i].tempoDeExec > FilaDeProcessos[j].tempoDeExec) {
          temp = FilaDeProcessos[i];
          FilaDeProcessos[i] = FilaDeProcessos[j];
          FilaDeProcessos[j] = temp;
        }
      }
    }
  }
}

void systemEscalonatorExecFIFOandSJF() {
  int size = sizeof FilaDeProcessos / sizeof *FilaDeProcessos;
  int delay;
  int tempoEspera = 0;
  int turn = 0;

  for (int i = 0; i < size; i++) {
    if (FilaDeProcessos[i].processId > 0 &&
        FilaDeProcessos[i].exited ==
            false) { // VERIFICA SE O PROCESSO ESTÃ ALOCADO
                     // printf("\nProcesso %d foi selecionado \n",
      // FilaDeProcessos[i].processId); FilaDeProcessos[i].processId = 1;
      for (int j = 0; j < turn; ++j) {
        putchar(' ');
      }
      for (int j = 0; j < FilaDeProcessos[i].tempoDeExec; ++j) {
        putchar('#');
        sleep(1);
      }
      printf("\n");
      if (i > 0) {
        if (FilaDeProcessos[i].tempoDeChegada <= turn) {
          turn += (FilaDeProcessos[i].tempoDeExec);
          delay = FilaDeProcessos[i].tempoDeExec;
        } else {
          delay = (FilaDeProcessos[i].tempoDeChegada - turn) +
                  FilaDeProcessos[i].tempoDeExec;
          turn += delay;
        }
        sleep(delay);
        // printf("\nProcesso %d foi executado em: %d s \n",
        // FilaDeProcessos[i].processId, turn);
      }
    }
  }

  printf("%d", turn);
}
void sjf() {
  printf("\n================ SJF =================\n");
  sortByTempoExec();
  systemEscalonatorExecFIFOandSJF();
};

void fifo() {
  printf("\n================ FIFO =================\n");
  sortByTempoDeChegada();
  systemEscalonatorExecFIFOandSJF();
};

void repeatLine(int numDeRepeticoes) {
  for (int i = 0; i < numDeRepeticoes; i++) {
    printf(" ");
  }
}

void ganttChartDrawer() {
  // system("clear");
  for (int i = 0; i < TAMFILAPROCESSOS; i++) {
    if (FilaDeProcessos[i].exited == true) {
      printf("\nProcesso %d:", i + 1);
      int lastTick = 0;
      for (int j = 0; j < TAMCHARTDRAWERARRAY; j++) {
        if (ChartDrawerrArray[j].proc == i + 1) {
          int delta = ChartDrawerrArray[j].tick - lastTick;
          if (delta == 1)
            delta = 0;
          repeatLine(delta);
          lastTick = ChartDrawerrArray[j].tick;
          printf("#");
        }
      }
    }
  }
}

// INICIAR PROCESSO POR ENTRADAS

int main() {
  // srand(time(0));
  initFreeFrameList();
  initFilaDeProcessos();
  initChartDrawerArray();
  initRAM();
  printf("\nNúmero de processos a serem iniciados (max 10): \n");
  int num = 0;
  scanf("%d", &num);
  for (int i = 0; i < num; i++) {
    int tempoDeChegada, tempoDeExec, deadline, prioridade;
    printf("Coloque em ordem: tempo de chegada, tempo de execução, deadline, "
           "prioridade\n");
    scanf("%d %d %d %d", &tempoDeChegada, &tempoDeExec, &deadline, &prioridade);
    initNewProcess(tempoDeChegada, tempoDeExec, deadline, prioridade);
  }
  int scan = 0;
  printf("\nEscolha entre as opções:\n0 -> EDF\n1 -> RoundRobin\n2 -> SJF\n3 "
         "-> Fifo\n");
  scanf("%d", &scan);
  switch (scan) {
  case 0: {
    int sobrecarga;
    printf("\n sobrecarga:\n");
    scanf("%d", &sobrecarga);
    ExecuteEDF(sobrecarga);
    ganttChartDrawer();
    break;
  }
  case 1: {
    int sobrecarga, quantum;
    printf("\n sobrecarga, quantum\n");
    scanf("%d %d", &sobrecarga, &quantum);
    ExecuteRoundRobin(quantum, sobrecarga);
    ganttChartDrawer();
    break;
  }
  case 2: {
    sjf();
    break;
  }
  case 3: {
    fifo();
    break;
  }
  }
  if (debug) {
    debugFunction();
  }
}
