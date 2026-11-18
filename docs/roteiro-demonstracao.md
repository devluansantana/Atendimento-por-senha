# Roteiro da demonstração: fila linear × fila circular (D-03)

**História:** D-03, Dupla B (Arthur e Bruno). Precisa ser aprovado pelo PO (João Ferreira).

Como apresentar: no terminal, na pasta do projeto, rodar `./bin/atendente` e escolher a opção **3 - Demonstração**. A tela para a cada passo e espera Enter, então dá tempo de explicar. Também dá para rodar só a demonstração com `./bin/demo` (no VS Code: tarefa **Abrir demonstração linear x circular**).

A demonstração roda toda em memória e **não mexe no `fila.dat`**. Usa `TAM = 5`.

---

## Antes de começar (fala de abertura)

> "O sistema real usa a fila **circular**. A linear aparece só aqui, para mostrar por que ela não serve. Vamos rodar as duas lado a lado com o mesmo roteiro e olhar o vetor e os índices a cada passo. O `_` é uma posição livre."

## Passo 1: enfileirar as senhas 1 a 5

**Na tela:** as duas ficam `[1 2 3 4 5]`. Linear com `fim = 5`; circular com `fim = 0`.

**O que falar:**
- "As duas ficaram cheias do mesmo jeito. A diferença está no `fim`."
- **Operador módulo:** "Na linear, o `fim` foi para 5, que já passou da última posição (a 4). Na circular, `fim = (4 + 1) % 5 = 0`. O módulo faz o índice **voltar para o começo** depois da última posição."

## Passo 2: chamar 3 senhas

**Na tela:** as duas ficam `[_ _ _ 4 5]`, com `inicio = 3` e `qtd = 2`.

**O que falar:**
- **FIFO:** "Saíram a 1, a 2 e a 3, nessa ordem. Primeiro a entrar, primeiro a sair. Nas duas filas."
- "Agora existem 3 posições livres no começo do vetor. Guardem essa informação para o próximo passo."

## Passo 3: tentar enfileirar a senha 6

**Na tela:**
- Linear: **"Fila cheia!"**
- Circular: a 6 entra na posição 0, `[6 _ _ 4 5]`.

**O que falar:**
- **Limitação da linear:** "O vetor tem 3 posições livres, mas a linear diz que está cheia. Os índices só andam para frente, então o espaço liberado no começo **nunca mais é usado**."
- "Num atendimento que não para, a linear ficaria 'cheia' com o vetor quase vazio. A circular reaproveita o espaço usando o módulo."

## Passo 4: chamar a próxima senha

**Na tela:** sai a 4 nas duas. Circular `[6 _ _ _ 5]` com `inicio = 4` e `fim = 1`.

**O que falar:**
- **FIFO de novo:** "Saiu a 4, a mais antiga. Depois dela vem a 5 e só então a 6. A ordem de chegada é respeitada mesmo a 6 estando na posição 0 do vetor."
- **Por que existe o `qtd`:** "Na circular, `inicio == fim` acontece tanto com a fila **vazia** quanto com a fila **cheia** (no fim do passo 1, os dois eram 0 e a fila estava cheia). Sem outro dado, não dá para diferenciar. O contador `qtd` resolve: `qtd == 0` é vazia, `qtd == TAM` é cheia."

---

## Resumo em 4 frases (para fechar)

1. **FIFO:** as duas filas chamam sempre a senha mais antiga.
2. **Limitação da linear:** os índices só avançam, e as posições liberadas se perdem.
3. **Operador módulo:** `(indice + 1) % TAM` volta para 0 e reaproveita o espaço.
4. **`qtd`:** distingue fila vazia de fila cheia quando `inicio == fim`.

## Perguntas que podem aparecer

**"Por que não usar só a fila linear?"**
Porque ela desperdiça as posições liberadas. A demonstração mostra isso: "cheia" com 3 posições livres.

**"E se ao invés de módulo a gente mover todos os elementos para o começo a cada chamada?"**
Funcionaria, mas cada chamada custaria percorrer o vetor inteiro. Na circular a chamada é uma conta só, independente do tamanho.

**"Por que o vetor tem tamanho fixo (5)?"**
Pela simplicidade e porque o arquivo `fila.dat` guarda a struct inteira num bloco só. O tamanho se muda em `TAM` no `include/fila.h`; para o teste de desempenho usamos 1000.

## Checklist de aprovação (PO)

- [ ] Os 4 passos batem com a tabela do documento de arquitetura
- [ ] As 4 explicações (FIFO, limitação da linear, módulo, `qtd`) estão presentes
- [ ] O texto é falável em até 3 minutos
