# Medição de desempenho (E-02)

**Pergunta da apresentação:** "Ler o arquivo inteiro toda vez não fica lento?"

## Como foi medido

Programa: `testes/medir_desempenho.c`, compilado com `TAM = 1000`:

```sh
gcc -Wall -DTAM=1000 -I. testes/medir_desempenho.c fila.c armazenamento.c -o medir_desempenho
./medir_desempenho
```

- Fila **cheia** (pior caso: todas as 1000 posições ocupadas).
- 100.000 repetições de `carregar_fila()` e de `salvar_fila()`, tempo total medido com `clock()`.
- Roda numa pasta temporária em `/tmp`; não mexe no `fila.dat` do projeto.

## Resultado

| Item | Valor |
|---|---|
| `TAM` | 1000 |
| `sizeof(Fila)` | 4016 bytes (≈ 4 KB) |
| `carregar_fila()` | **≈ 1,3 microssegundos** por chamada |
| `salvar_fila()` | ≈ 37 microssegundos por chamada |

Medido em container Linux; os números variam conforme a máquina, então **rode no computador da apresentação** e anote aqui o valor real.

## Como interpretar

- Cada chamada lê ou grava 4 KB, uma fração de milissegundo. Uma pessoa apertando uma opção do menu não percebe.
- O custo depende do `TAM`, não de quantas pessoas estão na fila: o arquivo tem sempre o mesmo tamanho.
- `salvar_fila()` é mais lento porque grava num arquivo temporário e troca com `rename()`. Isso garante que o `fila.dat` nunca fica pela metade se o programa cair no meio. O custo vale a segurança.
- Esses tempos pertencem ao cenário do trabalho. Em um sistema com milhões de registros e muitos atendentes gravando por segundo, a solução seria um banco de dados, que não relê tudo a cada ação.

## Resposta pronta para a banca

> "Medimos com TAM = 1000, o pior caso, com a fila cheia: ler o arquivo leva cerca de 1 microssegundo. Como o vetor tem tamanho fixo, o custo não cresce com o número de pessoas na fila. Para esta escala não é um problema; em um sistema grande usaríamos um banco de dados."
