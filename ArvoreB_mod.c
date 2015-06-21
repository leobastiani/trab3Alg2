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
/*BUSCA BINÁRIA*/
/*Parâmetros:
        page: ponteiro para a página na qual se está realizando a busca
        id: ID que está sendo buscado
        page_pos: número da página na qual está ou deveria estar o ID
        ideal_pos: posição da página em que está ou deveria estar o ID*/
bool b_search(arvoreb_node_t *page, id_type id, page_t *page_pos, unit *ideal_pos)
{
    int left, right, middle;
    left = -1;
    right = page->num_chaves;
    *offset = -1;

    while(left < right-1)
    {
        middle = (left + right)/2;

        if (page->chaves[middle].id < id)
        {
            //Filho direito
            *page_pos = page->filhos[middle+1];

            *ideal_pos = middle+1;
            left = middle;
        }

        else
        {

            if (page->chaves[middle].id > id)
            {
                //Filho esquerdo
                *page_pos = page->filhos[middle];

                *ideal_pos = middle;
                right = middle;
            }

            else
            {
                if (page->chaves[middle].id == id)
                {
                    *page_pos = page->page_num;
                    *ideal_pos = middle;
                    return true;
                }

            }
        }

    }

    return false;
}

/*GENERAL_SEARCH*/
/*Parâmetros:
        page: ponteiro para a página na qual se está realizando a busca
        id: ID que está sendo buscado
        page_pos: número da página na qual está ou deveria estar o ID
        ideal_pos: posição da página em que está ou deveria estar o ID*/
bool general_search(arvoreb_t *btree, id_type id, arvoreb_node_t *page, uint *ideal_pos)
{
    offset_t offset_root;                           //Byte offset da raiz
    uint pos;                                       //Variáel de posição a ser usada na busca binária
    bool busca = false;                             //Variável de controle da busca
    int i;

    //Posisionando ao início do arqvuivo de árvore-B
    fseek(btree->fd, 0, SEEK_SET);

    //Lendo byte offset da raiz
    fread(&offset_root, sizeof(long int), 1, btree);

    //Reposicionamenro no arquivo de acordo com o byte offset da raiz
    fseek(btree->fd, offset_root, SEEK_SET);

    //Lendo página raiz
    fread(page, sizeof(arvoreb_node_t), 1, btree->fd);

    //Enquanto a busca não for bem sucedida
    while(!busca && !page->folha)
    {
        busca = b_search(page, id, &pos, ideal_pos);

        if (!busca)
        {
            fseek(btree->fd, pos*sizeof(arvoreb_node_t), SEEK_SET);
            fread(page, sizeof(arvoreb_node_t), 1, btree->fd);
        }
    }

    //Se o elemento foi encontrado, offset é igual ao rrn do registro correspondente
    if (busca)
        return true;

    else
        return false;

}

/* ====================================================
   INSERÇÃO
   ==================================================== */
