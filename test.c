/*
Alunos participantes do trabalho:
Nomes (N°USP):
	Guilherme José Acra (7150306)
	Leonardo Guarnieri de Bastiani (8910434)
	Luiza Vilas Boas de Oliveira (8503972)
	Ricardo Chagas (8957242)
*/
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
	freopen("saida.txt", "w", stdout);
	remove(FILENAMEARVOREB);
	system("cp arvoreB2.btree arvoreB.btree");
	arvoreb_t *arv = createArvoreB(); printArvoreB(arv);
	srand(4);
	int N = 500;
	int max = 500;
	int i;
	for(i=0; i<N; i++) {
		printf("Chaves inseridas: %d\n", i);
		// int insert = rand() % max;
		// insertArvoreB(arv, insert, 0); printArvoreB(arv);
		int remov = rand() % max;
		removeArvoreB(arv, remov);
	}
	removeArvoreB(arv, 447);
	removeArvoreB(arv, 462);
	printArvoreB(arv);
	freeArvoreB(arv);
	return 0;
}

#endif // TEST