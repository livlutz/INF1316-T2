#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define check_size(x) (x != 8 && x != 16)
#define check_mem(x) (x < 1 || x > 4)
#define check_mode(x) (strcmp(x, "LRU") && strcmp(x, "NRU"))
#define error(msg) printf(msg); exit(1);

int calculaShift(int pageSize);