bool insertion(arvoreb_elem_t *chave, arvoreb_t *btree, FILE *dados)
{

    //Ponteiro para a página em que deveria estar a chave
    arvoreb_node_t *temp = (arvoreb_node_t*)malloc(sizeof(arvoreb_node_t));

    //Posição, dentro da página, em que deveria estar a chave
    uint *ideal_pos;

    //Posição da raiz no arquivo
    offset_t offset_root;

    int i, j, k;

    //Lendo a raiz
    fseek(btree->fd, 0, SEEK_SET);
    fread(&offset_root, sizeof(offset_t), 1, btree->fd);
    fseek(btree->fd, offset_root, SEEK_SET);
    fread(temp, sizeof(arvoreb_node_t), 1, btree->fd);

    /*CASO 1:Elemento duplicado*/
    if (general_search(btree->fd, chave->id, temp, ideal_pos))
    {
        printf("\nChave exixtente.\n");
        return false;
    }

    else
    {

    /*CASO 2: Árvore vazia*/

    if(isEmptyArvoreB(arv))
    {
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
        else
        {

    /*CASO 3: Pode ou não haver espaço na página*/
            if(temp->num_chaves >= ORDEM-1)
            {
                return recursive_insertion(chave, temp->page_num, -1, arv);
            }
        }
    }
}

bool recursive_insertion(arvoreb_elem_t *chave, page_t page, page_t filho, arvoreb_t *arv)
{
       //Nova página do split
        arvoreb_node_t *new_page;

        //Estruturas auxiliars do Split
        arvoreb_elem_t *split_aux;
        page_t *split;

        //Chave a ser promovida
        arvoreb_elem_t *prom = (arvoreb_elem_t*)malloc(sizeof(arvoreb_elem_t));

        //Parâmetros a serem passados à chamada recursiva
        page_t pai;
        page_t filho;


       arvoreb_node_t *temp = createNodeArvoreB();
       uint ideal_pos;
       int i;


       //Encontrando e lendo a página
       fseek(arv->fd, pageToOffset(page), SEEK_SET);
       fread(temp, sizeof(arvoreb_node_t), 1, arv->fd);

       //Caso haja espaço na página
       if(temp->num_chaves < ORDEM -1)
       {
           b_search(temp, chave.id, page, &ideal_pos);

           //Deslocando chaves para a direita
           i = temp->num_chaves-1;
           while(i>=ideal_pos)
           {
               temp->chaves[i+1].id = temp->chaves[i].id;
               temp->chaves[i+1].offset = temp->chaves[i].offset;
               temp->filhos[i+2] = temp->filhos[i+1];
           }

           //Inserindo chave
           temp->chaves[ideal_pos].id = chave.id;
           temp->chaves[ideal_pos].offset = chave.offset;
           temp->filhos[ideal_pos+1] = filho;

           //Salvando página atualizada
           saveNodeToFile(arv, temp);

           free(temp);
           free(prom);

           return true;
       }

       //Caso não haja espaço na página
       else
       {
            split_aux = (arvoreb_elem_t*)malloc(ORDEM*sizeof(arvoreb_elem_t));
            split = (page_t*)malloc((ORDEM+1)*sizeof(page_t));

            //Nova página para o split
            new_page = createNodeArvoreB();


            //Inserindo as chaves da página "temp" e de "chave" no vetor auxiliar, e os filhos de "temp" e o filho direito
            //de "chave" no vetor auxiliar antes do split

            i=0;
            while(i != ideal_pos && i<ORDEM-1)
            {
                split_aux[i].id = temp->chaves[i].id;
                split_aux[i].offset = temp->chaves[i].offset;
                split[i] = temp->filhos[i];
                i++;
            }

            //Inserindo a chave na posição ideal
            split_aux[i].id = temp->chaves[i].id;
            split_aux[i].offset = temp->chaves[i].offset;
            split[i] = temp->filhos[i];

            while(i<ORDEM-1)
            {
                split_aux[i+1].id = temp->chaves[i].id;
                split_aux[i+1].offset = temp->chaves[i].offset;
                split[i+1] = temp->filhos[i];
                i++;
            }

            split[i] = temp->filhos[i];

            //Posição do elemento a ser promovido
            j = ORDEM/2;
            k = 0;

            //Elemento a ser promovido
            prom->id = split_aux[j]->id;
            prom->offset = split_aux[j]->offset;

            //Split
            for(i = j+1; i<ORDEM-1; j++)
            {
                new_page->chaves[k].id = temp->chaves[i].id;
                new_page->chaves[k].offset = temp->chaves[i].offset;
                new_page->filhos[k] = temp->filhos[i];
                temp->chaves[i].id = 0;
                temp->chaves[i].offset = 0;
                temp->filhos[i] = -1;
                k++;
            }

            new_page->filhos[k] = temp->filhos[ORDEM-1];
            temp->filhos[ORDEM-1] = -1;

            //Atualizando dados da página
            if(temp->is_folha)
                new_page->is_folha = true;
            else
                new_page->is_folha = false;

            new_page->num_chaves = k;
            new_page->pai = temp->pai;
            btree->num_pages ++;
            new_page->page_num = btree->num_pages;

            //Salvando página no arquivo
            saveNodeToFile(btree, new_page);

            pai = temp->pai;
            filho = new_page->page_num;

            free(temp);
            free(new_page);

            //Caso a página que sofreu o split seja a raiz
            if(temp->page_num == arv->root)
            {
                //Nova raiz
                new_page = createNodeArvoreB();
                arv->num_pages ++;

                new_page->chaves[0].id = prom.id;
                new_page->chaves[0].offset = prom.offset;
                new_page->filhos[0] = temp->page_num;
                new_page->filhos[1] = filho;
                new_page->is_folha = true;
                new_page->pai = -1;
                new_page->page_num = arv->num_pages;

                arv->root = new_page->page_num;

                saveNodeToFile(arv, new_page);

                free(temp);
                free(new_page);

                return true;
            }

            //Promovendo chave
            recursive_insertion(prom, pai, filho, arv);

       }
}

/* ====================================================
   REMOÇÃO
   ==================================================== */
bool removeArvoreB(arvoreb_t *arv, id_type id) {

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
