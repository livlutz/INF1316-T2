#include "simulador.h"

#include <math.h>

#define BITS 32

struct tabela {
    Pagina* tbl;
};

struct pagina {
    unsigned int addressPhis; // ARM00000 00000XXX XXXXXXXX XXXXXXXX
    int time;
};

typedef struct pagina Pagina;
typedef struct tabela Tabela;

Tabela table;

void createTable(int pageSize) {
    int qtd = BITS - calculaShift(pageSize);
    table.tbl = (Pagina*)malloc(sizeof(Pagina) * pow(2, qtd));
    for (int i = 0; i < qtd; i++) {
        table.tbl->addressPhis = 0;
        table.tbl->time = 0;
    }
}

int calculaShift(int pageSize) {
    int base_shift = 10; //1kb
    double plus_shift = log2(pageSize);
    return base_shift + (int) plus_shift;
}