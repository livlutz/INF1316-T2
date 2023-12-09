#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define check_size(x) (x != 8 && x != 16)
#define check_mem(x) (x < 1 || x > 4)
#define check_mode(x) (strcmp(x, "LRU") && strcmp(x, "NRU"))
#define error(msg) printf(msg); exit(1);

// tamanho do endereco
#define TOTAL_BITS_SIZE 32
// expoente de 1 MB (2^20)
#define ONE_MB_EXP 20
// expoente de 1 KB (2^10)
#define ONE_KB_EXP 10

//Estruturas das paginas 
typedef struct pagina Pagina;

/*
 Cria as tabelas de paginas e de quadros
*/
void createTables();
/*
 Calcula o deslocamento com base no tamanho das paginas
*/
int calculaShift();
/*
 Definir o q essa funcao vai fazer
*/
void getPhysAddr(unsigned int index, unsigned int offset, char accessType);

/*
Faz o algoritmo de substituicao de paginas LRU
*/
int LRU ();

/*
Faz o algoritmo de substituicao de paginas NRU
*/
int NRU ();

/*
Reinicia os bits de referencia (interrupcao do relogio)
*/
void resetReference();