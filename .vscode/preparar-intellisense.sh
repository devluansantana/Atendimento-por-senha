#!/bin/sh
# Copia os headers do Linux (que estao no container) para .vscode/linux-headers/,
# para o IntelliSense do VS Code funcionar tambem com a pasta aberta direto no Windows
# (sem isso ele mostra erros falsos como "nao e possivel abrir unistd.h").
#
# Roda dentro do container. A tarefa "Compilar tudo" chama este script sozinha
# na primeira vez; para rodar na mao (no Windows):
#     docker compose exec -T dev sh .vscode/preparar-intellisense.sh
set -e
cd "$(dirname "$0")"

destino=linux-headers
rm -rf "$destino"
mkdir -p "$destino"

mkdir -p "$destino/include" "$destino/gcc-include"

# Os headers de firewall do kernel (linux/netfilter*) tem nomes que so mudam de
# maiuscula para minuscula (ipt_ECN.h e ipt_ecn.h), o que o Windows nao aceita.
# O projeto nao usa nenhum deles.
tar -C /usr/include --exclude='./linux/netfilter*' -chf - . | tar -C "$destino/include" -xf -
tar -C "$(gcc -print-file-name=include)" -chf - . | tar -C "$destino/gcc-include" -xf -

# Macros que o gcc define sozinho (__GNUC__, __x86_64__, __linux__, __SIZEOF_LONG__...).
# Os headers do Linux dependem delas; o IntelliSense no Windows nao as conhece.
gcc -dM -E -x c /dev/null | grep -v "__STDC" \
    | awk '{ print "#undef " $2; print }' > "$destino/gcc-macros.h"

echo "IntelliSense pronto: headers do Linux copiados para .vscode/$destino"
