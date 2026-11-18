# Atendimento por Senhas

Sistema de atendimento por senhas em C. São **dois programas** rodando em dois terminais ao mesmo tempo, que compartilham **uma única fila** guardada no arquivo `fila.dat`:

- **`cliente`** (o totem): gera uma senha nova e consulta quantas pessoas estão na frente.
- **`atendente`** (o guichê): chama a próxima senha, mostra a fila e roda a demonstração fila linear × circular.

O que o trabalho pede (FIFO, vetor com índices, operador módulo e a limitação da fila linear) fica isolado em `src/fila.c` e na demonstração do atendente.

- [Como funciona](#como-funciona)
- [Arquitetura](#arquitetura)
- [Como colocar para funcionar](#como-colocar-para-funcionar)
- [Como usar](#como-usar-passo-a-passo) · [Tarefas do VS Code](#tarefas-do-vs-code) · [Testes](#testes) · [Problemas comuns](#problemas-comuns)

## Como funciona

### Visão geral

```
    cliente (terminal 1)              atendente (terminal 2)
gera senha, consulta posição      chama a próxima, mostra a fila
              │                                  │
              └──────────────┐    ┌──────────────┘
                             ▼    ▼
                 ┌────────────────────────────┐
                 │ fila.lock   a trava        │
                 │ fila.dat    a fila inteira │
                 └────────────────────────────┘
```

Os dois programas **nunca conversam diretamente**. Um grava a fila no arquivo e o outro lê o arquivo depois: essa é toda a comunicação. Cada programa só carrega a fila para a memória quando o usuário escolhe uma opção do menu, e o `fila.lock` garante que só um mexe por vez.

### A fila circular (`include/fila.h` / `src/fila.c`)

A fila é um vetor de tamanho fixo e quatro números:

```c
#define TAM 5

typedef struct {
    int senhas[TAM];
    int inicio;        // índice da próxima senha a ser chamada
    int fim;           // índice onde entra a próxima senha gerada
    int qtd;           // quantas senhas estão esperando
    int proxima_senha; // contador para gerar números novos (1, 2, 3...)
} Fila;
```

- **Entrar na fila** (`enfileirar`): a senha nova vai para `senhas[fim]` e o `fim` anda uma posição. Se `qtd == TAM`, a fila está cheia e a função devolve `-1`.
- **Sair da fila** (`desenfileirar`): sai `senhas[inicio]`, a mais antiga (**FIFO**: primeiro a entrar, primeiro a sair), e o `inicio` anda uma posição. Se `qtd == 0`, devolve `-1`.
- **Consultar** (`posicao_da_senha`): percorre as `qtd` senhas a partir do `inicio` e devolve quantas estão na frente, ou `-1` se a senha não está na fila.

Os índices andam com o **operador módulo**: depois da última posição eles voltam para 0, reaproveitando o espaço liberado no começo do vetor.

```c
f->fim    = (f->fim + 1) % TAM;      // com TAM = 5: 3 → 4 → 0 → 1
f->inicio = (f->inicio + 1) % TAM;
```

**Por que existe o `qtd`:** na fila circular, `inicio == fim` acontece tanto com a fila vazia quanto com a cheia. O contador desfaz a dúvida: `qtd == 0` é vazia, `qtd == TAM` é cheia.

### Uma ação, do menu ao arquivo

Toda opção do menu, nos dois programas, segue o mesmo padrão. Exemplo do cliente gerando uma senha (`src/cliente.c`):

```c
travar();                 // cria fila.lock; se já existe, espera a vez
carregar_fila(&f);        // fila.dat → memória (um fread da struct inteira)
senha = enfileirar(&f);   // mexe só na cópia em memória
if (senha != -1)
    salvar_fila(&f);      // memória → fila.dat (fwrite)
destravar();              // apaga fila.lock
```

As consultas (**Consultar posição** e **Ver fila**) só leem: travam, carregam e destravam, sem salvar.

Exemplo com os dois programas abertos. Estado inicial do `fila.dat`: `senhas = [7 8 9 _ _]`, `inicio = 0`, `fim = 3`, `qtd = 3`, `proxima_senha = 10`.

| Passo | Quem | Ação | `fila.dat` depois |
|---|---|---|---|
| 1 | Cliente | Gerar senha: lê, enfileira a 10, grava | `[7 8 9 10 _]`, `fim = 4`, `qtd = 4` |
| 2 | Atendente | Chamar próxima: lê, tira a 7, grava | `inicio = 1`, `qtd = 3` |
| 3 | Cliente | Consultar a senha 10: lê e calcula | sem mudança (2 pessoas na frente: 8 e 9) |

No passo 2 o atendente já enxerga a senha 10 porque leu o arquivo que o cliente acabou de gravar.

Os programas **não ficam relendo o arquivo**: cada um lê só quando o usuário escolhe uma opção. A exceção é a opção **4 - Acompanhar fila ao vivo** do atendente, que relê a cada 1 s.

### O arquivo `fila.dat` (`src/armazenamento.c`)

- Guarda a **struct inteira** como um bloco de bytes (com `TAM = 5`, são 36 bytes). Um `fread` copia o bloco para uma variável `Fila` e um `fwrite` grava de volta; não é preciso montar elemento por elemento.
- **Sem arquivo**, `carregar_fila` começa uma fila nova (`inicializar`).
- **Arquivo estranho** (tamanho errado, gravado com outro `TAM` ou com índices fora do vetor): o programa avisa e para, em vez de ler lixo. Basta apagar o `fila.dat`.
- **Gravação segura:** `salvar_fila` grava primeiro em `fila.dat.tmp.<pid>` e depois troca pelo `fila.dat` com `rename()`, que no Linux acontece de uma vez. Se o programa cair no meio da gravação, quem se estraga é o temporário; o `fila.dat` continua inteiro.

### A trava `fila.lock`

Sem trava, uma senha pode sumir quando os dois programas mexem ao mesmo tempo. Isso se chama **condição de corrida**:

1. O cliente lê `qtd = 3`.
2. O atendente lê `qtd = 3`.
3. O cliente enfileira e grava `qtd = 4`.
4. O atendente desenfileira e grava `qtd = 2`, por cima do que o cliente gravou.
5. A senha gerada no passo 3 sumiu.

Como a trava resolve:

- Antes de ler, o programa tenta criar o `fila.lock` com `fopen("fila.lock", "wx")`. O `"x"` só cria se o arquivo **não existir**, e o Linux faz "verificar e criar" numa operação só (`O_CREAT | O_EXCL`). Por isso dois programas nunca conseguem ao mesmo tempo.
- **Conseguiu criar:** a vez é dele. Lê, mexe, grava e apaga o `fila.lock`.
- **Já existia:** o outro programa está mexendo. Espera 100 ms e tenta de novo.
- O `fila.lock` guarda o número do processo (PID) de quem está com a trava: `cat fila.lock` mostra quem é.
- Se o programa sair por erro segurando a trava, ela é apagada sozinha (`atexit`). Se ele for morto no meio (Ctrl+C, terminal fechado), a trava pode ficar: depois de 3 s esperando, o outro programa avisa para apagar o `fila.lock` (tarefa **Zerar a fila**).

A tarefa **Mostrar condição de corrida (com e sem trava)** mostra isso na prática: 4 processos gravando a mesma fila perdem mais da metade das 100 gravações sem a trava, e nenhuma com ela.

### Fila linear × circular (demonstração)

O sistema real usa a fila **circular**. A fila **linear** (`src/fila_linear.c`) aparece só na opção **3 - Demonstração** do atendente (ou `./bin/demo`), que roda em memória e não mexe no `fila.dat`. Com `TAM = 5`:

| Passo | Fila linear | Fila circular |
|---|---|---|
| Enfileira 1, 2, 3, 4, 5 | `[1 2 3 4 5]`, `fim = 5` | `[1 2 3 4 5]`, `fim = (4 + 1) % 5 = 0` |
| Chama 3 senhas | `[_ _ _ 4 5]`, `inicio = 3` | `[_ _ _ 4 5]`, `inicio = 3` |
| Tenta enfileirar a 6 | **"Fila cheia!"**: `fim = 5` já passou do vetor, mesmo com 3 posições livres | `[6 _ _ 4 5]`: a 6 entra na posição 0, reaproveitada |
| Chama a próxima | Sai a 4 (FIFO) | Sai a 4 (FIFO) |

Na linear, os índices só andam para frente, então as posições liberadas no começo nunca mais são usadas. O texto do que falar em cada passo está em [docs/roteiro-demonstracao.md](docs/roteiro-demonstracao.md).

## Arquitetura

### Camadas

Cada arquivo tem uma responsabilidade só. A lógica da fila não sabe que existe arquivo nem tela. Os headers (`.h`, a interface de cada camada) ficam em `include/` e as implementações (`.c`) em `src/`.

| Camada | Header (`include/`) | Implementação (`src/`) | Faz | Não faz | Usa | Dupla |
|---|---|---|---|---|---|---|
| Lógica da fila | `fila.h` | `fila.c` | Struct `Fila`, enfileirar, desenfileirar, posição (vetor, índices, módulo) | Ler arquivo, imprimir na tela | nada | A |
| Armazenamento | `armazenamento.h` | `armazenamento.c` | Ler e gravar `fila.dat`; travar e destravar com `fila.lock` | Mexer na lógica da fila | `fila.h` (só `inicializar`) | C |
| Interfaces | — | `cliente.c`, `atendente.c` | Menus; cada um tem seu `main` | Lógica de fila ou de arquivo | `fila.h`, `armazenamento.h`, `demonstracao.h` | D |
| Demonstração | `demonstracao.h`, `fila_linear.h` | `demonstracao.c`, `fila_linear.c`, `demo_main.c` | Roteiro linear × circular, em memória | Mexer no `fila.dat` | `fila.h` | B |

### Contrato entre as camadas

As funções de `include/fila.h` e `include/armazenamento.h` são o contrato entre as duplas. Enquanto ninguém mudar as assinaturas, cada dupla trabalha sem esperar a outra. **Não mude sem combinar com o time.**

```c
/* include/fila.h */
void inicializar(Fila *f);                 // fila vazia, próxima senha = 1
int  enfileirar(Fila *f);                  // senha gerada, ou -1 se cheia
int  desenfileirar(Fila *f);               // senha chamada, ou -1 se vazia
int  posicao_da_senha(Fila *f, int senha); // pessoas na frente, ou -1 se não está na fila

/* include/armazenamento.h */
void carregar_fila(Fila *f);               // fila.dat → memória (sem arquivo: fila nova)
void salvar_fila(const Fila *f);           // memória → fila.dat
void travar(void);                         // cria fila.lock, esperando a vez
void destravar(void);                      // apaga fila.lock
```

### Estrutura do repositório

```
├── include/                    headers (.h): a interface de cada camada
│   ├── fila.h                  struct Fila e funções da fila
│   ├── armazenamento.h         carregar/salvar fila.dat, travar/destravar
│   ├── demonstracao.h          demonstração linear × circular
│   └── fila_linear.h           fila linear (só na demonstração)
├── src/                        implementações (.c)
│   ├── fila.c                  lógica da fila circular
│   ├── armazenamento.c         fila.dat e fila.lock
│   ├── cliente.c               programa do cliente (main)
│   ├── atendente.c             programa do atendente (main)
│   ├── demonstracao.c          roteiro linear × circular
│   ├── fila_linear.c           fila linear
│   └── demo_main.c             main que roda só a demonstração
├── testes/                     testes automáticos e medição de desempenho
├── docs/                       roteiro da demonstração, desempenho, sprint 1
├── bin/                        executáveis gerados pelo compilar.sh (fora do Git)
├── compilar.sh, testar.sh      compila tudo / compila e roda os testes
├── compilar.bat                atalho do Windows (compila dentro do Docker)
├── Dockerfile                  ambiente Linux com gcc, gdb, valgrind e git
├── docker-compose.yml          sobe o container com a pasta montada em /app
├── .devcontainer/              abre o projeto dentro do container no VS Code
└── .vscode/                    tarefas, depuração (F5) e IntelliSense
```

`fila.dat` e `fila.lock` são criados na pasta de onde os programas são chamados (a raiz do projeto) e não vão para o Git.

### Ambiente: por que Docker

O projeto roda no **Linux, dentro de um container Docker**, para todo o time ter o mesmo ambiente. A camada de armazenamento usa recursos do Linux (`rename` atômico, `nanosleep`, `getpid`) e os testes usam `fork` para criar processos de verdade. Por isso **não compila no Windows nativo**.

- O container tem gcc, gdb, valgrind e git. O código **não** é copiado para dentro dele: a pasta do projeto é montada em `/app`, então o que você edita no VS Code aparece no container na hora.
- O container fica ligado para abrir vários terminais nele, como o do cliente e o do atendente. Os dois enxergam o mesmo `fila.dat`.
- As tarefas do VS Code chamam o container sozinhas: não é preciso digitar comandos do Docker.

## Como colocar para funcionar

### Pré-requisitos

- **Docker Desktop** instalado e **aberto**.
- **VS Code** com as extensões **C/C++** e **Dev Containers** (o VS Code sugere as duas ao abrir a pasta).
- **Git**.

### Primeira vez, no VS Code

1. Clone o repositório e abra a pasta no VS Code:
   ```sh
   git clone https://github.com/gabrielsantanaf/Atendimento-por-senha.git
   code Atendimento-por-senha
   ```
2. Com o Docker Desktop aberto, aperte **`Ctrl+Shift+B`** (tarefa **Compilar tudo**). Na primeira vez ela monta o container (leva alguns minutos), compila tudo em `bin/` e copia os headers do Linux para o IntelliSense não marcar erros falsos em `unistd.h`, `sys/wait.h` etc.
3. `Ctrl+Shift+P` → **Tasks: Run Task** → **Abrir cliente e atendente lado a lado**.

Pronto: os dois programas abrem em terminais divididos, usando a mesma fila. Para conferir que está tudo certo, rode a tarefa **Rodar todos os testes**.

**Opcional, Dev Container:** `Ctrl+Shift+P` → **Dev Containers: Reopen in Container** abre o próprio editor dentro do Linux do container. O IntelliSense passa a usar o gcc de verdade e o depurador (F5) funciona.

### Pelo terminal, sem VS Code

```sh
docker compose up -d --build      # sobe o container (na primeira vez ou depois de mudar o Dockerfile)
docker compose exec dev bash      # abre um terminal dentro do container (já na pasta do projeto)
./testar.sh                       # (dentro do container) compila e roda todos os testes
```

Para usar os dois programas, abra **dois terminais**, rode `docker compose exec dev bash` em cada um e depois `./bin/cliente` em um e `./bin/atendente` no outro. Rode sempre **a partir da pasta do projeto**: o `fila.dat` é criado na pasta de onde o programa é chamado, e os dois programas precisam enxergar o mesmo arquivo.

Para desligar o container: `docker compose down`. O `compilar.bat` é um atalho que compila dentro do Docker sem abrir o container.

## Como usar (passo a passo)

1. No terminal do cliente, escolha **1 - Gerar senha** algumas vezes. Cada uma mostra o número da senha.
2. Ainda no cliente, escolha **2 - Consultar posição**, digite uma senha e veja quantas pessoas estão na frente.
3. No terminal do atendente, escolha **2 - Ver fila**: aparece o vetor, `inicio`, `fim` e `qtd`.
4. No atendente, escolha **1 - Chamar próxima**: sai a senha mais antiga. Volte ao cliente e consulte de novo: a posição diminuiu.
5. No atendente, a opção **3 - Demonstração linear × circular** roda o roteiro em memória (aperte Enter para avançar). Ela não altera a fila real.
6. No atendente, a opção **4 - Acompanhar fila ao vivo** atualiza a tela a cada 1 s; aperte Enter para voltar ao menu.

A fila tem `TAM = 5`: com 5 senhas esperando, o cliente recebe "Fila cheia" até o atendente chamar alguém. Para começar do zero, use a tarefa **Zerar a fila** (ou apague o `fila.dat`).

## Tarefas do VS Code

`Ctrl+Shift+P` → **Tasks: Run Task** e escolha:

| Tarefa | O que faz |
|---|---|
| **Compilar tudo** (`Ctrl+Shift+B`) | `./compilar.sh`: gera os executáveis em `bin/`; erros de compilação aparecem no painel Problems |
| **Rodar todos os testes** | `./testar.sh`: compila e roda os três testes automáticos |
| **Abrir cliente e atendente lado a lado** | Compila e abre os dois programas em terminais divididos, com a mesma fila |
| **Abrir cliente** / **Abrir atendente** | Um programa por vez |
| **Abrir demonstração linear x circular** | O roteiro da apresentação (`./bin/demo`) |
| **Mostrar condição de corrida (com e sem trava)** | Com trava nada se perde; sem trava várias gravações somem |
| **Medir desempenho (E-02)** | Tempo de `carregar_fila()` e `salvar_fila()` com `TAM = 1000` |
| **Zerar a fila** | Apaga `fila.dat` e `fila.lock`; a próxima senha volta a ser 1 |
| **Preparar IntelliSense no Windows** | Copia de novo os headers do Linux (a **Compilar tudo** já faz isso na primeira vez) |
| **Docker: iniciar / reconstruir / desligar container** | Para quem abriu a pasta no Windows |

As tarefas funcionam com a pasta aberta no Windows (cada comando roda no container com `docker compose exec`) e dentro do Dev Container (rodam direto no Linux).

### Depurar (F5)

Dentro do Dev Container: clique à esquerda do número da linha para pôr um breakpoint, escolha **Depurar cliente**, **Depurar atendente** ou um dos testes na aba *Run and Debug* (`Ctrl+Shift+D`) e aperte **F5**. O programa roda no terminal integrado e para no breakpoint.

## Testes

| Comando | O que testa |
|---|---|
| `./testar.sh` | Compila e roda os três testes abaixo |
| `./bin/teste_fila` | F-05: fila vazia, cheia, volta do índice para 0, ordem FIFO e posição |
| `./bin/teste_armazenamento` | A-01 e A-02: gravar e reabrir a fila idêntica; a trava faz o segundo processo esperar |
| `./bin/teste_armazenamento --sem-trava` | Mostra a condição de corrida (para a apresentação) |
| `./bin/teste_dois_processos` | A-03: 20 senhas geradas e chamadas por dois processos, nenhuma perdida |
| `./bin/medir_desempenho` | E-02: tempo com `TAM = 1000` (resultado em [docs/desempenho.md](docs/desempenho.md)) |
| `./bin/demo` | Só a demonstração linear × circular |

Os testes rodam em pastas temporárias em `/tmp` e não mexem no `fila.dat` do projeto.

### Comandos de compilação (o que o `compilar.sh` faz)

```sh
mkdir -p bin
gcc -Wall -g -Iinclude src/cliente.c src/fila.c src/armazenamento.c -o bin/cliente
gcc -Wall -g -Iinclude src/atendente.c src/demonstracao.c src/fila_linear.c src/fila.c src/armazenamento.c -o bin/atendente
gcc -Wall -g -Iinclude src/demo_main.c src/demonstracao.c src/fila_linear.c src/fila.c -o bin/demo
gcc -Wall -g -Iinclude testes/teste_fila.c src/fila.c -o bin/teste_fila
gcc -Wall -g -Iinclude testes/teste_armazenamento.c src/fila.c src/armazenamento.c -o bin/teste_armazenamento
gcc -Wall -g -Iinclude testes/teste_dois_processos.c src/fila.c src/armazenamento.c -o bin/teste_dois_processos
gcc -Wall -g -Iinclude -DTAM=1000 testes/medir_desempenho.c src/fila.c src/armazenamento.c -o bin/medir_desempenho
```

O `-Iinclude` diz ao gcc onde estão os headers, por isso os fontes continuam com `#include "fila.h"` sem caminho. O `-g` só acrescenta informação para o depurador; não muda o comportamento.

## Perguntas que costumam aparecer

**"Ler o arquivo inteiro toda vez não fica lento?"** Não nessa escala. Com `TAM = 1000` e a fila cheia, `carregar_fila()` leva cerca de 9 microssegundos neste computador. O vetor tem tamanho fixo, então o custo depende do `TAM`, não de quantas pessoas estão na fila. Detalhes em [docs/desempenho.md](docs/desempenho.md).

**"E se o programa fechar no meio e deixar o `fila.lock`?"** Se ele sair por erro, a trava é apagada sozinha. Se for morto (Ctrl+C), o outro programa avisa depois de 3 s e basta apagar o `fila.lock`. Detectar trava abandonada automaticamente ficou fora do escopo.

**"Por que não usar só a fila linear?"** Porque ela desperdiça as posições liberadas: num atendimento contínuo, diria "cheia" com o vetor quase vazio. A demonstração mostra isso.

**"Por que dois programas e não um só?"** Para simular a situação real: o totem do cliente e o guichê do atendente são lugares diferentes, acessando a mesma fila.

## Problemas comuns

| Sintoma | Causa e solução |
|---|---|
| Erros vermelhos de `#include` (`unistd.h`, `sys/wait.h`...) no VS Code | A pasta está aberta no Windows e os headers do Linux ainda não foram copiados. Rode a tarefa **Compilar tudo** (ou use **Dev Containers: Reopen in Container**). |
| `failed to connect to the docker API` | O Docker Desktop está fechado. Abra e espere ele ficar pronto. |
| `./cliente: No such file or directory` | Os executáveis ficam em `bin/`: use `./bin/cliente` (e rode `./compilar.sh` antes, se ainda não compilou). |
| `Aguardando fila.lock...` | Um programa fechou no meio e deixou a trava. Feche cliente e atendente e rode **Zerar a fila** (ou `rm fila.lock`). |
| `fila.dat esta corrompido ou foi gravado com outro TAM` | Apague o `fila.dat` (tarefa **Zerar a fila**). |
| `Fila cheia!` no cliente | Normal com `TAM = 5`: chame senhas no atendente. |

## Documentação

- [docs/roteiro-demonstracao.md](docs/roteiro-demonstracao.md): o que falar em cada passo da demonstração (D-03).
- [docs/desempenho.md](docs/desempenho.md): medição de `carregar_fila()` e `salvar_fila()` (E-02).
- [docs/sprint1-dupla-c.md](docs/sprint1-dupla-c.md): planejamento e decisões da camada de armazenamento.

## Definition of Done

- [ ] Compila sem erros e sem warnings com `gcc -Wall`.
- [ ] Cumpre o critério de aceite, mostrado rodando.
- [ ] Foi revisada por alguém de outra dupla.
- [ ] Está no repositório principal, sem quebrar o que já funcionava.
