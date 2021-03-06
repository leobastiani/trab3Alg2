/*
Alunos participantes do trabalho:
Nomes (N°USP):
	Guilherme José Acra (7150306)
	Leonardo Guarnieri de Bastiani (8910434)
	Luiza Vilas Boas de Oliveira (8503972)
	Ricardo Chagas (8957242)
*/
#ifndef TEST

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "main.h"

#ifdef DEBUG
	#define clearScreen() ;
#endif // DEBUG

/*
Funcao main apenas pega a opcao do usuario e direciona o programa para a funcao correspondente
Variaveis:
	opc- opcao digitada pelo usuario
	btree- arvore binaria
	reg- possivel arquivo de registros ja existente
*/
int main(void) {
	int opc;
	arvoreb_t *btree;

	btree = createArvoreB();
	if (file_exists(FILE_REG) && isEmptyArvoreB(btree)) { //arquivo de registros ja existe e deve ser carregado
		FILE *reg;
		reg = fopen(FILE_REG, "r");
		Load_reg(reg, btree);
		fclose(reg);
	}

	opc = 1;
	while (opc) {
		clearScreen();
		main_menu();
		opc = _scanf_int();
		printf("==========================================================\n");

		switch (opc) {
			case 1:
				Insere_usuario(btree);
				break;

			case 2:
				Remove_usuario(btree);
				break;

			case 3:
				Busca_usuario(btree);
				break;

			case 4:
				clearScreen();
				printArvoreB(btree);
				system_pause();
				break;

			case 5:
				clearScreen();
				printf("Fechando o programa...\n");
				freeArvoreB(btree);
				opc = 0;
				break;

			default:
				printf("Opcao invalida, tente novamente\n");
				opc = 1; //Garante que o usuario podera escolher novamente
				system_pause();
				break;
		}

	}
	return 0;
}

//Funcao que imprime o menu principal
void main_menu (void) {
	printf("======================Menu Principal=====================\n");
	printf("1-Inserir usuario\n");
	printf("2-Remover usuario\n");
	printf("3-Pesquisar por ID\n");
	printf("4-Mostrar Arvore-B\n");
	printf("5-Fechar o programa\n");
	printf("Digite a opcao desejada: ");
}

/*
Funcao que insere um usuario na btree e no arquivo de registro, se necessario
Variaveis:
	btree- arvore binária
	id- numero da id a ser inserida
	pos- posicao do registro no arquivo de registros
	reg- arquivo de registros
	tu- tipo de usuario a ser inserido
	buff- guarda o tamanho da string a ser armazenada no arquivo de registros
	usr_name- nome do usuario a ser inserido
	buffer- string que guarda todo o registro
*/
void Insere_usuario (arvoreb_t *btree) {
	id_type id;
	offset_t pos;
	FILE *reg;
	int tu;
	char buff;
	char usr_name[MAX_USR_NM];
	char buffer[MAX_REG];

	printf("Digite a id do usuario: ");
	id = _scanf_int();
	if (searchArvoreB(btree, id) >= 0) { //Registro com mesmo nome ja existe
		printf("Usuario ja existente.\n");
		system_pause();
		return;
	}

	printf("Digite o nome do usuario: ");
	_scanf_s(usr_name, MAX_USR_NM);

	printf("Digite o tipo do usuario: ");
	tu = _scanf_int();
	printf("\n");
	switch(tu) {
		case 1:
		case 2:
		case 3:
			debug("Tipo de usuario adicionado: %d\n", tu);
			break;
		default:
			printf("Tipo de usuario invalido\n");
			system_pause();
			return;
	}

	//Insere no arquivo de registro
	reg = fopen(FILE_REG, "ab");
		sprintf(buffer, "%d|%s|%d|", id, usr_name, tu);
		pos = ftell(reg);
		buff = strlen(buffer);
		fwrite(&buff, sizeof(char), 1, reg);
		fwrite(buffer, sizeof(char), buff, reg);
	fclose(reg);

	//Insere na arvore binaria
	file_log("Execucao de operacao de INSERCAO de %d, %s, %d.\n", id, usr_name, tu);
	insertArvoreB(btree, id, pos);
	printf("Insercao feita com sucesso\n");
	system_pause();
}

/*
Funcao que remove um usuario da btree e do arquivo de registro
Variaveis:
	btree- arvore binaria
	id- numero da id a ser removida
	pos- posicao da id no arquivo de registros
	reg- arquivo de registros
	buff- guarda o tamanho da string do registro
	buffer- string que guarda todo o registro
*/
void Remove_usuario (arvoreb_t *btree) {
	id_type id;
	offset_t pos;
	FILE *reg;
	char buff; 
	char buffer[MAX_REG];

	printf("Digite a id do usuario que deseja consultar: ");
	id = _scanf_int();
	printf("\n");
	pos = searchArvoreB(btree, id);
	if (pos < 0) { //O usuario nao foi encontrado na arvore-b
		printf("Usuario nao cadastrado\n");
		system_pause();
		return;
	}

	//Operação para remoção do registro no arquivo
	reg = fopen(FILE_REG, "r+b");
		fseek(reg, pos, SEEK_SET);
		fread(&buff, sizeof(char), 1, reg);
		fread(buffer, sizeof(char), buff, reg);
		altera_srt(buffer);
		fseek(reg, pos+1, SEEK_SET); //Ja posiciona o ponteiro apos o indicador de tamanho
		fwrite(buffer, sizeof(char), buff, reg);
	fclose(reg);

	//Remove da arvore-b
	removeArvoreB(btree, id);
	printf("Remocao realizada com sucesso\n");
	system_pause();
}

