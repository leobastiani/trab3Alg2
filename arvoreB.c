#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "arvoreB.h"
#include "misc.h"

/* ====================================================
   INICIALIZA
   ==================================================== */
void initArvoreB(arvoreb_t *arv) {
	// apenas zera por completo a árvore
	memset(arv, 0, sizeof(arv));
	if(file_exists(FILENAMEARVOREB)) {
		// se a árvore já existe, carrega ela do arquivo
		arv->fd = fopen(FILENAMEARVOREB, "r+");
		loadArvoreBFromFile(arv);
		return ;
	}
	#ifdef DEBUG
		printf("O arquivo da árvore não existe, criando o arquivo da árvore\n");
	#endif // DEBUG
	arv->fd = fopen(FILENAMEARVOREB, "w+");
	// inicializa a árvore nessa função msmo
	// seta árvore vazia
	arv->root = -1;
	// no começo, não possui página vazia
	arv->empty_pages = -1;
	// agr salva no arquivo
	saveToFileArvoreB(arv);
}

arvoreb_t *createArvoreB() {
	arvoreb_t *result = malloc(sizeof(arvoreb_t));
	initArvoreB(result);
	return result;
}

void saveToFileArvoreB(arvoreb_t *arv) {
	rewind(arv->fd);
	fwrite(&arv->root, sizeof(page_t), 1, arv->fd);
	fwrite(&arv->num_pages, sizeof(uint), 1, arv->fd);
	fwrite(&arv->empty_pages, sizeof(page_t), 1, arv->fd);
}

void loadArvoreBFromFile(arvoreb_t *arv) {
	#ifdef DEBUG
		printf("Carregando a árvore da memória\n");
	#endif // DEBUG
	fread(&arv->root, sizeof(page_t), 1, arv->fd);
	fread(&arv->num_pages, sizeof(uint), 1, arv->fd);
	fread(&arv->empty_pages, sizeof(page_t), 1, arv->fd);
}

void saveNodeToFile(arvoreb_t *arv, arvoreb_node_t *node) {
	// TESTAR ANTES SE FOI REMOVIDO ALGUM NÓ
	if(node->page_num == 0) {
		// o valor da página é desconhecido, portanto, devemos criar uma página nova no disco
		if(arv->empty_pages != -1) {
			return ;
		} else {
			// cria uma nova página no arquivo
			#ifdef DEBUG
				printf("Criando uma página nova no disco\n");
			#endif // DEBUG
			arv->num_pages++; // aumenta o número de páginas
			node->page_num = arv->num_pages; // 1, 2, 3, ...
		}
	}
	fseek(arv->fd, pageToOffset(node->page_num), SEEK_SET);
	fwrite(node, sizeof(arvoreb_node_t), 1, arv->fd);
}

bool isEmptyArvoreB(arvoreb_t *arv) {
	return arv->root == -1;
}

bool isPageFull(arvoreb_t *arv, page_t page) {
	bool result;
	arvoreb_node_t *node = loadNodeFromFile(arv, page);
	result = node->num_chaves == ORDEM-1;
	free(node);
	return result;
}

offset_t pageToOffset(page_t page) {
	// anda o número de páginas
	offset_t result = ((page - 1) * sizeof(arvoreb_node_t));
	result += sizeof(page_t) * 2; // anda a raiz e o empty_pages
	result += sizeof(uint); // anda o num_pages
	return result;
}

arvoreb_node_t *loadNodeFromFile(arvoreb_t *arv, page_t page) {
	arvoreb_node_t *result = createNodeArvoreB();
	fseek(arv->fd, pageToOffset(page), SEEK_SET);
	fread(result, sizeof(arvoreb_node_t), 1, arv->fd);
	return result;
}

/* ====================================================
   BUSCA
   ==================================================== */
offset_t searchArvoreB(arvoreb_t *arv, id_type id) {
	if(isEmptyArvoreB(arv)) {
		return -1;
	}
	return _searchArvoreB(arv->root, id);
}

offset_t _searchArvoreB(page_t node, id_type id) {
	return -1;
}

/* ====================================================
   INSERÇÃO
   ==================================================== */
