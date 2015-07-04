#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
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
	debug("O arquivo da árvore não existe, criando o arquivo da árvore\n");
	arv->fd = fopen(FILENAMEARVOREB, "w+");
	// inicializa a árvore nessa função msmo
	// seta árvore vazia
	arv->root = -1;
	// no começo, não possui página vazia
	arv->empty_pages = -1;
	// agr salva no arquivo
	saveToFileArvoreB(arv);
}

/**
 * Cria a árvore B e inicializa a árvore B
 * @return precisa de free
 */
arvoreb_t *createArvoreB() {
	arvoreb_t *result = malloc(sizeof(arvoreb_t));
	initArvoreB(result);
	return result;
}

/**
 * Salva a árvore B no arquivo
 */
void saveToFileArvoreB(arvoreb_t *arv) {
	rewind(arv->fd);
	fwrite(&arv->root, sizeof(page_t), 1, arv->fd);
	fwrite(&arv->num_pages, sizeof(uint), 1, arv->fd);
	fwrite(&arv->empty_pages, sizeof(page_t), 1, arv->fd);
	fflush(arv->fd);
}

/**
 * Carrega a árvore B do arquivo
 */
void loadArvoreBFromFile(arvoreb_t *arv) {
	debug("Carregando a árvore da memória\n");
	rewind(arv->fd);
	fread(&arv->root, sizeof(page_t), 1, arv->fd);
	fread(&arv->num_pages, sizeof(uint), 1, arv->fd);
	fread(&arv->empty_pages, sizeof(page_t), 1, arv->fd);
}

/**
 * Salva um nó da árvore no arquivo
 * esta função altera o valor de node->page_num para a nova página alocada em disco
 */
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
			debug("Criando uma página nova no disco\n");
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
		if(arv->root == node->page_num) {
			arvoreb_node_t *nova_raiz = loadNodeFromFile(arv, arv->root);
			arv->root = nova_raiz->filhos[0];
			debug("Removeu a raiz! Nova raiz: %d\n", arv->root);
			free(nova_raiz);
		}
		saveToFileArvoreB(arv);
	}
	fseek(arv->fd, pageToOffset(node->page_num), SEEK_SET);
	fwrite(node, sizeof(arvoreb_node_t), 1, arv->fd);
	fflush(arv->fd);
	debug("Salvando página %d\n", node->page_num);
}

/**
 * verifica se a árvore está vazia
 * @return     true or false
 */
bool isEmptyArvoreB(arvoreb_t *arv) {
	return arv->root == -1;
}

/**
 * Verifica se a página está vazia
 * @param  page um inteiro que identifica a página no disco
 * @return      true or false
 */
bool isPageFull(arvoreb_t *arv, page_t page) {
	bool result;
	arvoreb_node_t *node = loadNodeFromFile(arv, page);
	result = node->num_chaves == ORDEM-1;
	free(node);
	return result;
}

/**
 * retorna o offset da página no arquivo de dados
 * @param  page um inteiro que identifica a página no disco
 */
offset_t pageToOffset(page_t page) {
	// anda o número de páginas
	offset_t result = ((page - 1) * sizeof(arvoreb_node_t));
	result += sizeof(page_t) * 2; // anda a raiz e o empty_pages
	result += sizeof(uint); // anda o num_pages
	return result;
}

/**
 * Carrega a página no disco e devolve o ponteiro do nó
 * @param  page um inteiro que identifica a página no disco
 * @return      precisa de free
 */
arvoreb_node_t *loadNodeFromFile(arvoreb_t *arv, page_t page) {
	if(page == -1) {
		return NULL;
	}
	arvoreb_node_t *result = createNodeArvoreB();
	fseek(arv->fd, pageToOffset(page), SEEK_SET);
	fread(result, sizeof(arvoreb_node_t), 1, arv->fd);
	return result;
}

/**
 * Carrega o filho de node do disco para a memória
 * @param  filho idx do filho no nó
 * @return       precisa de fre
 */
arvoreb_node_t *loadFilhoFromFile(arvoreb_t *arv, arvoreb_node_t *node, int filho) {
	return loadNodeFromFile(arv, node->filhos[filho]);
}

/* ====================================================
   BUSCA
   ==================================================== */
