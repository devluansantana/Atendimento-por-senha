/*
 * A-03: teste com dois processos (Dupla C).
 * Simula os dois terminais: um processo faz o papel do cliente (gera senhas) e
 * o outro faz o papel do atendente (chama senhas), os dois no mesmo fila.dat,
 * usando o padrao travar -> carregar -> mexer -> salvar -> destravar.
 *
 * Criterio de aceite: 20 senhas geradas e chamadas alternando os processos,
 * nenhuma perdida, e chamadas na ordem em que foram geradas (FIFO).
 *
 * Linux apenas (usa fork). Compilar na raiz do projeto:
 *     gcc -Wall -I. testes/teste_dois_processos.c fila.c armazenamento.c -o teste_dois_processos
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "fila.h"
#include "armazenamento.h"

#define TOTAL 20

static int falhas = 0;

#define CHECA(cond, msg) \
    do { if (cond) printf("[OK]    %s\n", msg); \
         else { printf("[FALHA] %s\n", msg); falhas++; } } while (0)

static void dormir_ms(long ms) {
    struct timespec t = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&t, NULL);
}

/* "Cliente": gera TOTAL senhas. Se a fila estiver cheia, espera e tenta de novo. */
static void processo_cliente(void) {
    Fila f;
    int geradas = 0;

    while (geradas < TOTAL) {
        int senha;
        travar();
        carregar_fila(&f);
        senha = enfileirar(&f);
        if (senha != -1) salvar_fila(&f);
        destravar();

        if (senha == -1) dormir_ms(2);   /* cheia: espera o atendente liberar */
        else geradas++;
    }
    _exit(0);
}

/* "Atendente": chama senhas ate ter chamado TOTAL; manda cada uma pelo pipe. */
static void processo_atendente(int escrita) {
    Fila f;
    int chamadas = 0;

    while (chamadas < TOTAL) {
        int senha;
        travar();
        carregar_fila(&f);
        senha = desenfileirar(&f);
        if (senha != -1) salvar_fila(&f);
        destravar();

        if (senha == -1) {
            dormir_ms(2);                /* vazia: espera o cliente gerar */
        } else {
            if (write(escrita, &senha, sizeof senha) != sizeof senha) _exit(1);
            chamadas++;
        }
    }
    _exit(0);
}

static int filho_ok(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int main(void) {
    char pasta[] = "/tmp/teste_dois_processos_XXXXXX";
    int canal[2];
    int chamadas[TOTAL];
    int lidas = 0;
    pid_t cliente, atendente;
    Fila f;

    if (mkdtemp(pasta) == NULL || chdir(pasta) != 0) {
        perror("pasta temporaria");
        return 1;
    }
    if (pipe(canal) != 0) {
        perror("pipe");
        return 1;
    }

    printf("A-03: 1 cliente e 1 atendente, %d senhas, TAM = %d\n\n", TOTAL, TAM);

    cliente = fork();
    if (cliente == 0) { close(canal[0]); close(canal[1]); processo_cliente(); }

    atendente = fork();
    if (atendente == 0) { close(canal[0]); processo_atendente(canal[1]); }

    close(canal[1]);
    while (lidas < TOTAL &&
           read(canal[0], &chamadas[lidas], sizeof chamadas[0]) == sizeof chamadas[0]) {
        lidas++;
    }
    close(canal[0]);

    CHECA(filho_ok(cliente),   "processo cliente terminou sem erro");
    CHECA(filho_ok(atendente), "processo atendente terminou sem erro");
    CHECA(lidas == TOTAL,      "o atendente chamou as 20 senhas");

    int em_ordem = 1;
    for (int i = 0; i < lidas; i++) {
        if (chamadas[i] != i + 1) em_ordem = 0;
    }
    CHECA(em_ordem, "senhas chamadas na ordem 1, 2, ..., 20 (nenhuma perdida, FIFO)");

    carregar_fila(&f);
    CHECA(f.qtd == 0 && f.proxima_senha == TOTAL + 1,
          "no fim a fila esta vazia e a proxima senha seria a 21");
    CHECA(access(ARQUIVO_TRAVA, F_OK) != 0, "fila.lock foi apagado no fim");

    printf("\n%d falha(s)\n", falhas);
    return falhas != 0;
}
