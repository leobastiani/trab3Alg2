#ifndef __ARVOREB_H__
#define __ARVOREB_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#define FILENAMEARVOREB "arvoreB.btree"

#define ORDEM 6
#define MIN_CHAVES ORDEM/2

typedef uint      id_type;
typedef long int  offset_t;
typedef uint      page_t;

typedef struct {
	id_type id;
	offset_t offset;
} arvoreb_elem_t;

typedef struct {
	page_t root;
	uint num_pages;
	page_t empty_pages;
	FILE *fd; // arquivo da árvore que fica constantemente aberto
} arvoreb_t;

typedef struct arvoreb_node_t {
	uint page_num; // é o número da página no disco, começa no 1
	arvoreb_elem_t chaves[ORDEM-1];
	page_t filhos[ORDEM];
	uint8_t num_chaves;
	bool is_folha;
} arvoreb_node_t;

/* ====================================================
   INICIALIZA
   ==================================================== */
void initArvoreB(arvoreb_t *arv);

arvoreb_t *createArvoreB();

void saveToFileArvoreB(arvoreb_t *arv);

void loadArvoreBFromFile(arvoreb_t *arv);

void saveNodeToFile(arvoreb_t *arv, arvoreb_node_t *node);

bool isEmptyArvoreB(arvoreb_t *arv);

bool isPageFull(arvoreb_t *arv, page_t page);

offset_t pageToOffset(page_t page);

arvoreb_node_t *loadNodeFromFile(arvoreb_t *arv, page_t page);

/* ====================================================
   BUSCA
   ==================================================== */
offset_t searchArvoreB(arvoreb_t *arv, id_type id);

offset_t _searchArvoreB(page_t node, id_type id);

/* ====================================================
   INSERÇÃO
   ==================================================== */
bool insertArvoreB(arvoreb_t *arv, id_type id, offset_t offset);

/* ====================================================
   REMOÇÃO
   ==================================================== */
bool removeArvoreB(arvoreb_t *arv, id_type id);

int getIndexNodeArvoreB(arvoreb_node_t *node, id_type id);

bool removeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, id_type id);

void removeFromFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx);

void removeFromNonFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx);

arvoreb_elem_t getPred(arvoreb_t *arv, arvoreb_node_t *node, int idx);

arvoreb_elem_t getSucc(arvoreb_t *arv, arvoreb_node_t *node, int idx);

void fillNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx);

void borrowFromPrev(arvoreb_t *arv, arvoreb_node_t *node, int idx);

void borrowFromNext(arvoreb_t *arv, arvoreb_node_t *node, int idx);

// função que une idx com idx+1
void mergeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx);

/* ====================================================
   NÓS
   ==================================================== */
void initNodeArvoreB(arvoreb_node_t *node);

arvoreb_node_t *createNodeArvoreB();

/* ====================================================
   DESALOCA DA MEMÓRIA
   ==================================================== */
void freeArvoreB(arvoreb_t *arv);

void printArvoreB(arvoreb_t *arv);

void printPagesArvoreB(arvoreb_t *arv, page_t page);

#endif //__ARVOREB_H__