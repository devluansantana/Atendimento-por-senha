/*
 * Programa do cliente (Dupla D): gera senha e consulta posicao.
 * Compilar: gcc -Wall cliente.c fila.c armazenamento.c -o cliente
 */
#include <stdio.h>
#include <stdlib.h>
#include "fila.h"
#include "armazenamento.h"

/* Le uma linha e converte para inteiro. Retorna 0 se a entrada acabou ou nao e numero. */
static int ler_inteiro(const char *pergunta, int *valor) {
    char linha[64];
    char *fim;

    printf("%s", pergunta);
    fflush(stdout);
    if (fgets(linha, sizeof linha, stdin) == NULL) return 0;
    *valor = (int)strtol(linha, &fim, 10);
    return fim != linha;
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

    if (!ler_inteiro("Digite sua senha: ", &senha)) {
        printf("\n  Senha invalida.\n");
        return;
    }

    travar();
    carregar_fila(&f);
    destravar();

    na_frente = posicao_da_senha(&f, senha);

    if (na_frente == -1) {
        printf("\n  A senha %d nao esta na fila (ja foi chamada ou nao existe).\n", senha);
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
        printf("\n=== CLIENTE ===\n");
        printf("1 - Gerar senha\n");
        printf("2 - Consultar posicao\n");
        printf("0 - Sair\n");
        if (!ler_inteiro("Opcao: ", &opcao)) break;

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
