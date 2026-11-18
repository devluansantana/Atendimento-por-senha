#!/bin/sh
# Compila os programas e os testes (Linux / dentro do Docker).
# Uso: ./compilar.sh
set -e
cd "$(dirname "$0")"

# -Wall: todos os avisos importantes (Definition of Done)
# -g:    informacao de depuracao, para o gdb mostrar as linhas do codigo
CFLAGS="-Wall -g"

gcc $CFLAGS cliente.c fila.c armazenamento.c -o cliente
gcc $CFLAGS atendente.c demonstracao.c fila_linear.c fila.c armazenamento.c -o atendente
gcc $CFLAGS demo_main.c demonstracao.c fila_linear.c fila.c -o demo
gcc $CFLAGS -I. testes/teste_fila.c fila.c -o teste_fila
gcc $CFLAGS -I. testes/teste_armazenamento.c fila.c armazenamento.c -o teste_armazenamento
gcc $CFLAGS -I. testes/teste_dois_processos.c fila.c armazenamento.c -o teste_dois_processos
gcc $CFLAGS -DTAM=1000 -I. testes/medir_desempenho.c fila.c armazenamento.c -o medir_desempenho

echo "Compilado: ./cliente ./atendente ./demo ./teste_fila ./teste_armazenamento ./teste_dois_processos ./medir_desempenho"
