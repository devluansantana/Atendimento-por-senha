#!/bin/sh
# Compila os programas e os testes (Linux / dentro do Docker).
# Uso: ./compilar.sh
set -e
cd "$(dirname "$0")"

gcc -Wall cliente.c fila.c armazenamento.c -o cliente
gcc -Wall atendente.c demonstracao.c fila_linear.c fila.c armazenamento.c -o atendente
gcc -Wall demo_main.c demonstracao.c fila_linear.c fila.c -o demo
gcc -Wall -I. testes/teste_fila.c fila.c -o teste_fila
gcc -Wall -I. testes/teste_armazenamento.c fila.c armazenamento.c -o teste_armazenamento
gcc -Wall -I. testes/teste_dois_processos.c fila.c armazenamento.c -o teste_dois_processos
gcc -Wall -DTAM=1000 -I. testes/medir_desempenho.c fila.c armazenamento.c -o medir_desempenho

echo "Compilado: ./cliente ./atendente ./demo ./teste_fila ./teste_armazenamento ./teste_dois_processos ./medir_desempenho"
