#ifdef TEST

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "arvoreB.h"

int main(int argc, char *argv[]) {
	arvoreb_t *arv = createArvoreB();
	
	
	freeArvoreB(arv);
	free(arv);
	return 0;
}

#endif // TEST