/*
Funcao que busca um usuario na btree
Variaveis:
	btree- arvore binaria
	usr- struct que representa e guarda as informações do usuário a ser buscado
	pos- posicao da id no arquivo de registros
	reg- arquivo de registros
	buff- guarda o tamanho da string do registro
	buffer- string que guarda todo o registro
*/
void Busca_usuario(arvoreb_t *btree) {
	offset_t pos;
	usr_t usr;
	FILE *reg;
	char buff;
	char buffer[MAX_REG];

	printf("Digite a id do usuario que deseja consultar: ");
	usr.id = _scanf_int();
	printf("\n");
	pos = searchArvoreB(btree, usr.id);
	if (pos < 0) { //Usuario nao foi encontrado
		printf("Usuario nao cadastrado\n");
		system_pause();
		return;
	} else { //Grava as informações do usuario
		reg = fopen(FILE_REG, "rb");
		if (reg == NULL) {
			printf("Nao foi possivel encontrar o arquivo de registros\n");
			system_pause();
			return;
		}
			fseek(reg, pos, SEEK_SET);
			fread(&buff, sizeof(char), 1, reg);
			fread(buffer, sizeof(char), buff, reg);
			strTOstruct(buffer, &usr);
			file_log("Chave %d encontrada, offset %ld\nNome: %s, Tipo Usuario: %d.\n", usr.id, pos, usr.nome, usr.tu);
			printf("Chave %d encontrada, offset %ld\nNome: %s, Tipo Usuario: %d.\n", usr.id, pos, usr.nome, usr.tu);
			system_pause();
		fclose(reg);
	}
}

/*
Funcao que carrega um arquivo de registros ja existente para uma btree
Variaveis:
	reg- arquivo de registros
	btree- arvore binaria
	usr- struct que representa um determinado usuário
	pos- posicao da id no arquivo de registros
	reg- arquivo de registros
	buff- guarda o tamanho da string a ser armazenada no arquivo de registros
	buffer- string que guarda um registro inteiro
*/
void Load_reg(FILE *reg, arvoreb_t *btree) {
	usr_t usr;
	offset_t pos;
	char buff;
	char buffer[MAX_REG];

	file_log("Execucao da criacao do arquivo de indice %s com base no arquivo de dados %s.\n", FILENAMEARVOREB, FILE_REG);
	rewind(reg);
	while(!feof(reg)) {
		pos = ftell(reg);
		fread(&buff, sizeof(char), 1, reg);
		fread(buffer, sizeof(char), buff, reg);
		strTOstruct(buffer, &usr);

		//Insere apenas na arvore binaria se o registro ainda nao foi removido
		if(usr.nome[0] != '*') {
			insertArvoreB(btree, usr.id, pos);
		}
	}
}

/*
Função que altera uma string de um registro para que ela passe a indicar que o registro foi apagado
Variáveis:
	str- string a ser alterada
	i- contador
Retorno:
	0- a string foi alterada com sucesso
	-3- a string não foi gravada corretamente
*/
int altera_srt(char *str) {
	int i;

	i = 0;
	while (str[i] != '\0' && str[i] != '|') {
		i++;
	}
	if (str[i] == '|') {
		str[i+1] = '*'; //Indicador de que o registro foi removido
		return 0;
	} else { //O registro esta gravado incorretamente
		printf("Registro corrompido\n");
		return -3;
	}
}

/*
Função que passa uma string com um registro para uma struct que representará o registro em memória
Variáveis:
	str- string com o registro
	usr- string para representar o registro
	i, j- contadores
	buffer- usado para gravar temporáriamente cada informação do registro separadamente
*/
void strTOstruct(char *str, usr_t *usr) {
	int i, j;
	char buffer[10];

	//Para id
	i = 0;
	j = 0;
	while (str[i] != '\0' && str[i] != '|') {
		buffer[j] = str[i];
		i++;
		j++;
	}
	buffer[j] = '\0';
	usr->id = atoi(buffer);

	//Para o nome
	i++;
	j = 0;
	while (str[i] != '\0' && str[i] != '|') {
		usr->nome[j] = str[i];
		i++;
		j++;
	}
	usr->nome[j] = '\0';

	//Para o tipo de usuario
	i++;
	usr->tu = atoi(str+i);
}
#endif // TEST