# Ambiente de desenvolvimento C (Linux) para o Atendimento por Senhas.
# O codigo NAO e copiado para a imagem: a pasta do projeto e montada em /app
# pelo docker-compose, entao o que voce edita no VS Code aparece no container na hora.
#
#   gcc, libc6-dev, make  -> compilar
#   gdb, valgrind         -> depurar e achar erros de memoria
#   git, ca-certificates  -> usar o Git de dentro do Dev Container
#   procps                -> ps / pkill, para encerrar um programa travado
FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        gcc libc6-dev make gdb valgrind git ca-certificates procps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

CMD ["bash"]
