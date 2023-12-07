#define SIM 1

#include "simulador.h"

struct pagina {
    unsigned int addressPhis; // ARM00000 00000XXX XXXXXXXX XXXXXXXX
    int time; // tempo de ultimo acesso
};

struct quadro {
    int num; // se -1 -> livre
};

Pagina* pagTable;
Quadro* frameTable;

int sizeFrameTable, sizePageTable;
int pageSize, memSize, mode;
int pageFaults = 0, writtenPages = 0;

int clock = 0;

int main(int argc, char* argv[]) {
    if check_mode(argv[1]) {
        error("Escolha entre os modos NRU ou LRU\n")
    }

    mode = strcmp(argv[1], "NRU") == 0 ? 0 : 1;
    pageSize = atoi(argv[3]);

    if check_size(pageSize) {
        error("As paginas tem que ter tamanho 8 Kbytes ou 16 Kbytes\n")
    }

    memSize = atoi(argv[4]);
    if check_mem(memSize) {
        error("A memoria fisica tem de ser entre 1Mbytes e 4Mbytes\n")
    }

    FILE* arq = fopen(argv[2], "r");
    if (arq == NULL) {
        error("Arquivo nao encontrado\n")
    }

    printf("Executando o simulador...\n");

    unsigned int address, page, offset = 0, physAddr;
    char accessType;
    int shift = calculaShift();
    createTables();

    while (!feof(arq)) {
        fscanf(arq, "%x %c ", &address, &accessType);

        page = address >> shift;
        for (int i = 0; i < shift; i++)
            offset += address & (0x1 << i);

        physAddr = getPhysAddr(page, offset);

        clock++;
    }

    fclose(arq);

    printf("Arquivo de entrada: %s\n", argv[2]);
    printf("Tamanho da memoria fisica: %d MB\n", memSize);
    printf("Tamanho das paginas: %d KB\n", pageSize);
    printf("Algoritmo de substituicao: %s\n", argv[1]);
    printf("Numero de Faltas de Paginas: %d\n", pageFaults);
    printf("Numero de Paginas Escritas: %d\n", writtenPages);

    return 0;
}

void createTables() {
    int exp = calculaShift(pageSize);

    int qtd = TOTAL_BITS_SIZE - exp;
    sizePageTable = pow(2, qtd);
    pagTable = (Pagina*)malloc(sizeof(Pagina) * sizePageTable);
    
    if (pagTable == NULL) {
        error("Espaco de memoria insuficiente\n")
    }
    
    for (int i = 0; i < sizePageTable; i++) {
        pagTable[i].addressPhis = 0;
        pagTable[i].time = 0;
    }

    qtd = ONE_MB_EXP + memSize - exp;
    sizeFrameTable = pow(2, qtd);
    frameTable = (Quadro*)malloc(sizeof(Quadro) * sizeFrameTable);

    if (frameTable == NULL) {
        error("Espaco de memoria insuficiente\n")
    }

    for (int i = 0; i < sizeFrameTable; i++) {
        frameTable[i].num = -1;
    }
}

unsigned int getPhysAddr(unsigned int index, unsigned int offset) {
    Pagina page = pagTable[index];
    unsigned int addr;
    int freeFrame = -1;
    
    // checa se presente
    if (!(page.addressPhis & PRESENT_BIT)) {
        // se nao, conta mais 1 page fault
        pageFaults++;

        // acha um frame livre
        for (int i = 0; i < sizeFrameTable; i++) {
            if (frameTable[i].num == -1) {
                freeFrame = i;
                frameTable[i].num = i;
            }
        }

        // se nao tiver frame livre -> swap
        if (freeFrame == -1) {
            // libera 1 e devolve numero do frame || swap();
        }

        // guarda frame na pagina
        page.addressPhis = PRESENT_BIT + frameTable[freeFrame].num;
    }

    page.time = clock;
    addr = page.addressPhis << calculaShift() + offset;

    return addr;
}

int calculaShift() {
    int base_shift = ONE_KB_EXP;
    double plus_shift = log2(pageSize);
    return base_shift + (int) plus_shift;
}