/**
* Função que realiza a busca binária em uma página
* @param page é a página em que é feita a bsca, id é o ID buscado, offset_page é um ponteiro para a página
*        em que o ID deveria estar, ideal_pos é um ponteiro para a posição em que o ID deveria estar na página identificada
*        por offset_page.
* @return a função retorna true se o elemento foi encontrado na página e false, caso contrário.
*/
bool b_search(arvoreb_t *arv, arvoreb_node_t *page, id_type id, page_t *offset_page, int *ideal_pos)
{}


/**
* Função que realiza a busca por um ID na árvore B
* @param id é o ID buscado
* @return a função retorna o byte offset do registro do arquivo de dados identificado por id
*/
offset_t searchArvoreB(arvoreb_t *arv, id_type id)
{}

/* ====================================================
   INSERÇÃO
   ==================================================== */
/**
* Função que insere uma nova chave na árvore B
* @param id e offset são os campos da nova chave a ser inserida
* @return true, caso a inserção seja bem sucedida e false, caso contrário
*/
bool insertArvoreB(arvoreb_t *arv, id_type id, offset_t offset) {
	if(isEmptyArvoreB(arv)) {
		arvoreb_node_t *node = createNodeArvoreB();
		node->chaves[0].id = id;
		node->chaves[0].offset = offset;
		node->num_chaves++;
		node->is_folha = true;
		saveNodeToFile(arv, node);
		arv->root = node->page_num;
		saveToFileArvoreB(arv);
		free(node);
		return true;
	}
	arvoreb_node_t *root = loadNodeFromFile(arv, arv->root);
	bool result;
	if(root->num_chaves == ORDEM-1) {
		arvoreb_node_t *grow = createNodeArvoreB();
		grow->filhos[0] = arv->root;
		splitNode(arv, 0, grow, root);
		int i = 0;
		if(grow->chaves[0].id < id) {
			i++;
		}
		arvoreb_node_t *filho = loadFilhoFromFile(arv, grow, i);
		result = insertNonFull(arv, filho, id, offset);
		free(filho);
		saveNodeToFile(arv, grow);
		arv->root = grow->page_num;
		saveToFileArvoreB(arv);
		free(grow);
	} else {
		result = insertNonFull(arv, root, id, offset);
	}
	free(root);
	return result;
}

void splitNode(arvoreb_t *arv, int i, arvoreb_node_t *this, arvoreb_node_t *y) {
	arvoreb_node_t *z = createNodeArvoreB();
	z->is_folha = y->is_folha;
	z->num_chaves = MIN_CHAVES;
	int j;
	for(j=0; j<MIN_CHAVES; j++) {
		z->chaves[j] = y->chaves[j+MIN_CHAVES];
	}
	if(z->is_folha) {
		for(j=0; j<MIN_CHAVES+1; j++) {
			z->filhos[j] = y->filhos[j+MIN_CHAVES+1];
		}
	}
	y->num_chaves = MIN_CHAVES;
	for(j=y->num_chaves; j>=i+1; j--) {
		this->filhos[j+1] = this->filhos[j];
	}
	saveNodeToFile(arv, z);
	this->filhos[i+1] = z->page_num;
	for(j=this->num_chaves-1; j>=i; j++) {
		this->chaves[j+1] = this->chaves[j];
	}
	this->chaves[i] = y->chaves[MIN_CHAVES];
	this->num_chaves++;
	saveNodeToFile(arv, this);
	saveNodeToFile(arv, y);
}

bool insertNonFull(arvoreb_t *arv, arvoreb_node_t *node, id_type id, offset_t offset) {
	int i = node->num_chaves-1;
	if(node->is_folha) {
		while(i >= 0 && node->chaves[i].id > id) {
			node->chaves[i+1] = node->chaves[i];
			i--;
		}
		node->chaves[i+1].id = id;
		node->chaves[i+1].offset = offset;
		node->num_chaves++;
	} else {
		while(i >= 0 && node->chaves[i].id > id) {
			i--;
		}
		arvoreb_node_t *filho = loadNodeFromFile(arv, node->filhos[i+1]);
		if(filho->num_chaves == ORDEM-1) {
			splitNode(arv, i+1, node, filho);
			if(node->chaves[i+1].id < id) {
				i++;
			}
		}
		insertNonFull(arv, filho, id, offset);
		free(filho);
	}

	saveNodeToFile(arv, node);
}

