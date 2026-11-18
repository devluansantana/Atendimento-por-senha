# Atendimento por Senhas

Dois programas em C que compartilham uma fila circular gravada no arquivo `fila.dat`:

- **cliente**: gera uma senha nova e consulta quantas pessoas estão na frente.
- **atendente**: chama a próxima senha, mostra a fila e roda a demonstração fila linear × circular.

Os programas não conversam diretamente: um grava a fila no arquivo e o outro lê depois.
O acesso é protegido pela trava `fila.lock`.

A documentação de arquitetura, backlog e sprints fica em [docs/](docs/).

## Estrutura

| Arquivo | Responsabilidade | Dupla |
|---|---|---|
| `fila.h` / `fila.c` | Struct da fila, enfileirar, desenfileirar, posição (vetor, índices, módulo) | A |
| `armazenamento.h` / `armazenamento.c` | Ler/gravar `fila.dat`, travar/destravar com `fila.lock` | C |
| `cliente.c` | Menu do cliente (`main` próprio) | D |
| `atendente.c` | Menu do atendente (`main` próprio); chama a demonstração | D |
| `demonstracao.h` / `demonstracao.c` | Roteiro passo a passo linear × circular, em memória | B |
| `fila_linear.h` / `fila_linear.c` | Fila linear, usada só na demonstração | B |
| `demo_main.c` | `main` que roda só a demonstração (`./demo`) | B |
| `testes/teste_fila.c` | Testes da fila (F-05) | A / B |
| `testes/teste_armazenamento.c` | Testes de aceite da A-01 e A-02 | C |
| `testes/teste_dois_processos.c` | A-03: 20 senhas entre dois processos, nenhuma perdida | C |
| `testes/medir_desempenho.c` | E-02: tempo de `carregar_fila()` com `TAM = 1000` | A |
| `docs/` | Documentação: roteiro da demonstração, desempenho, sprints | — |
| `compilar.sh` / `compilar.bat` | Compilam programas e testes (`.bat` = atalho que compila no Docker) | — |
| `Dockerfile` / `docker-compose.yml` | Ambiente de desenvolvimento com gcc, gdb e valgrind | — |

`fila.dat` e `fila.lock` são criados ao rodar e não vão para o repositório.

As assinaturas de `fila.h` e `armazenamento.h` são o contrato entre as duplas: não mude sem combinar com o time.

## Opção 1: Docker (recomendado, ambiente igual para todos)

Precisa do Docker Desktop aberto. A pasta do projeto é montada dentro do container:
você edita no VS Code normalmente e compila/roda no container.

```sh
docker compose up -d --build      # sobe o container (só na primeira vez ou após mudar o Dockerfile)
docker compose exec dev bash      # abre um terminal dentro do container
```

Dentro do container:

```sh
./compilar.sh
./teste_fila
./teste_armazenamento
```

Para testar os dois programas juntos, abra **dois terminais** no Windows e rode
`docker compose exec dev bash` em cada um. Depois:

```sh
./cliente      # terminal 1
./atendente    # terminal 2
```

Os dois rodam no mesmo container e enxergam o mesmo `fila.dat` e `fila.lock`.

## Como usar (passo a passo)

1. No terminal 1 (`./cliente`), escolha **1 - Gerar senha** algumas vezes. Cada uma mostra o número da senha.
2. Ainda no cliente, escolha **2 - Consultar posição**, digite uma senha e veja quantas pessoas estão na frente.
3. No terminal 2 (`./atendente`), escolha **2 - Ver fila**: aparece o vetor, `inicio`, `fim` e `qtd`.
4. No atendente, escolha **1 - Chamar próxima**: sai a senha mais antiga. Volte ao cliente e consulte de novo: a posição diminuiu.
5. No atendente, a opção **3 - Demonstração linear × circular** roda o roteiro em memória (aperte Enter para avançar). Ela não altera a fila real.
6. No atendente, a opção **4 - Acompanhar fila ao vivo** atualiza a tela a cada 1 s; aperte Enter para voltar ao menu.

A fila tem `TAM = 5`: com 5 senhas esperando, o cliente recebe "Fila cheia" até o atendente chamar alguém.
Para começar do zero, apague o `fila.dat`.

## Testes

```sh
./teste_fila                # F-05: vazia, cheia, volta do índice, FIFO
./teste_armazenamento       # A-01 e A-02: arquivo e trava
./teste_dois_processos      # A-03: 20 senhas entre cliente e atendente, nenhuma perdida
./medir_desempenho          # E-02: tempo com TAM = 1000 (resultado em docs/desempenho.md)
./demo                      # só a demonstração linear × circular
```

O roteiro do que falar em cada passo da demonstração está em [docs/roteiro-demonstracao.md](docs/roteiro-demonstracao.md).

Para desligar: `docker compose down`.

O container também tem `gdb` e `valgrind` para depuração.

## Atalho no Windows

O projeto usa recursos do Linux (`fork`, `rename` atômico, `nanosleep`), então **não compila no Windows nativo**.
O `compilar.bat` só é um atalho que compila dentro do Docker:

```bat
compilar.bat
```

Para rodar os programas, use `docker compose exec dev bash` como acima.

## Comandos de compilação (o que os scripts fazem)

```sh
gcc -Wall cliente.c fila.c armazenamento.c -o cliente
gcc -Wall atendente.c demonstracao.c fila_linear.c fila.c armazenamento.c -o atendente
gcc -Wall demo_main.c demonstracao.c fila_linear.c fila.c -o demo
gcc -Wall -I. testes/teste_fila.c fila.c -o teste_fila
gcc -Wall -I. testes/teste_armazenamento.c fila.c armazenamento.c -o teste_armazenamento
gcc -Wall -I. testes/teste_dois_processos.c fila.c armazenamento.c -o teste_dois_processos
gcc -Wall -DTAM=1000 -I. testes/medir_desempenho.c fila.c armazenamento.c -o medir_desempenho
```

Se um programa fechar no meio e deixar o `fila.lock`, apague o arquivo na mão.

## Definition of Done

- [ ] Compila sem erros e sem warnings com `gcc -Wall`.
- [ ] Cumpre o critério de aceite, mostrado rodando.
- [ ] Foi revisada por alguém de outra dupla.
- [ ] Está no repositório principal, sem quebrar o que já funcionava.
