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

typedef struct {
	id_type id;
	offset_t offset;
} arvoreb_elem_t;

typedef struct {
	struct arvoreb_node_t *root;
	uint num_pages;
	uint empty_page;
} arvoreb_t;

typedef struct arvoreb_node_t {
	uint page_num;
	arvoreb_elem_t *chaves;
	struct arvoreb_node_t *filhos;
	uint8_t num_chaves;
	bool is_folha;
} arvoreb_node_t;

/* ====================================================
   INICIALIZA
   ==================================================== */
void initArvoreB(arvoreb_t *arv);

arvoreb_t *createArvoreB();

/* ====================================================
   BUSCA
   ==================================================== */
arvoreb_elem_t *searchArvoreB(arvoreb_t *arv, id_type id);

arvoreb_elem_t *_searchArvoreB(arvoreb_node_t *node, id_type id);

/* ====================================================
   INSERÇÃO
   ==================================================== */
bool insertArvoreB(arvoreb_t *arv, id_type id, offset_t offset_t);

/* ====================================================
   REMOÇÃO
   ==================================================== */
bool removeArvoreB(arvoreb_t *arv, id_type id);

/* ====================================================
   DESALOCA DA MEMÓRIA
   ==================================================== */
void freeArvoreB(arvoreb_t *arv);

#endif //__ARVOREB_H__