/* ====================================================
   REMOÇÃO
   ==================================================== */
/**
 * Remove o elemento que possui o ID infromado por parâmetro
 * @return     true se foi encontrado
 */
bool removeArvoreB(arvoreb_t *arv, id_type id) {
	file_log("Execucao de operacao de REMOCAO de %d.\n", id);
	if(isEmptyArvoreB(arv)) {
		file_log("Chave %d não cadastrada\n", id);
		return false;
	}
	arvoreb_node_t *root = loadNodeFromFile(arv, arv->root);
	bool result = removeNodeArvoreB(arv, root, id);
	free(root);
	if(result == false) {
		file_log("Chave %d não cadastrada\n", id);
	} else {
		file_log("Chave %d removida com sucesso\n", id);
	}
	return result;
}

/**
 * obtem um índice maior ou igual a devida posição da chave no Nó
 * @param  node nó que a chave será procurado
 * @param  id   id do elemento
 * @return      a posição do índice, sendo ela maior ou igual
 */
int getIndexNodeArvoreB(arvoreb_node_t *node, id_type id) {
	int i = 0;
	while(i < node->num_chaves && node->chaves[i].id < id) {
		i++;
	}
	return i;
}

bool nodeHasKey(arvoreb_node_t *node, id_type key) {
	int i;
	for(i=0; i<node->num_chaves; i++) {
		if(node->chaves[i].id == key) {
			return true;
		}
	}
	return false;
}

/**
 * remove um elemento do nó na árvore b
 * @param  node nó em que será procurado o elemento
 * @param  id   id do elemento
 * @return      true se foi removido com sucesso
 */
bool removeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, id_type id) {
	debug("Pesquisando a chave na página: %d\n", node->page_num);
	int idx = getIndexNodeArvoreB(node, id);
	debug("IDX: %d, Chave: %d, num_chaves: %d\n", idx, node->chaves[idx].id, node->num_chaves);
	if(idx < node->num_chaves && node->chaves[idx].id == id) {
		// o id está nesse nó
		debug("Chave %d encontrada na página: %d\n", id, node->page_num);
		if(node->is_folha) {
			removeFromFolha(arv, node, idx);
		} else {
			removeFromNonFolha(arv, node, idx);
		}
		return true;
	}
	// procura em outros nós
	debug("A página %d é folha? %d\n", node->page_num, node->is_folha);
	if(node->is_folha) {
		// chave não encontrada
		return false;
	}
	page_t prox_pagina = node->filhos[idx];
	// está em algum dos filhos
	bool estaNoUltimoFilho = (idx == node->num_chaves);
	arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx);
	if(filho_esquerdo) {
		if(filho_esquerdo->num_chaves < MIN_CHAVES+1) {
			// se o filho da chave que encontramos não possui o mínimo de chaves, vamos preencher este filhos
			debug("Preenche a página %d para a remoção.\n", filho_esquerdo->page_num);
			fillNodeArvoreB(arv, node, idx);
		}
		free(filho_esquerdo);
	}
	// continua procurando para os filhos desse idx
	debug("estaNoUltimoFilho? %d\n", estaNoUltimoFilho);
	if(estaNoUltimoFilho && idx > node->num_chaves) {
		// foi retirado um filho
		prox_pagina = node->filhos[idx-1];
	}
	debug("Próxima página: %d\n", prox_pagina);
	node = loadNodeFromFile(arv, prox_pagina);
	bool result = removeNodeArvoreB(arv, node, id);
	free(node);
	return result;
}

/**
 * remove um elemento de um nó folha
 * @param idx  índice do elemento no nó da árvore
 */
void removeFromFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	debug("Removendo a chave %d de um nó folha\n", node->chaves[idx].id);
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

/**
 * remove um elemento do nó da árvore que não é folha
 * @param idx  índice do elemento no nó da árvore
 */
