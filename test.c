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
	printArvoreB(arv);
	insertArvoreB(arv, 1, 2);
	printArvoreB(arv);
	removeArvoreB(arv, 1);
	printArvoreB(arv);

	freeArvoreB(arv);
	free(arv);
	return 0;
}

#endif // TEST