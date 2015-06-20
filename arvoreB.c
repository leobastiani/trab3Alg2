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
	memset(arv, 0, sizeof(arvoreb_t));
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
	rewind(arv->fd);
	fread(&arv->root, sizeof(page_t), 1, arv->fd);
	fread(&arv->num_pages, sizeof(uint), 1, arv->fd);
	fread(&arv->empty_pages, sizeof(page_t), 1, arv->fd);
}

void saveNodeToFile(arvoreb_t *arv, arvoreb_node_t *node) {
	if(node->page_num == 0) {
		// o valor da página é desconhecido, portanto, devemos criar uma página nova no disco
		if(arv->empty_pages != -1) {
			// obtem o primeiro nó da lista de vazios
			arvoreb_node_t *empty_node = loadNodeFromFile(arv, arv->empty_pages);
			// define a próxima página da lista de vazios
			arv->empty_pages = empty_node->filhos[0];
			saveToFileArvoreB(arv);
			// este nó será salvo no lugar do nó vazio
			node->page_num = empty_node->page_num;
			free(empty_node);
		} else {
			// cria uma nova página no arquivo
			#ifdef DEBUG
				printf("Criando uma página nova no disco\n");
			#endif // DEBUG
			node->page_num = arv->num_pages+1; // 1, 2, 3, ...
		}
		arv->num_pages++; // aumenta o número de páginas
	}
	if(node->num_chaves == 0) {
		// se este nó não possui nenhuma chave, inclui ele na lista de vazios
		// adiciona a próxima página a lista
		node->filhos[0] = arv->empty_pages;
		// define o primeiro da pilha
		arv->empty_pages = node->page_num;
		arv->num_pages--;
		saveToFileArvoreB(arv);
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
		saveToFileArvoreB(arv);
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
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx]);
	if(filho_esquerdo->num_chaves < MIN_CHAVES) {
		// se o filho da chave que encontramos não possui o mínimo de chaves, vamos preencher este filhos
		fillNodeArvoreB(arv, node, idx);
	}
	free(filho_esquerdo);
	// continua procurando para os filhos desse idx
	page_t prox_pagina;
	if(estaNoUltimoFilho && idx > node->num_chaves) {
		// foi retirado um filho
		prox_pagina = node->filhos[idx-1];
	} else {
		// procura no último msmo, normalzin
		prox_pagina = node->filhos[idx];
	}
	free(node);
	node = loadNodeFromFile(arv, prox_pagina);
	bool result = removeNodeArvoreB(arv, node, id);
	free(node);
	return result;
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
	// não passei o id pelo argumento, mas é fácil resgatá-lo
	id_type id = node->chaves[idx].id;

	// para o filho esquerdo
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx]);
	bool runExit = false;
	if(filho_esquerdo->num_chaves >= MIN_CHAVES) {
		runExit = true;
		arvoreb_elem_t pred = getPred(arv, node, idx);
		node->chaves[idx] = pred;
		removeNodeArvoreB(arv, filho_esquerdo, pred.id);
	}
	free(filho_esquerdo);
	if(runExit) {
		return ;
	}

	// para o filho direito
	arvoreb_node_t *filho_direito = loadNodeFromFile(arv, node->filhos[idx+1]);
	if(filho_direito->num_chaves >= MIN_CHAVES) {
		runExit = true;
		arvoreb_elem_t succ = getSucc(arv, node, idx);
		node->chaves[idx] = succ;
		removeNodeArvoreB(arv, filho_direito, succ.id);
	}
	free(filho_direito);
	if(runExit) {
		return ;
	}

	// se os filhos da direita e da esquerda possui menos de MIN_CHAVES
	mergeNodeArvoreB(arv, node, idx);
	filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx]);
	removeNodeArvoreB(arv, filho_esquerdo, id);
	free(filho_esquerdo);
}

arvoreb_elem_t getPred(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	// obtem o maior filho da subárvore da esquerda
	arvoreb_elem_t result;
	page_t next_page = node->filhos[idx];
	while(next_page != -1) {
		arvoreb_node_t *cur = loadNodeFromFile(arv, next_page);
		next_page = cur->filhos[cur->num_chaves];
		result = cur->chaves[cur->num_chaves-1];
		free(cur);
	}
	return result;
}

