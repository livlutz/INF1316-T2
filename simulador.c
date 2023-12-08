// Arthur Augusto Claro Sardella - 2212763 - 3WA
// Lívia Lutz dos Santos - 2211055 - 3WB

#define SIM 1

// REMOVER ANTES DE ENTREGAR
//#define DEBUG 1

#include "simulador.h"

struct pagina {
    unsigned int addressPhis; // XXXXXXXX XXXXXXXX XXX00000 00000000
    int time; // tempo de ultimo acesso
    int A; //flag de pagina na memoria
    int R; //flag de pagina referenciada
    int M; //flag de pagina modificada
};

struct quadro {
    int num; //se -1 -> livre
    Pagina* p; //ponteiro para a pagina
};

Pagina* pagTable;
Quadro* frameTable;

int sizeFrameTable, sizePageTable;
int pageSize, memSize, mode;
int pageFaults = 0, writtenPages = 0;

//contador simulando a passagem do tempo
int clock = 0;

#if DEBUG
int main(void) {
    int argc = 5;
    char* argv[5] = {"", "NRU", "C:\\Users\\Doctor Christ\\Documents\\inf1316-T2\\simulador.log", "16", "1"};
#else
int main(int argc, char* argv[]) {

#endif
    if (argc != 5) {
        error("Uso apropriado do programa: .\\sim-virtual (LRU || NRU) (8 || 16) (1..4)\n")
    }

    if check_mode(argv[1]) {
        error("Escolha entre os modos NRU ou LRU\n")
    }

    //Pega o algoritmo de substituicao de paginas
    mode = strcmp(argv[1], "NRU") == 0 ? 0 : 1;

    //Pega o tamanho das paginas em Kb
    pageSize = atoi(argv[3]);

    if check_size(pageSize) {
        error("As paginas tem que ter tamanho 8 Kbytes ou 16 Kbytes\n")
    }

    //Pega o tamanho da memoria fisica em Mb
    memSize = atoi(argv[4]);
    if check_mem(memSize) {
        error("A memoria fisica tem de ser entre 1Mbytes e 4Mbytes\n")
    }

    //Abre o arquivo de entrada
    FILE* arq = fopen(argv[2], "r");
    if (arq == NULL) {
        error("Arquivo nao encontrado\n")
    }

    printf("Executando o simulador...\n");

    unsigned int address, page, offset = 0, physAddr;

    //guarda o tipo de acesso (R ou W)
    char accessType;

    //calcula a quantidade de bits menos significativos
    int shift = calculaShift();
    createTables();

    //Le o arquivo de entrada
    while (!feof(arq)) {
        //guarda o endereco e o tipo de acesso
        fscanf(arq, "%x %c ", &address, &accessType);

        //calcula o indice da pagina (endereco logico) descartando os bits menos significativos
        page = address >> shift;
        for (int i = 0; i < shift; i++)
            offset += address & (0x1 << i);

        //calcula o endereco fisico
        physAddr = getPhysAddr(page, offset, accessType);

        //incrementa o contador do simulador
        clock++;
        if (clock % 8045 == 0)
            resetReference();
    }

    fclose(arq);

    //formata a saida da simulacao
    printf("Arquivo de entrada: %s\n", argv[2]);
    printf("Tamanho da memoria fisica: %d MB\n", memSize);
    printf("Tamanho das paginas: %d KB\n", pageSize);
    printf("Algoritmo de substituicao: %s\n", argv[1]);
    printf("Numero de Faltas de Paginas: %d\n", pageFaults);
    printf("Numero de Paginas Escritas: %d\n", writtenPages);

    return 0;
}

void createTables() {
    //calcula o deslocamento com base no tamanho das paginas
    int exp = calculaShift(pageSize);

    //calcula o tamanho das tabelas de paginas
    int qtd = TOTAL_BITS_SIZE - exp;
    sizePageTable = pow(2, qtd);
    pagTable = (Pagina*)malloc(sizeof(Pagina) * sizePageTable);
    
    if (pagTable == NULL) {
        error("Espaco de memoria insuficiente\n")
    }

    //Inicializa os campos de pagina
    for (int i = 0; i < sizePageTable; i++) {
        (pagTable + i)->addressPhis = 0;
        (pagTable + i)->time = 0;
        (pagTable + i)->A = 0;
        (pagTable + i)->R = 0;
        (pagTable + i)->M = 0;
    }

    //calcula o tamanho da tabela de quadros
    sizeFrameTable = (pow(2, ONE_MB_EXP) * memSize) / pow(2, exp);
    frameTable = (Quadro*)malloc(sizeof(Quadro) * sizeFrameTable);

    if (frameTable == NULL) {
        error("Espaco de memoria insuficiente\n")
    }

    //Inicializa os campos de quadro
    for (int i = 0; i < sizeFrameTable; i++) {
        (frameTable + i)->num = -1;
        (frameTable + i)->p = NULL;
    }
}

unsigned int getPhysAddr(unsigned int index, unsigned int offset, char accessType) {
    Pagina* page = pagTable + index;
    unsigned int addr;
    int freeFrame = -1;
    
    // checa se presente
    if (!(page->A)) {
        // se nao, conta mais 1 page fault
        pageFaults++;

        // acha um frame livre
        for (int i = 0; i < sizeFrameTable; i++) {
            if (frameTable[i].num == -1) {
                freeFrame = i;
                frameTable[i].num = i;
                frameTable[i].p = page;
                break;
            }
        }

        // se nao tiver frame livre -> swap
        if (freeFrame == -1) {
            // libera 1 e devolve numero do frame || swap();
            if (mode) {
                int i = LRU();
                freeFrame = i;
                frameTable[i].num = i;
                frameTable[i].p = page;
            }
                
            else {
                int i = NRU();
                freeFrame = i;
                frameTable[i].num = i;
                frameTable[i].p = page;
            }
        }

        // guarda endereço fisico inicial do frame na pagina
        page->addressPhis = frameTable[freeFrame].num << calculaShift();
    }

    //Se a pagina for aberta para escrita -> seta a flag de escrita
    if (accessType == 'W') {
        page->M = 1;
    }

    //Senao seta a flag de leitura
    page->R = 1;

    //Atualiza os campos e endereco da proxima pagina
    page->time = clock;
    page->A = 1;
    addr = page->addressPhis + offset;

    return addr;
}

int calculaShift() {
    int base_shift = ONE_KB_EXP;
    double plus_shift = log2(pageSize);
    return base_shift + (int) plus_shift;
}

int LRU(){
    /*Princípio: descartar a página que ficou sem acesso durante o
        periodo de tempo mais longo
        Pegar o time de cada pagina e ver qual tem o time menor e descarta-la*/
    int smallestSize = INT_MAX, indexBS = 0;
    
    Pagina* maior = frameTable[0].p;

    //Se o tempo de acesso for < que o menor tempo, pega a pagina
    for(int i = 0; i < sizeFrameTable; i++){
        if(frameTable[i].p->time < smallestSize){
            smallestSize = frameTable[i].p->time;
            indexBS = i;
            maior = frameTable[i].p;
        }
    }
    //Seta a flag de modificado para 0 e aumenta a qtd de paginas sujas
    if (maior->M) {
        writtenPages++;
        maior->M = 0;
    }
    
    //Seta as flags de presenca e leitura para 0
    maior->A = 0;
    maior->R = 0;

    //Inicializa os campos da pagina descartada
    frameTable[indexBS].p = NULL;
    frameTable[indexBS].num = -1;

    //Retorna o indice da pagina descartada
    return indexBS;
}

int NRU (){
    /*Verificar bits R E M
    Prioridade de se manter na memoria:
    R | M
    1   1
    1   0
    0   1
    0   0  
    prioridade de ser descartado vai de baixo pra cima*/

    Pagina *atual = NULL, *descartado = NULL;
    int i, indDescart = 0;

    //Compara as flags R e M da pagina atual
    for(i = 0; i < sizeFrameTable; i++){

        atual = frameTable[i].p;

        if(atual->R == 0 && atual->M == 0){
            descartado = atual;
            indDescart = i;
            break;
        }
        else if(atual->R == 0 && atual->M == 1){
            if (descartado == NULL) {
                descartado = atual;
                indDescart = i;
                continue;
            }

            if (descartado->R) {
                descartado = atual;
                indDescart = i;
            }
        }
        else if(atual->R == 1 && atual->M == 0){
            if (descartado == NULL) {
                descartado = atual;
                indDescart = i;
                continue;
            }

            if (descartado->R && descartado->M) {
                descartado = atual;
                indDescart = i;
            }
        }
        else {
            if (descartado == NULL) {
                descartado = atual;
                indDescart = i;
            }
        }
    }

    //Se a pagina descartada for modificada, aumenta a qtd de paginas sujas e seta a flag M para 0
    if (descartado->M) {
        writtenPages++;
        descartado->M = 0;
    }

    //Seta as flags de presenca e leitura para 0
    descartado->A = 0;
    descartado->R = 0;

    //Inicializa os campos da pagina descartada
    frameTable[indDescart].p = NULL;
    frameTable[indDescart].num = -1;

    //Retorna o indice da pagina descartada
    return indDescart;
}

void resetReference() {
    for (int i = 0; i < sizeFrameTable; i++)
        if (frameTable[i].p != NULL)
            frameTable[i].p->R = 0;
    return;
}