void removeFromNonFolha(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	// não passei o id pelo argumento, mas é fácil resgatá-lo
	id_type id = node->chaves[idx].id;
	debug("Removendo a chave %d de uma não folha.\n", id);

	// para o filho esquerdo
	arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx);
	bool runExit = false;
	if(filho_esquerdo->num_chaves >= MIN_CHAVES+1) {
		runExit = true;
		arvoreb_elem_t pred = getPred(arv, node, idx);
		node->chaves[idx] = pred;
		saveNodeToFile(arv, node);
		removeNodeArvoreB(arv, filho_esquerdo, pred.id);
	}
	free(filho_esquerdo);
	if(runExit) {
		return ;
	}

	// para o filho direito
	arvoreb_node_t *filho_direito = loadFilhoFromFile(arv, node, idx+1);
	if(filho_direito->num_chaves >= MIN_CHAVES+1) {
		runExit = true;
		arvoreb_elem_t succ = getSucc(arv, node, idx);
		node->chaves[idx] = succ;
		saveNodeToFile(arv, node);
		removeNodeArvoreB(arv, filho_direito, succ.id);
	}
	free(filho_direito);
	if(runExit) {
		return ;
	}

	// se os filhos da direita e da esquerda possui menos de MIN_CHAVES
	mergeNodeArvoreB(arv, node, idx);
	filho_esquerdo = loadFilhoFromFile(arv, node, idx);
	removeNodeArvoreB(arv, filho_esquerdo, id);
	free(filho_esquerdo);
}

/**
 * obtém o maior elemento da subárvore da esquerda
 * @param  idx  índice do elemento no nó Node
 */
arvoreb_elem_t getPred(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	file_log("Chave %d rebaixada\n", node->chaves[idx].id);
	arvoreb_elem_t result;
	page_t next_page = node->filhos[idx];
	while(next_page != -1) {
		arvoreb_node_t *cur = loadNodeFromFile(arv, next_page);
		next_page = cur->filhos[cur->num_chaves];
		result = cur->chaves[cur->num_chaves-1];
		free(cur);
	}
	file_log("Chave %d promovida\n", result.id);
	return result;
}

/**
 * obtém o menor elemento da subárvore da direita
 * @param  idx  índice do elemento no nó Node
 */
arvoreb_elem_t getSucc(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	file_log("Chave %d rebaixada\n", node->chaves[idx].id);
	arvoreb_elem_t result;
	page_t next_page = node->filhos[idx+1];
	while(next_page != -1) {
		arvoreb_node_t *cur = loadNodeFromFile(arv, next_page);
		next_page = cur->filhos[0];
		result = cur->chaves[0];
		free(cur);
	}
	file_log("Chave %d promovida\n", result.id);
	return result;
}

/**
 * preenche o filho[idx] com algum dos filhos de Node que possui menos do que MIN_CHAVES
 * @param idx  um inteiro que identifica a página no disco
 */
void fillNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	debug("Fill da página %d\n", node->page_num);
	if(idx != 0) {
		arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx-1);
		if(filho_esquerdo->num_chaves >= MIN_CHAVES+1) {
			borrowFromPrev(arv, node, idx);
			free(filho_esquerdo);
			return ;
		}
		free(filho_esquerdo);
	}
	if(idx != node->num_chaves) {
		arvoreb_node_t *filho_direito = loadFilhoFromFile(arv, node, idx+1);
		if(filho_direito->num_chaves >= MIN_CHAVES+1) {
			borrowFromNext(arv, node, idx);
			free(filho_direito);
			return ;
		}
		free(filho_direito);
	}

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
	// debug("borrowFromPrev da página %d com pg: %d e pg: %d\n", node->page_num, node->filhos[idx], node->filhos[idx-1]);
	file_log("Redistribuicao de chaves - entre as páginas irmas %d e %d\n", node->filhos[idx], node->filhos[idx-1]);
	arvoreb_node_t *filho_direito = loadFilhoFromFile(arv, node, idx);
	arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx-1);

	int i;
	for(i=filho_direito->num_chaves-1; i>=0; i--) {
		filho_direito->chaves[i+1] = filho_direito->chaves[i];
	}
	if(!filho_direito->is_folha) {
		for(i=filho_direito->num_chaves; i>=0; i--) {
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

	saveNodeToFile(arv, node);
	saveNodeToFile(arv, filho_esquerdo);
	saveNodeToFile(arv, filho_direito);
	free(filho_esquerdo);
	free(filho_direito);
}

void borrowFromNext(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	// debug("borrowFromNext da página %d com pg: %d e pg: %d\n", node->page_num, node->filhos[idx], node->filhos[idx+1]);
	file_log("Redistribuicao de chaves - entre as páginas irmas %d e %d\n", node->filhos[idx], node->filhos[idx+1]);
	arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx);
	arvoreb_node_t *filho_direito = loadFilhoFromFile(arv, node, idx+1);

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

	saveNodeToFile(arv, node);
	saveNodeToFile(arv, filho_esquerdo);
	saveNodeToFile(arv, filho_direito);
	free(filho_esquerdo);
	free(filho_direito);
}

