// memory/bus.h
#ifndef BUS_H
#define BUS_H

#include <iostream>
#include <vector>
#include "../defs.h"
#include "memory.h"

class Bus {
private:
    Memory& ram; // Referência à RAM física

public:
    // O Barramento precisa saber quem está conectado nele
    Bus(Memory& ram_ref) : ram(ram_ref) {}

    // --- LEITURA (Load) ---
    uint32_t read(uint32_t address) {
        // 1. Verifica se é RAM Principal
        if (address >= RAM_BASE && address <= RAM_LIMIT) {
            return ram.read_word(address);
        }
        
        // 2. Verifica se é VRAM (Vídeo) - Implementaremos depois
        if (address >= VRAM_BASE && address <= VRAM_LIMIT) {
            std::cout << "[BUS] Leitura da VRAM (Ainda nao implementado): " << std::hex << address << "\n";
            return 0;
        }

        std::cerr << "[BUS-ERRO] Tentativa de leitura em endereco invalido: " << std::hex << address << "\n";
        return 0;
    }

    uint8_t read_byte(uint32_t address) {
        // Validações de endereço iguais ao read()
        if (address >= RAM_BASE && address <= RAM_LIMIT) return ram.read_byte(address);
        if (address >= VRAM_BASE && address <= VRAM_LIMIT) return ram.read_byte(address);
        if (address >= DEVICE_BASE) return 0; // Leitura de byte em IO não implementada
        return 0;
    }

    // --- ESCRITA (Store) ---
    void write(uint32_t address, uint32_t value) {
        // 1. RAM Principal
        if (address >= RAM_BASE && address <= RAM_LIMIT) {
            ram.write_word(address, value);
            return;
        }

        // 2. VRAM (Vídeo)
        if (address >= VRAM_BASE && address <= VRAM_LIMIT) {
            ram.write_word(address, value);
            // No futuro, aqui você guarda num buffer de vídeo
            //std::cout << "[BUS] Escrita na VRAM @ " << std::hex << address << " val: " << value << "\n";
            return;
        }
        
        // 3. E/S (Teclado/Periféricos)
        if (address >= DEVICE_BASE) {
             std::cout << "[BUS] Escrita em Periferico (IO) @ " << std::hex << address << " val: " << value << "\n";
             return;
        }

        std::cerr << "[BUS-ERRO] Tentativa de escrita em endereco invalido: " << std::hex << address << "\n";
    }

    void write_byte(uint32_t address, uint8_t value) {
        // 1. RAM Principal
        if (address >= RAM_BASE && address <= RAM_LIMIT) {
            ram.write_byte(address, value); // A RAM já tem esse método da Fase 1
            return;
        }

        // 2. VRAM (Vídeo)
        if (address >= VRAM_BASE && address <= VRAM_LIMIT) {
            ram.write_byte(address, value);
            return;
        }
        
        // 3. E/S
        if (address >= DEVICE_BASE) {
             std::cout << "[BUS] Write Byte IO @ " << std::hex << address 
                       << " val: '" << (char)value << "'\n";
             return;
        }

        std::cerr << "[BUS-ERRO] Write Byte invalido: " << std::hex << address << "\n";
    }

    // --- LEITURA DE 16 BITS (HALF) ---
    uint16_t read_half(uint32_t address) {
        if (address >= RAM_BASE && address <= RAM_LIMIT) {
            // Reconstrói 2 bytes (Little Endian)
            return (uint16_t)ram.read_byte(address) | 
                   ((uint16_t)ram.read_byte(address + 1) << 8);
        }
        // ... (VRAM e I/O geralmente não usam Half, mas pode adicionar se quiser)
        return 0;
    }

    // --- ESCRITA DE 16 BITS (HALF) ---
    void write_half(uint32_t address, uint16_t value) {
        if (address >= RAM_BASE && address <= RAM_LIMIT) {
            ram.write_byte(address, (uint8_t)(value & 0xFF));
            ram.write_byte(address + 1, (uint8_t)((value >> 8) & 0xFF));
            return;
        }
        // ... (VRAM)
        if (address >= VRAM_BASE && address <= VRAM_LIMIT) {
             ram.write_byte(address, (uint8_t)(value & 0xFF));
             ram.write_byte(address + 1, (uint8_t)((value >> 8) & 0xFF));
             return;
        }
    }
};

#endif