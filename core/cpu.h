// cpu.h
#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <iomanip> // Para formatar o hex
#include "instruction.h"
#include "../defs.h"
#include "../memory/bus.h"

class CPU {
private:
    uint32_t regs[32]; // x0 a x31
    uint32_t pc;       // Program Counter
    Bus& bus;       // Referência à memória (Barramento)
    bool running;

public:
    // Construtor conecta a CPU na RAM
    CPU(Bus& bus_ref) : bus(bus_ref) {
        reset();
    }

    void reset() {
        pc = PC_START;
        running = true;
        for (int i = 0; i < 32; i++) regs[i] = 0;
        // x2 (Stack Pointer) geralmente começa no topo da memória, 
        // mas vamos deixar 0 por enquanto.
    }

    bool is_running() const { return running; }

    // Getter para PC (útil para debug)
    uint32_t get_pc() const { return pc; }

    // Leitura de Registrador
    uint32_t read_reg(int reg_index) {
        if (reg_index == 0) return 0; // x0 é sempre 0 (HARDWIRED)
        return regs[reg_index];
    }

    // Escrita em Registrador
    void write_reg(int reg_index, uint32_t value) {
        if (reg_index == 0) return; // Escritas em x0 são ignoradas
        regs[reg_index] = value;
    }

    // Função de Debug: Dump dos registradores
    void dump_regs() {
        std::cout << "--- CPU STATE (PC: 0x" << std::hex << pc << ") ---\n";
        for (int i = 0; i < 32; i += 4) {
            std::cout << "x" << std::dec << i << "-" << i+3 << ":\t";
            for (int j = 0; j < 4; j++) {
                // Formatação bonita em Hex
                std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex 
                          << regs[i+j] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "--------------------------------\n\n";
    }

    uint32_t fetch() {
        // Proteção simples para não ler lixo fora da memória
        if (pc >= MEM_SIZE) {
            std::cerr << "[ERRO] PC fora dos limites: " << pc << "\n";
            return 0;
        }
        return bus.read(pc);
    }

    // UM CICLO DE CLOCK (STEP)
    void step() {
        // 1. FETCH
        uint32_t raw_inst = fetch();
        
        // 2. DECODE
        DecodedInst d = InstructionDecoder::decode(raw_inst);

        // DEBUG: Mostrar o que foi decodificado
        std::cout << "[DECODE] PC: 0x" << std::hex << pc 
                  << " | Opcode: 0x" << (int)d.opcode 
                  << " | Rd: " << std::dec << d.rd 
                  << " | Imm: " << d.imm << "\n";

        // 3. EXECUTE (Ainda vazio, mas agora temos os dados!)
        uint32_t next_pc = pc + 4;

        execute(d, next_pc);

        // 4. UPDATE PC
        pc = next_pc;
        
        // Proteção x0
        regs[0] = 0;
    }

    void execute(DecodedInst d, uint32_t& next_pc) {
        // Variáveis auxiliares para legibilidade
        // Leitura dos registradores de origem (rs1 e rs2)
        uint32_t val_rs1 = read_reg(d.rs1);
        uint32_t val_rs2 = read_reg(d.rs2); 
        uint32_t result = 0;
        bool write_to_rd = true; // A maioria escreve em Rd, mas Branches/Stores não.

        switch (d.opcode) {
            
            // --- R-TYPE (Registrador OP Registrador) ---
            // Ex: ADD, SUB, XOR, SLT, SLL...
            case OP_R_TYPE: {
                switch (d.funct3) {
                    case 0x0: // ADD ou SUB
                        if (d.funct7 == 0x00) // ADD
                            result = val_rs1 + val_rs2;
                        else if (d.funct7 == 0x20) // SUB
                            result = val_rs1 - val_rs2;
                        break;
                    case 0x1: // SLL (Shift Left Logical)
                        result = val_rs1 << (val_rs2 & 0x1F); // Só usa os 5 bits baixos
                        break;
                    case 0x2: // SLT (Set Less Than - Signed)
                        result = ((int32_t)val_rs1 < (int32_t)val_rs2) ? 1 : 0;
                        break;
                    case 0x3: // SLTU (Set Less Than - Unsigned)
                        result = (val_rs1 < val_rs2) ? 1 : 0;
                        break;
                    case 0x4: // XOR
                        result = val_rs1 ^ val_rs2;
                        break;
                    case 0x5: // SRL ou SRA (Shifts Right)
                        if (d.funct7 == 0x00) // SRL (Logical - preenche com 0)
                            result = val_rs1 >> (val_rs2 & 0x1F);
                        else if (d.funct7 == 0x20) // SRA (Arithmetic - mantém sinal)
                            result = (int32_t)val_rs1 >> (val_rs2 & 0x1F);
                        break;
                    case 0x6: // OR
                        result = val_rs1 | val_rs2;
                        break;
                    case 0x7: // AND
                        result = val_rs1 & val_rs2;
                        break;
                }
                break;
            }

            // --- I-TYPE (Registrador OP Imediato) ---
            // Ex: ADDI, XORI, SLTI...
            case OP_I_TYPE: {
                // Aqui operamos val_rs1 com d.imm
                switch (d.funct3) {
                    case 0x0: // ADDI
                        result = val_rs1 + d.imm;
                        break;
                    case 0x1: // SLLI (Shift por constante)
                        result = val_rs1 << (d.shamt & 0x1F);
                        break;
                    case 0x2: // SLTI (Signed)
                        result = ((int32_t)val_rs1 < d.imm) ? 1 : 0;
                        break;
                    case 0x3: // SLTIU (Unsigned)
                        // Atenção: d.imm é int32, cast para uint32 para comparar sem sinal
                        result = (val_rs1 < (uint32_t)d.imm) ? 1 : 0;
                        break;
                    case 0x4: // XORI
                        result = val_rs1 ^ d.imm;
                        break;
                    case 0x5: // SRLI ou SRAI
                        if (d.funct7 == 0x00) // SRLI
                            result = val_rs1 >> (d.shamt & 0x1F);
                        else if (d.funct7 == 0x20) // SRAI
                            result = (int32_t)val_rs1 >> (d.shamt & 0x1F);
                        break;
                    case 0x6: // ORI
                        result = val_rs1 | d.imm;
                        break;
                    case 0x7: // ANDI
                        result = val_rs1 & d.imm;
                        break;
                }
                break;
            }

            // --- LUI (Load Upper Immediate) ---
            case OP_LUI:
                result = d.imm; // O imediato já foi shiftado << 12 no Decoder
                break;

            // --- AUIPC (Add Upper Immediate to PC) ---
            case OP_AUIPC:
                result = pc + d.imm;
                break;

            case OP_LOAD: {
                uint32_t addr = val_rs1 + d.imm;
                uint32_t loaded_val = 0;
                
                switch(d.funct3) {
                    case 0x0: // LB (Load Byte - Signed) -> Extende sinal
                        loaded_val = (int32_t)(int8_t)bus.read_byte(addr); 
                        break;
                    
                    case 0x1: // LH (Load Half - Signed) -> Extende sinal
                        loaded_val = (int32_t)(int16_t)bus.read_half(addr);
                        break;
                        
                    case 0x2: // LW (Load Word)
                        loaded_val = bus.read(addr);
                        break;
                        
                    case 0x4: // LBU (Load Byte - Unsigned) -> Zera os bits superiores
                        loaded_val = bus.read_byte(addr); 
                        break;
                        
                    case 0x5: // LHU (Load Half - Unsigned) -> Zera os bits superiores
                        loaded_val = bus.read_half(addr);
                        break;
                        
                    default:
                        std::cerr << "[AVISO] Funct3 de Load nao suportado: " << d.funct3 << "\n";
                        break;
                }
                result = loaded_val;
                break;
            }

            // --- STORE (Registrador -> Memória) ---
            case OP_STORE: {
                uint32_t addr = val_rs1 + d.imm;
                uint32_t data_to_write = val_rs2;
                
                switch(d.funct3) {
                    case 0x0: // SB (Store Byte)
                        bus.write_byte(addr, (uint8_t)(data_to_write & 0xFF));
                        break;

                    case 0x1: // SH (Store Half) [NOVO]
                        bus.write_half(addr, (uint16_t)(data_to_write & 0xFFFF));
                        break;

                    case 0x2: // SW (Store Word)
                        bus.write(addr, data_to_write);
                        std::cout << "   [MEM] Write 0x" << std::hex << data_to_write 
                                  << " @ 0x" << addr << "\n";
                        break;
                        
                    default:
                        std::cerr << "[AVISO] Funct3 de Store nao suportado\n";
                        break;
                }
                
                write_to_rd = false;
                break;
            }

            case OP_BRANCH: {
                bool take_branch = false;
                
                // Compara val_rs1 com val_rs2
                switch (d.funct3) {
                    case 0x0: // BEQ (Branch Equal)
                        take_branch = (val_rs1 == val_rs2);
                        break;
                    case 0x1: // BNE (Branch Not Equal)
                        take_branch = (val_rs1 != val_rs2);
                        break;
                    case 0x4: // BLT (Less Than - Signed)
                        take_branch = ((int32_t)val_rs1 < (int32_t)val_rs2);
                        break;
                    case 0x5: // BGE (Greater Equal - Signed)
                        take_branch = ((int32_t)val_rs1 >= (int32_t)val_rs2);
                        break;
                    case 0x6: // BLTU (Less Than - Unsigned)
                        take_branch = (val_rs1 < val_rs2);
                        break;
                    case 0x7: // BGEU (Greater Equal - Unsigned)
                        take_branch = (val_rs1 >= val_rs2);
                        break;
                }

                if (take_branch) {
                    // PC = PC + Offset (Imediato)
                    next_pc = pc + d.imm;
                    std::cout << "   [BRANCH] Taken! Jumping to 0x" << std::hex << next_pc << "\n";
                } else {
                    std::cout << "   [BRANCH] Not Taken.\n";
                }
                
                write_to_rd = false; // Branch não escreve em registrador
                break;
            }

            // --- JAL (Jump And Link - Incondicional) ---
            case OP_JAL: {
                // 1. Salva o endereço de retorno (PC+4) em rd (link)
                write_reg(d.rd, pc + 4);
                
                // 2. Calcula o salto (PC atual + Imediato)
                next_pc = pc + d.imm;
                
                std::cout << "   [JAL] Jump to 0x" << std::hex << next_pc 
                          << ", Return Addr saved in x" << std::dec << d.rd << "\n";
                
                write_to_rd = false; // Já escrevemos manualmente acima
                break;
            }

            case OP_JALR: {
                // 1. Salva o endereço de retorno (PC+4)
                write_reg(d.rd, pc + 4);
                
                // 2. Calcula salto: (rs1 + imm) 
                // A especificação diz para zerar o bit menos significativo (& ~1)
                next_pc = (val_rs1 + d.imm) & ~1;
                
                std::cout << "   [JALR] Jump to 0x" << std::hex << next_pc 
                          << " (Ret Addr stored in x" << std::dec << d.rd << ")\n";
                
                write_to_rd = false;
                break;
            }

            case OP_ECALL: { // 0x73
                // ECALL é Environment Call (chamada de sistema)
                // Vamos usar a convenção do simulador RARS:
                // Se a7 (x17) == 10, então é EXIT.
                
                uint32_t syscall_code = read_reg(17); // x17 é a7
                
                if (syscall_code == 10) { // Code 10 = Exit
                    std::cout << "   [SYS] ECALL: Exit code detected. Halting CPU.\n";
                    running = false; // Desliga a CPU
                } else {
                    std::cout << "   [SYS] ECALL: Unknown code " << syscall_code << "\n";
                }
                
                write_to_rd = false;
                break;
            }

            // --- NOP ou Não implementado ainda ---
            default:
                write_to_rd = false; // Não faz nada
                break;
        }

        // 4. WRITE BACK (Escrever no registrador de destino)
        if (write_to_rd && d.rd != 0) {
            write_reg(d.rd, result);
            
            // DEBUG: Mostrar o resultado calculado
            std::cout << "   [EXEC] x" << std::dec << d.rd << " = 0x" 
                      << std::hex << result << "\n";
        }
    }
};

#endif