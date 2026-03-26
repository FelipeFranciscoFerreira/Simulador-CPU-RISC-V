// defs.h
#ifndef DEFS_H
#define DEFS_H

#include <cstdint> // Para uint32_t, uint8_t, etc.

// Endereço inicial padrão (pode variar, mas 0 é comum em simuladores simples)
#define MEM_SIZE 0x90000 // 4KB de RAM (pequeno para testar)
#define PC_START 0x00000000

// --- MAPA DE MEMÓRIA (Fonte: PDF Pag 4) ---
#define RAM_BASE      0x00000000
#define RAM_LIMIT     0x0007FFFF // RAM Principal (512KB)

#define VRAM_BASE     0x00080000
#define VRAM_LIMIT    0x0008FFFF // VRAM (64KB)

#define DEVICE_BASE   0x0009FC00 // Periféricos (E/S Mapeada)
// Nota: Seu PC começa em 0x00000000

// --- OPCODES BASE (RV32I) ---
// Olhando os 7 bits finais (opcode)
// Exemplo: R-Type é 0110011 (binário) -> 0x33 (hex)
#define OP_R_TYPE   0x33 
#define OP_I_TYPE   0x13 // ex: ADDI
#define OP_LOAD     0x03 // ex: LW
#define OP_STORE    0x23 // ex: SW
#define OP_BRANCH   0x63 // ex: BEQ
#define OP_JAL      0x6F 
#define OP_JALR     0x67
#define OP_LUI      0x37
#define OP_AUIPC    0x17
#define OP_ECALL    0x73

#endif