#!/bin/sh
# Compila os programas e os testes (Linux / dentro do Docker).
# Uso: ./compilar.sh        (os executaveis vao para bin/)
set -e
cd "$(dirname "$0")"

# -Wall:     todos os avisos importantes (Definition of Done)
# -g:        informacao de depuracao, para o gdb mostrar as linhas do codigo
# -Iinclude: os headers (.h) ficam na pasta include/
CFLAGS="-Wall -g -Iinclude"

mkdir -p bin

gcc $CFLAGS src/cliente.c src/fila.c src/armazenamento.c -o bin/cliente
gcc $CFLAGS src/atendente.c src/demonstracao.c src/fila_linear.c src/fila.c src/armazenamento.c -o bin/atendente
gcc $CFLAGS src/demo_main.c src/demonstracao.c src/fila_linear.c src/fila.c -o bin/demo
gcc $CFLAGS testes/teste_fila.c src/fila.c -o bin/teste_fila
gcc $CFLAGS testes/teste_armazenamento.c src/fila.c src/armazenamento.c -o bin/teste_armazenamento
gcc $CFLAGS testes/teste_dois_processos.c src/fila.c src/armazenamento.c -o bin/teste_dois_processos
gcc $CFLAGS -DTAM=1000 testes/medir_desempenho.c src/fila.c src/armazenamento.c -o bin/medir_desempenho

echo "Compilado em bin/: cliente atendente demo teste_fila teste_armazenamento teste_dois_processos medir_desempenho"
