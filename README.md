# CPLEX Aplicado ao Problema da Mochila 0-1

Este repositório contém uma implementação robusta e eficiente em C++ utilizando a API **CPLEX Concert** para resolver o clássico **Problema da Mochila 0-1 (0-1 Knapsack Problem)** de forma exata.

---

## 🛠️ Funcionalidades do Solver

* **Modelagem Inteira Binária**: Modelado com variáveis binárias ($x_i \in \{0,1\}$), função objetivo de maximização do lucro e restrição única de peso.
* **Parser Híbrido e Defensivo**:
  * Carrega instâncias de formato chave-valor (com metadados explícitos `N:` e `CAPACITY:`).
  * Carrega diretamente instâncias no formato padrão da literatura académica da biblioteca **KPLIB**.
  * Suporta nomes de itens contendo múltiplos espaços (ex: `Item Super Premium A 50 40`).
* **Validação Estrutural**:
  * Rejeita pesos negativos ($w_i < 0.0$).
  * Emite alertas para lucros negativos ou itens com peso zero e valor positivo.
  * Alerta sobre a presença de dados extras no fim do arquivo (*garbage detection*).
* **Exportação do Modelo**: Exporta automaticamente a formulação matemática para `knapsack_model.lp` para fácil depuração no formato CPLEX LP.

---

## 💻 Como Compilar e Executar

### Pré-requisitos
* **IBM ILOG CPLEX Optimization Studio** (versão 22.2 ou compatível).
* Compilador compatível com C++11 (ex: `clang++` ou `g++`).
* Utilitário `make` para compilação automatizada.

### Passos de Compilação
1. Certifique-se de que os caminhos das bibliotecas CPLEX no [Makefile](Makefile) correspondem ao local de instalação no seu sistema.
2. Compile executando o comando:
   ```bash
   make clean && make mochila_solver
   ```

### Passos de Execução
Para resolver uma instância específica, passe o caminho do arquivo como argumento para o executável:
```bash
./mochila_solver caminho/da_instancia.dat
```

---

## 📈 Exemplo de Saída do Solver
Ao resolver uma instância estruturada, o solver produz estatísticas e a lista detalhada dos itens selecionados:

```text
Default variable names x1, x2 ... being created.
Default row names c1, c2 ... being created.
========================================
MOCHILA SOLVER STATISTICS
========================================
Arquivo de Instância:         instances/mochila_exemplo.dat
Total de Itens:               8
Itens Selecionados:           2 (25%)
Capacidade Máxima da Mochila: 165
Capacidade Utilizada:         164 (99.3939%)
Lucro Ótimo:                  319
Tempo de Execução:            0.00356104 segundos
========================================
Detalhe dos Itens Selecionados:
ID/Nome	Lucro/Valor	Peso
Item1	135		70
Item8	184		94
========================================
```

---

## 📚 Estrutura do Repositório
* `mochila_solver.cpp`: Código-fonte completo em C++ do parser e do solver CPLEX.
* `mochila_relatorio.tex`: Código LaTeX do relatório técnico com a modelagem e tabelas de resultados.
* `Relatorio do Problema da Mochila com CPLEX - Caio Anderson Bezerra Fernandes.pdf`: Relatório Técnico em PDF.
* `Makefile`: Regras de compilação C++ linkando Concert e CPLEX.
* `.gitignore`: Filtro de arquivos para evitar commitar binários e logs.