bool insertArvoreB(arvoreb_t *arv, id_type id, offset_t offset) {
	// se a árvore está vazia
	if(isEmptyArvoreB(arv)) {
		// cria o primeiro nó
		#ifdef DEBUG
			printf("Inserindo na árvore: ID: %d => offset: %ld\n", id, offset);
		#endif // DEBUG
		arvoreb_node_t *node = createNodeArvoreB();
		node->chaves[0].id = id;
		node->chaves[0].offset = offset;
		node->num_chaves = 1;
		node->is_folha = true;
		saveNodeToFile(arv, node);
		// atualiza a raiz
		arv->root = node->page_num;
		saveToFileArvoreB(arv);
		free(node);
		return true;
	}
	// se a raiz está cheia
	if(isPageFull(arv, arv->root)) {
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
	if(isEmptyArvoreB(arv)) {
		return false;
	}
	#ifdef DEBUG
		printf("Removendo a chave com ID: %d\n", id);
	#endif // DEBUG
	arvoreb_node_t *root = loadNodeFromFile(arv, arv->root);
	bool result = removeNodeArvoreB(arv, root, id);
	if(root->num_chaves == 0) {
		// a raiz está vazia
		if(root->is_folha) {
			arv->root = -1;
		} else {
			// se ele n for folha, o primeiro filho passa a ser a raiz
			arv->root = root->filhos[0];
		}
	}
	free(root);
	return result;
}

int getIndexNodeArvoreB(arvoreb_node_t *node, id_type id) {
	int i = 0;
	while(i < ORDEM-1 && node->chaves[i].id < id) {
		i++;
	}
	return i;
}

bool removeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, id_type id) {
	int idx = getIndexNodeArvoreB(node, id);
	if(idx < node->num_chaves && node->chaves[idx].id == id) {
		// o id está nesse nó
		if(node->is_folha) {
			removeFromFolha(arv, node, idx);
		} else {
			removeFromNonFolha(arv, node, idx);
		}
		return true;
	}
	// procura em outros nós
	if(node->is_folha) {
		// chave não encontrada
		return false;
	}
	// está em algum dos filhos
	bool estaNoUltimoFilho = (idx == node->num_chaves);
	return true;
}

void removeFromFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	// move todos os ídices a direita dele para a esquerda
	int i;
	for(i=idx+1; i<node->num_chaves; i++) {
		node->chaves[i-1] = node->chaves[i];
	}
	// reduz uma chave
	node->num_chaves--;
	// salva no arquivo
	saveNodeToFile(arv, node);
}

void removeFromNonFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx) {

}

/* ====================================================
   NÓS
   ==================================================== */
void initNodeArvoreB(arvoreb_node_t *node) {
	// zera completamente
	memset(node, 0, sizeof(arvoreb_node_t));
	// seta os filhos para -1
	int i;
	for(i=0; i<ORDEM; i++) {
		node->filhos[i] = -1;
	}
}

arvoreb_node_t *createNodeArvoreB() {
	arvoreb_node_t *result = malloc(sizeof(arvoreb_node_t));
	initNodeArvoreB(result);
	return result;
}

/* ====================================================
   DESALOCA DA MEMÓRIA
   ==================================================== */
void freeArvoreB(arvoreb_t *arv) {
	fclose(arv->fd);
}

void printArvoreB(arvoreb_t *arv) {
	section("IMPRIMINDO A ÁRVORE B");
	printf("root => %d\n", arv->root);
	printf("num_pages => %d\n", arv->num_pages);
	printf("empty_pages => %d\n", arv->empty_pages);
	// imrpimindo nós
	printPagesArvoreB(arv, arv->root);
	printf("\n\n");
}

void printPagesArvoreB(arvoreb_t *arv, page_t page) {
	if(page == -1) {
		return ;
	}
	arvoreb_node_t *node = loadNodeFromFile(arv, page);
	page_t filhos[ORDEM];
	// copia os filhos
	int i;
	for(i=0; i<ORDEM; i++) {
		filhos[i] = node->filhos[i];
	}
	printf("PAGINA: %d\n", node->page_num);
	printf("NUM_CHAVES: %d\n", node->num_chaves);
	printf("%s\n", (node->is_folha) ? "É FOLHA!" : "nao é folha");
	for(i=0; i<node->num_chaves; i++) {
		printf("\t\tChave: %-2d => offset: %-2ld\n", node->chaves[i].id, node->chaves[i].offset);
	}
	free(node);
	for(i=0; i<ORDEM; i++) {
		printPagesArvoreB(arv, filhos[i]);
	}
}