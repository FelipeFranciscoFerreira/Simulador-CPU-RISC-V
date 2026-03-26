# RISC-V RV32I Processor Simulator

Este projeto é um simulador de ciclo único para a arquitetura **RISC-V (RV32I)** desenvolvido em C++. Ele emula o comportamento de um processador real, realizando as etapas de busca, decodificação e execução de instruções, com suporte a saída de vídeo mapeada em memória.

---

### Arquitetura do Sistema

O simulador é dividido em componentes modulares que replicam a organização de hardware:

* **Unidade de Processamento (CPU)**: Gerencia 32 registradores de propósito geral e o Program Counter (PC).
* **Barramento de Dados (Bus)**: Atua como o hub central, direcionando requisições de leitura e escrita para o componente correto com base no endereço.
* **Memória RAM**: Implementação física da memória principal utilizando armazenamento em bytes e suporte a *Little Endian*.
* **Monitor VRAM**: Renderiza caracteres ASCII em uma grade de $64 \times 16$ posições diretamente do mapa de memória.

---

### Mapa de Memória

O endereçamento de 32 bits é segmentado para permitir a comunicação com diferentes periféricos:

| Região | Endereço Inicial | Endereço Limite | Função |
| :--- | :--- | :--- | :--- |
| **RAM Principal** | `0x00000000` | `0x0007FFFF` | Armazenamento de instruções e dados (512KB). |
| **VRAM (Vídeo)** | `0x00080000` | `0x0008FFFF` | Memória de vídeo para saída de texto (64KB). |
| **I/O Mapeado** | `0x0009FC00` | - | Endereço base para periféricos e dispositivos externos. |

---

### Instruções Suportadas (RV32I)

O decodificador está preparado para processar os seguintes formatos:

* **Tipo-R**: Instruções aritméticas entre registradores (ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT).
* **Tipo-I**: Operações com imediatos (ADDI, XORI, ORI, ANDI) e instruções de LOAD (LW, LB, LH).
* **Tipo-S**: Instruções de escrita em memória (SW, SB, SH).
* **Tipo-B**: Saltos condicionais baseados em comparação (BEQ, BNE, BLT, BGE).
* **Tipo-U/J**: Carregamento de imediatos superiores e saltos incondicionais (LUI, AUIPC, JAL, JALR).
* **ECALL**: Chamada de sistema para finalizar a execução via código `10` no registrador `x17`.

---

### Organização de Arquivos

```text
📂 Projeto_Simulador
├── 📄 main.cpp            # Ponto de entrada e loop principal
├── 📄 defs.h              # Definições de constantes e opcodes
├── 📄 instruction.h       # Lógica de decodificação de instruções
├── 📂 core
│   └── 📄 cpu.h           # Implementação da CPU e Unidade de Execução
├── 📂 memory
│   ├── 📄 bus.h           # Lógica do barramento de comunicação
│   └── 📄 memory.h        # Implementação da RAM física
└── 📂 peripherals
    └── 📄 io.h            # Sistema de monitor e renderização de VRAM
```

---

### Guia de Uso

#### Compilação
Utilize o compilador G++ para gerar o executável:
```bash
g++ main.cpp -o main
```

#### Execução
O simulador lê as instruções em formato hexadecimal de um arquivo `prog.txt`. Para rodar e gerar um relatório simultâneo:
```powershell
./main.exe | Tee-Object -FilePath "relatorio_simulacao.txt"
```

> **Nota**: O simulador possui um limite de segurança de 1000 ciclos para evitar loops infinitos caso o programa carregado não contenha um `ECALL` de encerramento.

---

### Exemplo de Saída (Trace de Execução)

O log detalha cada etapa do ciclo de instrução e o estado da VRAM:

```text
[DECODE] PC: 0x14 | Opcode: 0x33 | Rd: 9 | Imm: 0
   [EXEC] x9 = 0x42
[DECODE] PC: 0x18 | Opcode: 0x23 | Rd: 0 | Imm: 0
[DECODE] PC: 0x1c | Opcode: 0x13 | Rd: 5 | Imm: 1
   [EXEC] x5 = 0x2
```

---

Esta tabela de referência detalha como o componente `InstructionDecoder` interpreta os bits brutos de 32 bits para preencher a estrutura `DecodedInst`, permitindo que a CPU execute a lógica correta.

---

### Visão Geral dos Formatos RISC-V

Para entender como o simulador processa as instruções, é fundamental visualizar como os 32 bits são distribuídos entre opcodes, registradores e valores imediatos.



### Tabela de Comparação de Tipos

| Tipo | Opcode (bits 0-6) | Campos Extraídos | Tratamento do Imediato (`imm`) | Exemplo no Código |
| :--- | :--- | :--- | :--- | :--- |
| **R** (Register) | `0x33` | `rd`, `rs1`, `rs2`, `funct3`, `funct7` | Não possui imediato (valor padrão 0). | `ADD`, `SUB`, `XOR` |
| **I** (Immediate) | `0x13`, `0x03`, `0x67` | `rd`, `rs1`, `funct3`, `imm` | Bits com extensão de sinal. | `ADDI`, `LW`, `JALR` |
| **S** (Store) | `0x23` | `rs1`, `rs2`, `funct3`, `imm` | Combina bits e [11:7]. | `SW`, `SB`, `SH` |
| **B** (Branch) | `0x63` | `rs1`, `rs2`, `funct3`, `imm` | Bits embaralhados; inclui bit de sinal no 31. | `BEQ`, `BNE`, `BLT` |
| **U** (Upper) | `0x37`, `0x17` | `rd`, `imm` | Bits; os 12 bits inferiores são zerados. | `LUI`, `AUIPC` |
| **J** (Jump) | `0x6F` | `rd`, `imm` | Reconstrução complexa de 20 bits com sinal. | `JAL` |

---

### Lógica Interna de Decodificação

O simulador utiliza técnicas de manipulação de bits (bitmasking e shifts) para isolar cada campo. Abaixo estão os pontos críticos da implementação:

* **Extensão de Sinal**: Para os tipos **I**, **S**, **B** e **J**, o simulador garante que valores negativos sejam preservados ao converter o imediato para um inteiro de 32 bits assinado (`int32_t`).
* **Opcodes Base**: O seletor principal da CPU utiliza as constantes definidas no arquivo de cabeçalho para direcionar o fluxo de execução.
* **Campos Fixos**: Independentemente do tipo, os bits de 0 a 6 sempre representam o `opcode`, facilitando a primeira etapa da decodificação.
* **Shamt**: Para instruções de deslocamento (como `SLLI`), o campo `shamt` compartilha a mesma posição física do registrador `rs2`.

> **Nota de Implementação**: O decodificador preenche todos os campos (`rd`, `rs1`, `rs2`, etc.) para todas as instruções. No entanto, a CPU ignora os campos que não pertencem ao formato específico da instrução atual.

---