/**
 * função que une idx com idx+1
 * @param idx  um inteiro que identifica a página no disco
 */
void mergeNodeArvoreB(arvoreb_t *arv, arvoreb_node_t *node, int idx) {
	debug("Merge de %d e %d\n", node->filhos[idx], node->filhos[idx+1]);
	arvoreb_node_t *filho_esquerdo = loadFilhoFromFile(arv, node, idx);
	arvoreb_node_t *filho_direito = loadFilhoFromFile(arv, node, idx+1);

	filho_esquerdo->chaves[MIN_CHAVES] = node->chaves[idx];
	int i;
	for(i=0; i<filho_direito->num_chaves; i++) {
		filho_esquerdo->chaves[i+MIN_CHAVES+1] = filho_direito->chaves[i];
	}
	if(!filho_esquerdo->is_folha) {
		for(i=0; i<=filho_direito->num_chaves; i++) {
			filho_esquerdo->filhos[i+MIN_CHAVES+1] = filho_direito->filhos[i];
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
	filho_direito->num_chaves = 0; // esvazia o filho direito


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

/**
 * cria e inicializa um Nó da árvore
 * @return precisa de free
 */
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
	arv->fd = NULL;
	free(arv);
}

/* ====================================================
   IMPRIME A ÁRVORE
   ==================================================== */
void printArvoreB(arvoreb_t *arv) {
	file_log("Execucao de operacao para mostrar a arvore-B gerada:\n");
	section("IMPRIMINDO A ÁRVORE B");
	printf("       root => %d\n", arv->root);
	printf("  num_pages => %d\n", arv->num_pages);
	printf("empty_pages => ");
	// imprimindo lista de vazios
	page_t next_page = arv->empty_pages;
	while(next_page != -1) {
		printf("%d, ", next_page);
		arvoreb_node_t *node = loadNodeFromFile(arv, next_page);
		next_page = node->filhos[0];
		free(node);
	}
	printf("FIM\n");
	// imrpimindo nós
	printPagesArvoreB(arv, arv->root);
	printf("/* =============================	 */\n\n");
}

/**
 * imprime um nó da árvore recebendo a página no arquivo
 */
void printPagesArvoreB(arvoreb_t *arv, page_t page) {
	if(page == -1) {
		return ;
	}
	printf("\n");
	arvoreb_node_t *node = loadNodeFromFile(arv, page);
	// copia os filhos
	int i;
	printf("PAGINA: %d\n", node->page_num);
	printf("NUM_CHAVES: %d\n", node->num_chaves);
	printf("%s\n", (node->is_folha) ? "É FOLHA!" : "nao é folha");
	printf("\t\tFILHO ESQUERDO: %d\n", node->filhos[0]);
	for(i=0; i<node->num_chaves; i++) {
		printf("\t\tChave: %-2d => offset: %-2ld | FILHO: %d\n", node->chaves[i].id, node->chaves[i].offset, node->filhos[i+1]);
	}
	for(i=0; i<node->num_chaves+1; i++) {
		printPagesArvoreB(arv, node->filhos[i]);
	}
	free(node);
}

void deleteFileArvoreB() {
	remove(FILENAMEARVOREB);
}

/**
 * Use-a como se fosse dar um printf na tela
 * já salva no arquivo automaticamente
 * se DEBUG está ativado, imprime na tela também
 */
void file_log(char *str, ...) {
	debug("ARQUIVO LOG: ");
	FILE *fd = _fopen(FILENAMELOG, "a");
	va_list args;
	va_start(args, str);
	vfprintf(fd, str, args);
	#ifdef DEBUG
		vfprintf(stdout, str, args);
	#endif // DEBUG
	va_end(args);
	fclose(fd);
}
