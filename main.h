#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "arvoreB.h"
#include "misc.h"

#define MAX_USR_NM 50
#define MAX_REG 70
#define FILE_REG "registro.reg"

typedef struct REGISTRO {
	int id;
	char nome[MAX_USR_NM];
	int tu;
} usr_t;

void main_menu(void);
void Insere_usuario (arvoreb_t *btree);
void Remove_usuario (arvoreb_t *btree);
void Busca_usuario(arvoreb_t *btree);
void Load_reg(FILE *reg, arvoreb_t *btree);
int altera_srt(char *str);
void strTOstruct(char *str, usr_t *usr);