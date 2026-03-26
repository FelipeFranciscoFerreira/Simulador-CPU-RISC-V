// peripherals/io.h
#ifndef IO_H
#define IO_H

#include <iostream>
#include <iomanip> // Necessário para std::hex, std::setw
#include "../memory/memory.h"
#include "../defs.h"

class Monitor {
public:
    static void dump_vram(Memory& mem) {
        // Configuração da "Tela"
        const int COLUMNS = 64;
        const int ROWS = 16; 
        
        std::cout << "\n======================== MONITOR VRAM (" << COLUMNS << "x" << ROWS << ") ========================\n";
        
        for (int row = 0; row < ROWS; row++) {
            // Imprime o endereço base da linha
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::hex 
                      << (VRAM_BASE + row * COLUMNS) << " | ";

            for (int col = 0; col < COLUMNS; col++) {
                uint32_t addr = VRAM_BASE + (row * COLUMNS) + col;
                uint8_t byte = mem.read_byte(addr);
                
                // 1. Se for zero, espaço vazio (limpa a tela)
                if (byte == 0) {
                    std::cout << " "; 
                } 
                // 2. Se for caractere imprimível (Letras, Números, Pontuação)
                else if (byte >= 32 && byte <= 126) {
                    std::cout << (char)byte;
                } 
                // 3. Se for controle (Fibonacci, enter, tab...), mostra em HEX
                else {
                    // Imprime [XX]
                    std::cout << "[" << std::setw(2) << std::setfill('0') << std::hex << (int)byte << "]";
                }
            }
            // Fecha a linha.
            // Nota: Como o HEX ocupa mais espaço que uma letra, a borda direita
            // vai ficar desalinhada nas linhas que tiverem números. Isso é normal!
            std::cout << " |\n"; 
        }
        std::cout << "====================================================================\n";
    }
};

#endif