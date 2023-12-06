#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define check_size(x) (x != 8 && x != 16)
#define check_mem(x) (x < 1 || x > 4)
#define check_mode(x) (strcmp(x, "LRU") && strcmp(x, "NRU"))
#define error(msg) printf(msg); exit(1);

void NRU();
void LRU();

int main (int argc, char* argv[]) {
    if check_mode(argv[1]) {
        error("Escolha entre os modos NRU ou LRU\n")
    }

    int mode = strcmp(argv[1], "NRU") == 0 ? 0 : 1;
    int size = atoi(argv[3]);

    if check_size(size) {
        error("As paginas tem que ter tamanho 8 Kbytes ou 16 Kbytes\n")
    }

    int memory = atoi(argv[4]);
    if check_mem(memory) {
        error("A memoria fisica tem de ser entre 1Mbytes e 4Mbytes\n")
    }

    FILE* arq = fopen(argv[2], "r");
    if (arq == NULL) {
        error("Arquivo nao encontrado\n")
    }

    int address;
    char accessType;
    while(!feof(arq)) {
        fscanf(arq, "%x %c\n", address, accessType);

        
    }

    return 0;
}

void NRU () {

}

void LRU () {

}