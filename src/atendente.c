/*
 * Programa do atendente (Duplas D e B): chama a proxima senha, mostra a fila
 * e roda a demonstracao fila linear x circular.
 * Compilar (na raiz do projeto): ./compilar.sh   -> gera bin/atendente
 * (junta src/atendente.c, demonstracao.c, fila_linear.c, fila.c e armazenamento.c)
 */
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "fila.h"
#include "armazenamento.h"
#include "demonstracao.h"

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

/* I-04: mostra o vetor (so as posicoes ocupadas), inicio, fim e qtd. */
static void mostrar_fila(const Fila *f) {
    printf("\n  Vetor: [");
    for (int i = 0; i < TAM; i++) {
        int ocupada = 0;
        for (int k = 0; k < f->qtd; k++) {
            if ((f->inicio + k) % TAM == i) { ocupada = 1; break; }
        }
        if (ocupada) printf(" %d", f->senhas[i]);
        else         printf(" _");
    }
    printf(" ]\n");
    printf("  inicio=%d  fim=%d  qtd=%d\n", f->inicio, f->fim, f->qtd);

    if (f->qtd == 0) {
        printf("  Ninguem esperando.\n");
    } else {
        printf("  Ordem de chamada:");
        for (int k = 0; k < f->qtd; k++) {
            printf(" %d", f->senhas[(f->inicio + k) % TAM]);
        }
        printf("\n");
    }
}

/* I-03 + I-05: travar -> carregar -> desenfileirar -> salvar -> destravar */
static void chamar_proxima(void) {
    Fila f;
    int senha;

    travar();
    carregar_fila(&f);
    senha = desenfileirar(&f);
    if (senha != -1) {
        salvar_fila(&f);
    }
    destravar();

    if (senha == -1) {
        printf("\n  A fila esta vazia. Nenhuma senha para chamar.\n");
    } else {
        printf("\n  Chamando a senha %d\n", senha);
    }
}

/* I-04 + I-05: travar -> carregar -> destravar (so le, nao precisa salvar) */
static void ver_fila(void) {
    Fila f;

    travar();
    carregar_fila(&f);
    destravar();

    mostrar_fila(&f);
}

/* I-06 (opcional): redesenha a fila a cada 1 s ate o usuario apertar Enter. */
static void acompanhar_fila(void) {
    for (;;) {
        Fila f;
        fd_set leitura;
        struct timeval espera = { 1, 0 };

        travar();
        carregar_fila(&f);
        destravar();

        printf("\033[2J\033[H");  /* limpa a tela e volta o cursor ao topo */
        printf("=== FILA AO VIVO (atualiza a cada 1 s) ===");
        mostrar_fila(&f);
        printf("\n  [Enter para voltar ao menu]\n");
        fflush(stdout);

        FD_ZERO(&leitura);
        FD_SET(STDIN_FILENO, &leitura);
        if (select(STDIN_FILENO + 1, &leitura, NULL, NULL, &espera) > 0) {
            char descarte[64];
            if (fgets(descarte, sizeof descarte, stdin) == NULL) return;
            return;
        }
    }
}

int main(void) {
    int opcao;

    for (;;) {
        int lido;

        printf("\n=== ATENDENTE ===\n");
        printf("1 - Chamar proxima\n");
        printf("2 - Ver fila\n");
        printf("3 - Demonstracao linear x circular\n");
        printf("4 - Acompanhar fila ao vivo\n");
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
            chamar_proxima();
            break;
        case 2:
            ver_fila();
            break;
        case 3:
            demonstracao();
            break;
        case 4:
            acompanhar_fila();
            break;
        default:
            printf("Opcao invalida.\n");
        }
    }

    return 0;
}
