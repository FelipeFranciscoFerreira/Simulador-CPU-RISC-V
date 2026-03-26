// instruction.h
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include "../defs.h"

// Pacote de dados prontos para a CPU executar
struct DecodedInst {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct3;
    uint32_t funct7;
    int32_t  imm; // O imediato já extendido (com sinal) e pronto para uso
    uint32_t shamt; // Shift amount (para instruções de shift)
};

class InstructionDecoder {
public:
    static DecodedInst decode(uint32_t inst) {
        DecodedInst d;
        
        // 1. Extração dos Campos Básicos (Presentes na maioria dos formatos)
        // Ver Figura 3 do PDF para conferir as posições
        d.opcode = inst & 0x7F;                  // Bits 0-6
        d.rd     = (inst >> 7)  & 0x1F;          // Bits 7-11
        d.funct3 = (inst >> 12) & 0x07;          // Bits 12-14
        d.rs1    = (inst >> 15) & 0x1F;          // Bits 15-19
        d.rs2    = (inst >> 20) & 0x1F;          // Bits 20-24
        d.funct7 = (inst >> 25) & 0x7F;          // Bits 25-31
        d.shamt  = d.rs2;                        // Shamt fica na mesma posição que rs2
        d.imm    = 0;                            // Padrão

        // 2. Extração e Extensão de Sinal dos Imediatos
        // Dependendo do Opcode, o imediato está embaralhado de forma diferente.
        
        switch (d.opcode) {
            // --- TIPO I (Immediate) ---
            // Usado em: ADDI, LW, JALR, etc.
            // Bits [31:20] viram o imediato
            case OP_I_TYPE:
            case OP_LOAD:
            case OP_JALR:
                // O cast para (int32_t) força o compilador a fazer Shift Aritmético
                // (arrasta o sinal negativo se bit 31 for 1)
                d.imm = (int32_t)inst >> 20; 
                break;

            // --- TIPO S (Store) ---
            // Usado em: SW
            // Imediato quebrado em bits [31:25] e [11:7]
            case OP_STORE: {
                int32_t imm11_5 = (int32_t)(inst & 0xFE000000) >> 20; // Bits superiores
                int32_t imm4_0  = (inst >> 7) & 0x1F;                 // Bits inferiores
                d.imm = imm11_5 | imm4_0; // Junta os dois pedaços
                break;
            }

            // --- TIPO B (Branch) ---
            // Usado em: BEQ, BNE
            // É o mais chato: bits totalmente embaralhados.
            // [31]->12, [30:25]->10:5, [11:8]->4:1, [7]->11
            case OP_BRANCH: {
                int32_t bit12 = (inst >> 31) & 1;
                int32_t bit11 = (inst >> 7) & 1;
                int32_t bit10_5 = (inst >> 25) & 0x3F;
                int32_t bit4_1  = (inst >> 8) & 0x0F;
                
                // Reconstrói o número e extende o sinal manualmente
                int32_t val = (bit12 << 12) | (bit11 << 11) | (bit10_5 << 5) | (bit4_1 << 1);
                
                // Extensão de sinal (se bit 12 for 1, preenche o resto com 1s)
                if (bit12) val |= 0xFFFFE000; 
                d.imm = val;
                break;
            }

            // --- TIPO U (Upper) ---
            // Usado em: LUI, AUIPC
            // Imediato de 20 bits nos bits [31:12]
            case OP_LUI:
            case OP_AUIPC:
                d.imm = inst & 0xFFFFF000; // Zera os 12 últimos bits
                break;

            // --- TIPO J (Jump) ---
            // Usado em: JAL
            // Também embaralhado: [31]->20, [30:21]->10:1, [20]->11, [19:12]->19:12
            case OP_JAL: {
                int32_t bit20    = (inst >> 31) & 1;
                int32_t bit10_1  = (inst >> 21) & 0x3FF;
                int32_t bit11    = (inst >> 20) & 1;
                int32_t bit19_12 = (inst >> 12) & 0xFF;

                int32_t val = (bit20 << 20) | (bit19_12 << 12) | (bit11 << 11) | (bit10_1 << 1);
                
                // Extensão de sinal
                if (bit20) val |= 0xFFE00000;
                d.imm = val;
                break;
            }
        }
        return d;
    }
};

#endif