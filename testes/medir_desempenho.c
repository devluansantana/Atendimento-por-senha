/*
 * E-02: medicao de desempenho de carregar_fila() com TAM = 1000 (Dupla A).
 *
 * Responde a pergunta da apresentacao: "ler o arquivo inteiro toda vez nao fica lento?"
 * Compilar com TAM = 1000 (na raiz do projeto):
 *     gcc -Wall -DTAM=1000 -Iinclude testes/medir_desempenho.c src/fila.c src/armazenamento.c -o bin/medir_desempenho
 *     ./bin/medir_desempenho
 *
 * Roda numa pasta em /tmp, entao nao mexe no fila.dat do projeto.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "fila.h"
#include "armazenamento.h"

#define REPETICOES 100000

int main(void) {
    char pasta[] = "/tmp/medir_desempenho_XXXXXX";
    Fila f, lida;
    clock_t ini, fim;
    double total_s;

    if (mkdtemp(pasta) == NULL || chdir(pasta) != 0) {
        perror("pasta temporaria");
        return 1;
    }

    /* fila cheia: o pior caso, com todas as posicoes ocupadas */
    inicializar(&f);
    while (enfileirar(&f) != -1) { }
    salvar_fila(&f);

    printf("TAM = %d  |  sizeof(Fila) = %zu bytes  |  %d leituras\n\n",
           TAM, sizeof(Fila), REPETICOES);

    ini = clock();
    for (int i = 0; i < REPETICOES; i++) {
        carregar_fila(&lida);
    }
    fim = clock();
    total_s = (double)(fim - ini) / CLOCKS_PER_SEC;
    printf("carregar_fila():  %.6f s no total  ->  %.2f microssegundos por chamada\n",
           total_s, total_s / REPETICOES * 1e6);

    ini = clock();
    for (int i = 0; i < REPETICOES; i++) {
        salvar_fila(&f);
    }
    fim = clock();
    total_s = (double)(fim - ini) / CLOCKS_PER_SEC;
    printf("salvar_fila():    %.6f s no total  ->  %.2f microssegundos por chamada\n",
           total_s, total_s / REPETICOES * 1e6);

    /* limpeza da pasta temporaria */
    remove(ARQUIVO_FILA);
    if (chdir("/") == 0) rmdir(pasta);
    return 0;
}
