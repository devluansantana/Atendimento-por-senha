#!/bin/sh
# Compila tudo e roda todos os testes automaticos (Linux / dentro do Docker).
# Uso: ./testar.sh
# Termina com codigo 0 so se todos os testes passarem.
set -e
cd "$(dirname "$0")"

./compilar.sh

falhou=""
for teste in teste_fila teste_armazenamento teste_dois_processos; do
    echo
    echo "===== $teste ====="
    if ! ./bin/"$teste"; then
        falhou="$falhou $teste"
    fi
done

echo
if [ -z "$falhou" ]; then
    echo "TODOS OS TESTES PASSARAM"
else
    echo "FALHARAM:$falhou"
    exit 1
fi