arvoreb_elem_t getSucc(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	// obtem o menor filho da subárvore da direita
	arvoreb_elem_t result;
	page_t next_page = node->filhos[idx+1];
	while(next_page != -1) {
		arvoreb_node_t *cur = loadNodeFromFile(arv, next_page);
		next_page = cur->filhos[0];
		result = cur->chaves[0];
		free(cur);
	}
	return result;
}

void fillNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx-1]);
	if(idx != 0 && filho_esquerdo->num_chaves >= MIN_CHAVES) {
		borrowFromPrev(arv, node, idx);
	}
	free(filho_esquerdo);

	arvoreb_node_t *filho_direito = loadNodeFromFile(arv, node->filhos[idx+1]);
	if(idx != node->num_chaves && filho_direito->num_chaves >= MIN_CHAVES) {
		borrowFromNext(arv, node, idx);
	}
	free(filho_direito);

	// se foi removido do meio da página, junta os filhos
	if(idx != node->num_chaves) {
		// último filho
		mergeNodeArvoreB(arv, node, idx);
	} else {
		// penúltimo filho
		mergeNodeArvoreB(arv, node, idx-1);
	}
}

void borrowFromPrev(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	arvoreb_node_t *filho_direito = loadNodeFromFile(arv, node->filhos[idx]);
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx-1]);

	int i;
	for(i=filho_direito->num_chaves-1; i>=0; i--) {
		filho_direito->chaves[i+1] = filho_direito->chaves[i];
	}
	if(!filho_direito->is_folha) {
		for(i=filho_direito->num_chaves; i>=0; i++) {
			filho_direito->filhos[i+1] = filho_direito->filhos[i];
		}
	}
	filho_direito->chaves[0] = node->chaves[idx-1];
	if(!node->is_folha) {
		filho_direito->filhos[0] = filho_esquerdo->filhos[filho_esquerdo->num_chaves];
	}
	node->chaves[idx-1] = filho_esquerdo->chaves[filho_esquerdo->num_chaves-1];

	filho_direito->num_chaves++;
	filho_esquerdo->num_chaves--;

	free(filho_esquerdo);
	free(filho_direito);
}

void borrowFromNext(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx]);
	arvoreb_node_t *filho_direito = loadNodeFromFile(arv, node->filhos[idx+1]);

	filho_esquerdo->chaves[filho_esquerdo->num_chaves] = node->chaves[idx];
	if(!filho_esquerdo->is_folha) {
		filho_esquerdo->filhos[filho_esquerdo->num_chaves+1] = filho_direito->filhos[0];
	}
	node->chaves[idx] = filho_direito->chaves[0];
	int i;
	for(i=1; i<filho_direito->num_chaves; i++) {
		filho_direito->chaves[i-1] = filho_direito->chaves[i];
	}
	if(!filho_direito->is_folha) {
		for(i=0; i<=filho_direito->num_chaves; i++) {
			filho_direito->filhos[i-1] = filho_direito->filhos[i];
		}
	}
	filho_esquerdo->num_chaves++;
	filho_direito->num_chaves--;

	free(filho_esquerdo);
	free(filho_direito);
}

// função que une idx com idx+1
void mergeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	arvoreb_node_t *filho_esquerdo = loadNodeFromFile(arv, node->filhos[idx]);
	arvoreb_node_t *filho_direito = loadNodeFromFile(arv, node->filhos[idx+1]);

	filho_esquerdo->chaves[MIN_CHAVES-1] = node->chaves[idx];
	int i;
	for(i=0; i<filho_direito->num_chaves; i++) {
		filho_esquerdo->chaves[i+MIN_CHAVES] = filho_direito->chaves[i];
	}
	if(!filho_esquerdo->is_folha) {
		for(i=0; i<=filho_direito->num_chaves; i++) {
			filho_esquerdo->filhos[i+MIN_CHAVES] = filho_direito->filhos[i];
		}
	}
	for(i=idx+1; i<node->num_chaves; i++) {
		node->chaves[i-1] = node->chaves[i];
	}

	for(i=idx+2; i<=node->num_chaves; i++) {
		node->filhos[i-1] = node->filhos[i];
	}
	filho_esquerdo->num_chaves += filho_direito->num_chaves+1;
	node->num_chaves--;


	saveNodeToFile(arv, node);
	saveNodeToFile(arv, filho_esquerdo);
	saveNodeToFile(arv, filho_direito);
	free(filho_esquerdo);
	free(filho_direito);
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