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
	uint page_num;          //Número da página no disco, começa no 1
	unit pai;               //Número da página-pai no disco
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

bool b_search(arvoreb_node_t *page, id_type id, page_t *page_pos, unit *ideal_pos);
bool general_search(arvoreb_t *btree, id_type id, arvoreb_node_t *page, uint *ideal_pos);

/* ====================================================
   INSERÇÃO
   ==================================================== */

bool insertion(arvoreb_elem_t *chave, arvoreb_t *btree, FILE *dados);
bool recursive_insertion(arvoreb_elem_t *chave, page_t page, page_t filho, arvoreb_t *arv);

/* ====================================================
   REMOÇÃO
   ==================================================== */
bool removeArvoreB(arvoreb_t *arv, id_type id);

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
