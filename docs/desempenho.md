# Medição de desempenho (E-02)

**Pergunta da apresentação:** "Ler o arquivo inteiro toda vez não fica lento?"

## Como foi medido

Programa: `testes/medir_desempenho.c`, compilado com `TAM = 1000`:

```sh
gcc -Wall -DTAM=1000 -Iinclude testes/medir_desempenho.c src/fila.c src/armazenamento.c -o bin/medir_desempenho
./bin/medir_desempenho
```

O `./compilar.sh` já faz essa compilação; no VS Code, use a tarefa **Medir desempenho (E-02)**.

- Fila **cheia** (pior caso: todas as 1000 posições ocupadas).
- 100.000 repetições de `carregar_fila()` e de `salvar_fila()`, tempo total medido com `clock()`.
- Roda numa pasta temporária em `/tmp`; não mexe no `fila.dat` do projeto.

## Resultado

| Item | Valor |
|---|---|
| `TAM` | 1000 |
| `sizeof(Fila)` | 4016 bytes (≈ 4 KB) |
| `carregar_fila()` | **≈ 9 microssegundos** por chamada (0,009 ms) |
| `salvar_fila()` | ≈ 130 microssegundos por chamada (0,13 ms) |

Medido em 18/11/2026 no computador da apresentação (Windows + Docker Desktop, container Linux), duas execuções seguidas: 8,9 e 9,3 µs para ler; 125 e 132 µs para gravar. Em outra máquina, mais rápida, a mesma medição deu ≈ 1,3 µs e ≈ 37 µs: os números variam conforme o computador, mas ficam sempre muito abaixo de 1 milissegundo. Se apresentar em outro computador, rode de novo (tarefa **Medir desempenho (E-02)** no VS Code) e anote aqui.

## Como interpretar

- Cada chamada lê ou grava 4 KB, uma fração de milissegundo. Uma pessoa apertando uma opção do menu não percebe.
- O custo depende do `TAM`, não de quantas pessoas estão na fila: o arquivo tem sempre o mesmo tamanho.
- `salvar_fila()` é mais lento porque grava num arquivo temporário e troca com `rename()`. Isso garante que o `fila.dat` nunca fica pela metade se o programa cair no meio. O custo vale a segurança.
- Esses tempos pertencem ao cenário do trabalho. Em um sistema com milhões de registros e muitos atendentes gravando por segundo, a solução seria um banco de dados, que não relê tudo a cada ação.

## Resposta pronta para a banca

> "Medimos com TAM = 1000, o pior caso, com a fila cheia: ler o arquivo leva cerca de 9 microssegundos, menos de um centésimo de milissegundo. Como o vetor tem tamanho fixo, o custo não cresce com o número de pessoas na fila. Para esta escala não é um problema; em um sistema grande usaríamos um banco de dados."
