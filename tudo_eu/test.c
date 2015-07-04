#ifdef TEST

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "arvoreB.h"

int main(int argc, char *argv[]) {
	deleteFileArvoreB();
	arvoreb_t *arv = createArvoreB(); printArvoreB(arv);
	
	// inserindo muitos elementos
	insertArvoreB(arv,  9,  1);         printArvoreB(arv);
	insertArvoreB(arv, 11, 20);         printArvoreB(arv);
	insertArvoreB(arv,  3, 34);         printArvoreB(arv);
	insertArvoreB(arv,  7, 42);         printArvoreB(arv);
	insertArvoreB(arv,  5, 34);         printArvoreB(arv);
	insertArvoreB(arv, 14, 31);         printArvoreB(arv);
	insertArvoreB(arv, 15, 29);         printArvoreB(arv);
	insertArvoreB(arv,  6, 16);         printArvoreB(arv);
	insertArvoreB(arv,  2, 24);         printArvoreB(arv);
	insertArvoreB(arv, 10, 41);         printArvoreB(arv);
	insertArvoreB(arv,  4,  7);         printArvoreB(arv);
	insertArvoreB(arv,  8, 36);         printArvoreB(arv);
	insertArvoreB(arv, 12, 23);         printArvoreB(arv);
	insertArvoreB(arv,  1, 23);         printArvoreB(arv);
	insertArvoreB(arv, 13, 29);         printArvoreB(arv);

	// removendo
	removeArvoreB(arv,  6);             printArvoreB(arv);
	removeArvoreB(arv,  7);             printArvoreB(arv);
	removeArvoreB(arv,  2);             printArvoreB(arv);
	removeArvoreB(arv, 10);             printArvoreB(arv);
	removeArvoreB(arv, 12);             printArvoreB(arv);
	removeArvoreB(arv,  1);             printArvoreB(arv);
	removeArvoreB(arv, 15);             printArvoreB(arv);
	removeArvoreB(arv, 14);             printArvoreB(arv);
	removeArvoreB(arv,  4);             printArvoreB(arv);
	removeArvoreB(arv,  9);             printArvoreB(arv);
	removeArvoreB(arv,  8);             printArvoreB(arv);
	removeArvoreB(arv,  3);             printArvoreB(arv);
	removeArvoreB(arv, 13);             printArvoreB(arv);
	removeArvoreB(arv, 11);             printArvoreB(arv);
	removeArvoreB(arv,  5);             printArvoreB(arv);

	freeArvoreB(arv);
	return 0;
}

#endif // TEST