// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <iostream>
#include <iomanip>
#include "../defs.h"

class Memory {
private:
    std::vector<uint8_t> data; // O hardware físico (bytes)

public:
    Memory() {
        data.resize(MEM_SIZE, 0); // Inicia tudo com 0
    }

    // Leitura de 1 Byte (sem sinal)
    uint8_t read_byte(uint32_t address) {
        if (address >= MEM_SIZE) {
            std::cerr << "[ERRO] Leitura fora da memoria: " << address << "\n";
            return 0;
        }
        return data[address];
    }

    // Escrita de 1 Byte
    void write_byte(uint32_t address, uint8_t value) {
        if (address >= MEM_SIZE) {
            std::cerr << "[ERRO] Escrita fora da memoria: " << address << "\n";
            return;
        }
        data[address] = value;
    }

    // Leitura de 1 Word (32 bits) - LITTLE ENDIAN
    uint32_t read_word(uint32_t address) {
        if (address + 3 >= MEM_SIZE) {
            std::cerr << "[ERRO] Leitura de Word fora dos limites\n";
            return 0;
        }
        // Monta os 4 bytes:
        // Byte 0 (LSB) | Byte 1 << 8 | Byte 2 << 16 | Byte 3 << 24
        return (uint32_t)data[address] |
               ((uint32_t)data[address + 1] << 8) |
               ((uint32_t)data[address + 2] << 16) |
               ((uint32_t)data[address + 3] << 24);
    }

    // Escrita de 1 Word (32 bits) - LITTLE ENDIAN
    void write_word(uint32_t address, uint32_t value) {
        if (address + 3 >= MEM_SIZE) {
            std::cerr << "[ERRO] Escrita de Word fora dos limites\n";
            return;
        }
        // Desmonta o int32 em 4 bytes
        data[address]     = (uint8_t)(value & 0xFF);         // Pega os 8 bits finais
        data[address + 1] = (uint8_t)((value >> 8) & 0xFF);  // Pega os próx 8 bits
        data[address + 2] = (uint8_t)((value >> 16) & 0xFF);
        data[address + 3] = (uint8_t)((value >> 24) & 0xFF);
    }
    
    // Auxiliar para carregar dados direto (para testes)
    void load_program(const std::vector<uint32_t>& program) {
        uint32_t addr = 0;
        for (uint32_t word : program) {
            write_word(addr, word);
            addr += 4;
        }
    }
};

#endif