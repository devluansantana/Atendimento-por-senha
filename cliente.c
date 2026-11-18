/*
 * Programa do cliente (Dupla D): gera senha e consulta posicao.
 * Compilar: gcc -Wall cliente.c fila.c armazenamento.c -o cliente
 */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fila.h"
#include "armazenamento.h"

/* Le uma linha e converte para inteiro.
   Retorna  1 se a linha tem so um numero inteiro (ex.: "2" ou " 2 "),
            0 se nao tem (linha vazia, "abc", "1abc", numero grande demais),
           -1 se a entrada acabou (Ctrl+D ou terminal fechado). */
static int ler_inteiro(const char *pergunta, int *valor) {
    char linha[64];
    char *fim;
    long numero;

    printf("%s", pergunta);
    fflush(stdout);
    if (fgets(linha, sizeof linha, stdin) == NULL) return -1;

    /* Linha maior que o buffer: descarta o resto, senao ele viraria a proxima resposta. */
    if (strchr(linha, '\n') == NULL && !feof(stdin)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        return 0;
    }

    errno = 0;
    numero = strtol(linha, &fim, 10);
    if (fim == linha || errno == ERANGE || numero < INT_MIN || numero > INT_MAX) return 0;
    while (isspace((unsigned char)*fim)) fim++;   /* aceita espacos e o '\n' no fim */
    if (*fim != '\0') return 0;                    /* sobrou texto, ex.: "1abc" */

    *valor = (int)numero;
    return 1;
}

/* I-01 + I-05: travar -> carregar -> enfileirar -> salvar -> destravar */
static void gerar_senha(void) {
    Fila f;
    int senha;

    travar();
    carregar_fila(&f);
    senha = enfileirar(&f);
    if (senha != -1) {
        salvar_fila(&f);
    }
    destravar();

    if (senha == -1) {
        printf("\n  Fila cheia! Aguarde alguem ser atendido e tente de novo.\n");
    } else {
        printf("\n  Sua senha: %d\n", senha);
    }
}

/* I-02 + I-05: travar -> carregar -> consultar -> destravar */
static void consultar_posicao(void) {
    Fila f;
    int senha, na_frente;

    if (ler_inteiro("Digite sua senha: ", &senha) != 1 || senha < 1) {   /* senhas comecam em 1 */
        printf("\n  Senha invalida.\n");
        return;
    }

    travar();
    carregar_fila(&f);
    destravar();

    na_frente = posicao_da_senha(&f, senha);

    if (na_frente == -1) {
        /* Toda senha menor que proxima_senha ja foi gerada: se nao esta na fila, ja foi chamada. */
        if (senha < f.proxima_senha) {
            printf("\n  A senha %d ja foi chamada.\n", senha);
        } else {
            printf("\n  A senha %d nao existe (ainda nao foi gerada).\n", senha);
        }
    } else if (na_frente == 0) {
        printf("\n  Senha %d: voce e o proximo a ser chamado!\n", senha);
    } else if (na_frente == 1) {
        printf("\n  Senha %d: tem 1 pessoa na sua frente.\n", senha);
    } else {
        printf("\n  Senha %d: tem %d pessoas na sua frente.\n", senha, na_frente);
    }
}

int main(void) {
    int opcao;

    for (;;) {
        int lido;

        printf("\n=== CLIENTE ===\n");
        printf("1 - Gerar senha\n");
        printf("2 - Consultar posicao\n");
        printf("0 - Sair\n");
        lido = ler_inteiro("Opcao: ", &opcao);

        if (lido == -1) break;               /* entrada acabou */
        if (lido == 0) {                     /* Enter vazio, letra etc.: pergunta de novo */
            printf("Opcao invalida.\n");
            continue;
        }
        if (opcao == 0) break;

        switch (opcao) {
        case 1:
            gerar_senha();
            break;
        case 2:
            consultar_posicao();
            break;
        default:
            printf("Opcao invalida.\n");
        }
    }

    return 0;
}
