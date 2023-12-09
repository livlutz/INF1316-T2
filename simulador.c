// Arthur Augusto Claro Sardella - 2212763 - 3WA
// Lívia Lutz dos Santos - 2211055 - 3WB


#include "simulador.h"

struct pagina {
    int time; // tempo de ultimo acesso
    int A; //flag de pagina na memoria
    int R; //flag de pagina referenciada
    int M; //flag de pagina modificada
};

//Array de paginas
Pagina* pagTable;

int sizePageTable, qtdFreeSpace;
int pageSize, memSize, mode;
int pageFaults = 0, writtenPages = 0;

//contador simulando a passagem do tempo
int clock = 0;

int main(int argc, char* argv[]) {

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

    unsigned int address, page;

    //guarda o tipo de acesso (R ou W)
    char accessType;

    //calcula a quantidade de bits menos significativos
    int shift = calculaShift();
    createTable();

    //Le o arquivo de entrada
    while (!feof(arq)) {
        //guarda o endereco e o tipo de acesso
        fscanf(arq, "%x %c ", &address, &accessType);

        //calcula o indice da pagina (endereco logico) descartando os bits menos significativos
        page = address >> shift;

        //faz os algoritmos de substituicao de paginas
        loadPage(page, accessType);

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

void createTable() {
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
        (pagTable + i)->time = 0;
        (pagTable + i)->A = 0;
        (pagTable + i)->R = 0;
        (pagTable + i)->M = 0;
    }

    //Calcula quantidade de paginas que podem estar em memoria ao mesmo tempo
    qtdFreeSpace = (pow(2, ONE_MB_EXP) * memSize) / pow(2, exp);
}

void loadPage(unsigned int index, char accessType) {
    Pagina* page = pagTable + index;
    unsigned int addr;
    
    // checa se presente
    if (!(page->A)) {
        // se nao, conta mais 1 page fault
        pageFaults++;
        if (qtdFreeSpace <= 0) {
            qtdFreeSpace++;
            // libera uma pagina
            if (mode) {
                LRU();
            }

            else {
                NRU();
            }
        }
        qtdFreeSpace--;

        page->A = 1;
    }

    //Se a pagina for aberta para escrita -> seta a flag de escrita
    if (accessType == 'W') {
        page->M = 1;
    }

    //Seta a flag de referencia
    page->R = 1;

    //Atualiza os campos e endereco da proxima pagina
    page->time = clock;
}

int calculaShift() {
    int base_shift = ONE_KB_EXP;
    double plus_shift = log2(pageSize);
    return base_shift + (int) plus_shift;
}

void LRU(){
    /*Princípio: descartar a página que ficou sem acesso durante o
        periodo de tempo mais longo
        Pegar o time de cada pagina e ver qual tem o time menor e descarta-la*/
    int smallestSize = INT_MAX;
    
   //PEGAR A MAIOR PAGINA
   Pagina *menor;

    //Se o tempo de acesso for < que o menor tempo, pega a pagina
    for(int i = 0; i < sizePageTable; i++){
        if(pagTable[i].A && pagTable[i].time < smallestSize){
            smallestSize = pagTable[i].time;
            menor = pagTable + i;
        }
    }

    //Seta a flag de modificado para 0 e aumenta a qtd de paginas sujas
    if (menor->M) {
        writtenPages++;
        menor->M = 0;
    }
    
    //Seta as flags de presenca e leitura para 0
    menor->A = 0;
    menor->R = 0;
}

void NRU (){
    /*Verificar bits R E M
    Prioridade de se manter na memoria:
    R | M
    1   1
    1   0
    0   1
    0   0  
    prioridade de ser descartado vai de baixo pra cima*/

    Pagina *atual, *descartado = NULL;
    int i;

    //Compara as flags R e M da pagina atual
    for(i = 0; i < sizePageTable; i++){

        atual = pagTable + i;

        if (atual->A == 0) continue;

        if(atual->R == 0 && atual->M == 0){
            descartado = atual;
            break;
        }

        else if(atual->R == 0 && atual->M == 1){
            if (descartado == NULL) {
                descartado = atual;
                continue;
            }

            if (descartado->R) {
                descartado = atual;
            }
        }
        else if(atual->R == 1 && atual->M == 0){
            if (descartado == NULL) {
                descartado = atual;
                continue;
            }

            if (descartado->R && descartado->M) {
                descartado = atual;
            }
        }
        else {
            if (descartado == NULL) {
                descartado = atual;
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
}

void resetReference() {
    for (int i = 0; i < sizePageTable; i++)
        if (pagTable[i].A)
            pagTable[i].R = 0;
    return;
}
