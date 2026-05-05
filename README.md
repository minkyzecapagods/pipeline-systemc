# Pipeline MIPS - Simulador em SystemC

Simulação de um processador RISC com pipeline de 5 estágios, implementado em SystemC 3.0.2 (C++17). Desenvolvido como trabalho prático da disciplina **DIM0129 - Organização de Computadores** (UFRN).


## Visão Geral

O projeto modela a organização completa de um processador RISC inspirado no MIPS, com **Parte Operativa (PO)** e **Parte de Controle (PC)** separadas, executando instruções em pipeline. A simulação gera formas de onda no formato VCD, visualizáveis no GTKWave.

### ISA Suportada

| Instrução | Operação |
|-----------|----------|
| `AND` | E lógico bit a bit |
| `OR` | OU lógico bit a bit |
| `XOR` | OU exclusivo bit a bit |
| `NOT` | Negação bit a bit |
| `CMP` | Comparação (subtrai e usa flags) |
| `ADD` | Adição |
| `SUB` | Subtração |
| `LD` | Leitura da memória de dados |
| `ST` | Escrita na memória de dados |
| `J` | Salto incondicional |
| `JN` | Salto condicional - se resultado negativo |
| `JZ` | Salto condicional - se resultado zero |


## Arquitetura do Pipeline

O pipeline é composto por **5 estágios**:

```
IF → ID → EX → MEM → WB
```

| Estágio | Nome | Descrição |
|---------|------|-----------|
| **IF** | Instruction Fetch | Lê a instrução da memória, incrementa o PC |
| **ID** | Instruction Decode | Decodifica a instrução, lê registradores, gera sinais de controle |
| **EX** | Execute | Executa a operação na ULA, calcula endereço de salto |
| **MEM** | Memory Access | Acessa a memória de dados (LD/ST) |
| **WB** | Write Back | Escreve o resultado no banco de registradores |

### Decisões de Projeto

- **Palavra:** 32 bits;
- **Banco de registradores:** 32 registradores (`$0`–`$31`), `$0` sempre zero;
- **Memória de instruções (ROM):** 1024 × 32 bits;
- **Memória de dados (RAM):** 1024 × 32 bits (endereçamento por palavra);
- **Formato tipo R:** `opcode(6) | rs(5) | rt(5) | rd(5) | shamt(5) | funct(6)`;
- **Formato tipo I:** `opcode(6) | rs(5) | rt(5) | imediato(16)`;
- **Extensão de sinal:** imediato de 16 bits para 32 bits (com sinal);
- **Modos de endereçamento:** registrador, imediato e base + deslocamento (LD/ST);
- **Dependências de dados:** resolvidas com *NOPs* manuais (bolhas explícitas nos programas de teste);
- **Dependências de controle:** saltos resolvidos no estágio EX; instruções no *delay slot* são executadas.

## Estrutura do Projeto

```
pipeline-systemc/

├── docs/ 
│   ├── diagram.png             # Diagrama de blocos
│   ├── especificacao.pdf       # Especificações para o trabalho
│   ├── estados_pipeline.png    # Diagrama de pipeline
│   └── relatorio.pdf           # Relatório para entrega do trabalho
├── include/
│   ├── banco_registradores.h   # Banco de 32 registradores (leitura combinacional, escrita síncrona)
│   ├── controle.h              # Unidade de controle principal (decodifica opcode)
│   ├── controle_ula.h          # Unidade de controle da ULA (decodifica funct)
│   ├── extensor_sinal.h        # Extensão de sinal 16 para 32 bits
│   ├── memoria_dados.h         # RAM de dados (1024 palavras)
│   ├── memoria_instrucoes.h    # ROM de instruções (1024 palavras)
│   ├── mips_top.h              # Top-level: instancia e conecta todos os módulos
│   ├── mux2.h                  # Multiplexadores 2:1 (Int32, Uint5, Uint32)
│   ├── pc.h                    # Program Counter (síncrono com reset)
│   ├── registradores_pipeline.h# Registradores IF/ID, ID/EX, EX/MEM, MEM/WB
│   ├── separador_instrucao.h   # Fatiador de instrução em campos (opcode, rs, rt, rd, imm)
│   ├── somador.h               # Somadores para PC+4 e cálculo de branch
│   ├── ula.h                   # ULA: AND, OR, ADD, XOR, NOT, SUB, CMP
│   └── unidade_saltos.h        # Decide se o salto é tomado (J, JN, JZ)
├── src/
│   └── main.cpp                # Testbench: lê programa em hex, instancia o top, gera VCD
├── tests/
│   ├── test1.txt               # Teste: LRI, ADD, SUB
│   ├── test2.txt               # Teste: ST, LD e verificação com ADD
│   └── test3.txt               # Teste: JZ com delay slot
├── LICENSE
├── Makefile
└── README.md
```

