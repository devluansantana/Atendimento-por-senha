# Atendimento por Senhas

Dois programas em C que compartilham uma fila circular gravada no arquivo `fila.dat`:

- **cliente**: gera uma senha nova e consulta quantas pessoas estão na frente.
- **atendente**: chama a próxima senha, mostra a fila e roda a demonstração fila linear × circular.

Os programas não conversam diretamente: um grava a fila no arquivo e o outro lê depois.
O acesso é protegido pela trava `fila.lock`.

O projeto roda no **Linux, dentro do Docker**. A camada de armazenamento usa recursos do Linux (`rename` atômico, `nanosleep`, e os testes usam `fork`), então não compila no Windows nativo.

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
| `compilar.sh` / `testar.sh` | Compila tudo / compila e roda todos os testes | — |
| `compilar.bat` | Atalho do Windows que compila dentro do Docker | — |
| `Dockerfile` / `docker-compose.yml` | Ambiente Linux com gcc, gdb, valgrind e git | — |
| `.devcontainer/` | Abre o projeto dentro do container no VS Code | — |
| `.vscode/` | Tarefas (compilar, testar, abrir os programas), depuração (F5) e IntelliSense | — |
| `docs/` | Roteiro da demonstração, desempenho, sprint 1 da Dupla C | — |

`fila.dat` e `fila.lock` são criados ao rodar e não vão para o repositório.

As assinaturas de `fila.h` e `armazenamento.h` são o contrato entre as duplas: não mude sem combinar com o time.

## Começando (VS Code + Dev Container, recomendado)

Precisa do **Docker Desktop** aberto e das extensões **Dev Containers** e **C/C++** no VS Code (o VS Code sugere as duas ao abrir a pasta).

1. Abra a pasta do projeto no VS Code.
2. `Ctrl+Shift+P` → **Dev Containers: Reopen in Container**. Na primeira vez ele monta a imagem (alguns minutos) e já compila tudo.
3. Pronto: o editor, o terminal (`Ctrl+'`), as tarefas e o depurador rodam no Linux do container.

### Sem o Dev Container (pasta aberta direto no Windows)

Também funciona. Com o Docker Desktop aberto, rode uma vez a tarefa **Compilar tudo** (`Ctrl+Shift+B`): ela sobe o container, compila e copia os headers do Linux para `.vscode/linux-headers/`. A partir daí o IntelliSense entende `unistd.h`, `sys/wait.h` etc. e para de marcar erros falsos. Os headers copiados não vão para o Git.

### Tarefas

`Ctrl+Shift+P` → **Tasks: Run Task** e escolha:

| Tarefa | O que faz |
|---|---|
| **Compilar tudo** (`Ctrl+Shift+B`) | `./compilar.sh`; erros de compilação aparecem no painel Problems |
| **Rodar todos os testes** | `./testar.sh`: compila e roda os três testes automáticos |
| **Abrir cliente e atendente lado a lado** | Compila e abre os dois programas em terminais divididos, com a mesma fila |
| **Abrir cliente** / **Abrir atendente** | Um programa por vez |
| **Abrir demonstração linear x circular** | O roteiro da apresentação (`./demo`) |
| **Mostrar condição de corrida (com e sem trava)** | Com trava nada se perde; sem trava várias gravações somem |
| **Medir desempenho (E-02)** | Tempo de `carregar_fila()`/`salvar_fila()` com `TAM = 1000` |
| **Zerar a fila** | Apaga `fila.dat` e `fila.lock`; a próxima senha volta a ser 1 |
| **Preparar IntelliSense no Windows** | Copia de novo os headers do Linux (a **Compilar tudo** já faz isso na primeira vez) |
| **Docker: iniciar / reconstruir / desligar container** | Para quem abriu a pasta no Windows |

As tarefas também funcionam com a pasta aberta no Windows: nesse caso cada comando roda no container com `docker compose exec`.

### Depurar (F5)

Dentro do Dev Container: clique à esquerda do número da linha para pôr um breakpoint, escolha **Depurar cliente**, **Depurar atendente** ou um dos testes na aba *Run and Debug* (`Ctrl+Shift+D`) e aperte **F5**. O programa roda no terminal integrado e para no breakpoint.

## Sem o VS Code (terminal do Windows)

```sh
docker compose up -d --build      # sobe o container (só na primeira vez ou após mudar o Dockerfile)
docker compose exec dev bash      # abre um terminal dentro do container
```

Dentro do container:

```sh
./testar.sh                       # compila e roda todos os testes
```

