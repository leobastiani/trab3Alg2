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
	puts("caso 1, inserindo e removendo apenas um elemento");
	arvoreb_t *arv = createArvoreB();
	printArvoreB(arv);
	insertArvoreB(arv, 1, 2);
	printArvoreB(arv);
	removeArvoreB(arv, 1);
	printArvoreB(arv);
	freeArvoreB(arv);
	deleteFileArvoreB();

	puts("caso 2, inserindo dois elementos e removendo um");
	arv = createArvoreB();
	printArvoreB(arv);
	insertArvoreB(arv, 1, 2);
	insertArvoreB(arv, 2, 3);
	printArvoreB(arv);
	removeArvoreB(arv, 2);
	printArvoreB(arv);
	freeArvoreB(arv);
	deleteFileArvoreB();

	puts("caso 3, inserindo mais de ORDEM elementos");

	free(arv);
	return 0;
}

#endif // TEST