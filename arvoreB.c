#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "arvoreB.h"

/* ====================================================
   INICIALIZA
   ==================================================== */
void initArvoreB(arvoreb_t *arv) {
	// apenas zera por completo a árvore
	memset(arv, 0, sizeof(arv));
	// no começo, não possui página vazia
	arv->empty_page = -1;
}

arvoreb_t *createArvoreB() {
	arvoreb_t *result = malloc(sizeof(arvoreb_t));
	initArvoreB(result);
	return result;
}

uint8_t isNodeFull(arvoreb_node_t *node) {
	if(node->num_chaves == ORDEM-1) {
		return true;
	}
	return false;
}

/* ====================================================
   BUSCA
   ==================================================== */
arvoreb_elem_t *searchArvoreB(arvoreb_t *arv, id_type id) {
	if(arv->root == NULL) {
		return NULL;
	}
	return _searchArvoreB(arv->root, id);
}

arvoreb_elem_t *_searchArvoreB(arvoreb_node_t *node, id_type id) {

}

/* ====================================================
   INSERÇÃO
   ==================================================== */
bool insertArvoreB(arvoreb_t *arv, id_type id, offset_t offset) {
	// se a árvore está vazia
	if(arv->root == NULL) {
		// cria o primeiro nó
		arvoreb_node_t *node = createNodeArvoreB(id, offset);
		return true;
	}
	// se a raiz está cheia
	if(isNodeFull(arv->root)) {
		return true;
	}
	// se o nó não está cheio, insere nos nós de baixo
	// return insertNodeNonFull(arv->root, id, offset);
	return false; // me remova!
}

/* ====================================================
   REMOÇÃO
   ==================================================== */
bool removeArvoreB(arvoreb_t *arv, id_type id) {

}

/* ====================================================
   NÓS
   ==================================================== */
void initNodeArvoreB(arvoreb_node_t *node, id_type id, offset_t offset) {
	// zera completamente
	memset(node, 0, sizeof(arvoreb_node_t));
	// malloc para chaves e filhos
	node->chaves = malloc((ORDEM-1) * sizeof(arvoreb_elem_t));
	node->filhos = malloc(ORDEM * sizeof(arvoreb_node_t));
	// começa com uma chave
	node->num_chaves = 1;
}

arvoreb_node_t *createNodeArvoreB(id_type id, offset_t offset) {
	arvoreb_node_t *result = malloc(sizeof(arvoreb_node_t));
	initNodeArvoreB(result, id, offset);
	return result;
}

/* ====================================================
   DESALOCA DA MEMÓRIA
   ==================================================== */
void freeArvoreB(arvoreb_t *arv) {
	free(arv->root);
}