## Dependências

| Ferramenta | Versão recomendada |
|------------|-------------------|
| g++ | ≥ 9 (suporte a C++17) |
| SystemC | 3.0.2 (instalado em `/usr/local/systemc-3.0.2`) |
| GTKWave | qualquer versão recente |

> **Instalação do SystemC:** siga as instruções oficiais em [systemc.org](https://systemc.org). O `Makefile` já aponta para `/usr/local/systemc-3.0.2` (ajuste os caminhos `CXXFLAGS` e `LDFLAGS` se necessário).

## Como Usar

### 1. Compilar

```bash
make build
```
O executável é gerado em `build/mips_sim`.

### 2. Executar uma simulação

Passe um arquivo de instruções (em hexadecimal, um opcode por linha):

```bash
make run ARGS=tests/test1.txt
```

O arquivo de formas de onda `simulacao.vcd` é movido automaticamente para `waves/`.

### 3. Visualizar as ondas

```bash
make run_waves
```

Abre `waves/simulacao.vcd` no GTKWave. Sinais já rastreados:

| Sinal | Descrição |
|-------|-----------|
| `1_Clock` | Clock do sistema |
| `2_IF_Instrucao` | Instrução no estágio IF/ID |
| `3_ID_RS` / `4_ID_RT` | Registradores fonte lidos no ID |
| `5_EX_Resultado_ULA` | Resultado da ULA no estágio EX |
| `6_WB_Dado_Escrito` | Dado escrito no WB |
| `7_WB_Reg_Destino` | Registrador destino no WB |

### 4. Limpar artefatos

```bash
make clean
```

### Resumo dos comandos

```bash
make                            # Sugere a ajuda
make help                       # Exibe ajuda
make build                      # Compila
make run ARGS=tests/test1.txt   # Compila e simula
make run_waves                  # Abre GTKWave
make clean                      # Remove build/ e waves/
```

## Programas de Teste

Os arquivos em `tests/` são programas escritos em hexadecimal (um opcode por linha). Linhas iniciadas com `#` são comentários e são ignoradas.

### test1.txt - Operações aritméticas

Carrega dois valores com LRI, insere NOPs para resolver dependências de dados e então executa ADD e SUB. Verifica o funcionamento básico do pipeline e do Write-Back.

```
LRI $1, 5      → $1 = 5
LRI $2, 3      → $2 = 3
NOP × 3        → aguarda WB
ADD $3, $1, $2 → $3 = 8
NOP × 3
SUB $4, $3, $2 → $4 = 5
```

### test2.txt - Memória (LD/ST)

Demonstra o ciclo completo de escrita e leitura na RAM de dados.

```
LRI $1, 15     → $1 = 15
LRI $2, 4      → $2 = 4 (endereço base)
NOP × 3
ST $1, 0($2)   → mem[4] = 15
NOP × 3
LD $3, 0($2)   → $3 = mem[4] = 15
NOP × 3
ADD $4, $3, $3 → $4 = 30 (confirma leitura)
```

### test3.txt - Salto condicional (JZ) com delay slot

Demonstra o comportamento de salto quando a condição é satisfeita. As duas instruções imediatamente após o `JZ` estão no *delay slot* e são executadas independentemente do salto.

```
LRI $1, 10 / LRI $2, 10
JZ $1, $2, +4   → 10 == 10 → salta
ADD $5, ... / ADD $6, ...  ← executados (delay slot)
SUB $7, ... / SUB $8, ...  ← pulados
AND $9, $1, $2  ← alvo do salto
```

## Formato de Arquivo de Instruções

Crie seus próprios programas com arquivos `.txt` seguindo o formato:

```
# Comentário (linha ignorada)
20010005   # LRI $1, 5   (hex de 32 bits)
00000000   # NOP
00221820   # ADD $3, $1, $2
```

Execute com:

```bash
make run ARGS=meu_programa.txt
```

---

## Contexto Acadêmico

Trabalho prático da disciplina **DIM0129 - Organização de Computadores**, UFRN.  
Especificação disponível em [`docs/especificacao.pdf`](docs/especificacao.pdf).

O trabalho exigia: implementação de todos os componentes da PO e PC individualmente, integração do pipeline completo e simulação de pelo menos 3 algoritmos com análise de desempenho (ciclos de clock por instrução, tratamento de dependências de dados e controle).

## Autores
Este projeto foi criado por @minkyzecapagods e @olive-mdrs.