Para testar os dois programas juntos, abra **dois terminais** no Windows e rode
`docker compose exec dev bash` em cada um. Depois:

```sh
./cliente      # terminal 1
./atendente    # terminal 2
```

Os dois rodam no mesmo container e enxergam o mesmo `fila.dat` e `fila.lock`.

Para desligar: `docker compose down`.

O `compilar.bat` é um atalho que compila dentro do Docker sem abrir o container: `compilar.bat`.

## Como usar (passo a passo)

1. No terminal 1 (`./cliente`), escolha **1 - Gerar senha** algumas vezes. Cada uma mostra o número da senha.
2. Ainda no cliente, escolha **2 - Consultar posição**, digite uma senha e veja quantas pessoas estão na frente.
3. No terminal 2 (`./atendente`), escolha **2 - Ver fila**: aparece o vetor, `inicio`, `fim` e `qtd`.
4. No atendente, escolha **1 - Chamar próxima**: sai a senha mais antiga. Volte ao cliente e consulte de novo: a posição diminuiu.
5. No atendente, a opção **3 - Demonstração linear × circular** roda o roteiro em memória (aperte Enter para avançar). Ela não altera a fila real.
6. No atendente, a opção **4 - Acompanhar fila ao vivo** atualiza a tela a cada 1 s; aperte Enter para voltar ao menu.

A fila tem `TAM = 5`: com 5 senhas esperando, o cliente recebe "Fila cheia" até o atendente chamar alguém.
Para começar do zero, use a tarefa **Zerar a fila** (ou apague o `fila.dat`).

## Testes

```sh
./testar.sh                 # compila e roda os três testes abaixo
./teste_fila                # F-05: vazia, cheia, volta do índice, FIFO
./teste_armazenamento       # A-01 e A-02: arquivo e trava
./teste_dois_processos      # A-03: 20 senhas entre cliente e atendente, nenhuma perdida
./teste_armazenamento --sem-trava   # mostra a condição de corrida (para a apresentação)
./medir_desempenho          # E-02: tempo com TAM = 1000 (resultado em docs/desempenho.md)
./demo                      # só a demonstração linear × circular
```

Os testes rodam em pastas temporárias em `/tmp` e não mexem no `fila.dat` do projeto.

O roteiro do que falar em cada passo da demonstração está em [docs/roteiro-demonstracao.md](docs/roteiro-demonstracao.md).

## Comandos de compilação (o que o `compilar.sh` faz)

```sh
gcc -Wall -g cliente.c fila.c armazenamento.c -o cliente
gcc -Wall -g atendente.c demonstracao.c fila_linear.c fila.c armazenamento.c -o atendente
gcc -Wall -g demo_main.c demonstracao.c fila_linear.c fila.c -o demo
gcc -Wall -g -I. testes/teste_fila.c fila.c -o teste_fila
gcc -Wall -g -I. testes/teste_armazenamento.c fila.c armazenamento.c -o teste_armazenamento
gcc -Wall -g -I. testes/teste_dois_processos.c fila.c armazenamento.c -o teste_dois_processos
gcc -Wall -g -DTAM=1000 -I. testes/medir_desempenho.c fila.c armazenamento.c -o medir_desempenho
```

O `-g` só acrescenta informação para o depurador; não muda o comportamento.

## Problemas comuns

| Sintoma | Causa e solução |
|---|---|
| Erros vermelhos de `#include` (`unistd.h`, `sys/wait.h`...) no VS Code | A pasta está aberta no Windows e os headers do Linux ainda não foram copiados. Rode a tarefa **Compilar tudo** (ou use **Dev Containers: Reopen in Container**). |
| `failed to connect to the docker API` | O Docker Desktop está fechado. Abra e espere ele ficar pronto. |
| `Aguardando fila.lock...` | Um programa fechou no meio e deixou a trava. Feche cliente e atendente e rode **Zerar a fila** (ou `rm fila.lock`). |
| `fila.dat esta corrompido ou foi gravado com outro TAM` | Apague o `fila.dat` (tarefa **Zerar a fila**). |
| `Fila cheia!` no cliente | Normal com `TAM = 5`: chame senhas no atendente. |

## Definition of Done

- [ ] Compila sem erros e sem warnings com `gcc -Wall`.
- [ ] Cumpre o critério de aceite, mostrado rodando.
- [ ] Foi revisada por alguém de outra dupla.
- [ ] Está no repositório principal, sem quebrar o que já funcionava.
