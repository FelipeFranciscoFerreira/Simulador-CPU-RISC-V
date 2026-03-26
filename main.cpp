// main.cpp
#include <iostream>
#include <fstream>  // Para ler arquivo
#include <sstream>  // Para converter string hex
#include <string>
#include <vector>
#include <thread>   // Opcional: para sleep (animação)
#include <chrono>   // Opcional: para tempo

#include "defs.h"
#include "memory/memory.h"
#include "memory/bus.h"
#include "core/cpu.h"
#include "peripherals/io.h"

// Função auxiliar para carregar arquivo Hex
void load_program_from_file(Bus& bus, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo " << filename << "\n";
        exit(1);
    }

    std::string line;
    uint32_t addr = RAM_BASE;
    
    std::cout << "--- Carregando Programa: " << filename << " ---\n";

    while (std::getline(file, line)) {
        // Ignora linhas vazias
        if (line.empty()) continue;

        // Converte string hex "00500113" para uint32_t
        uint32_t instruction;
        std::stringstream ss;
        ss << std::hex << line;
        ss >> instruction;

        // Escreve na memória via Barramento
        bus.write(addr, instruction);
        addr += 4;
    }
    file.close();
    std::cout << "Programa carregado com sucesso! (" << (addr - RAM_BASE) << " bytes)\n\n";
}

int main() {
    // 1. Instanciação do Hardware
    Memory ram; 
    Bus system_bus(ram);
    CPU my_cpu(system_bus);

    // 2. Carregar Programa do Arquivo (Flexibilidade total!)
    // Crie o arquivo "prog.txt" com os hexadecimais
    load_program_from_file(system_bus, "prog.txt");

    std::cout << "--- CPU START ---\n";
    
    // 3. Loop Dinâmico
    // Roda enquanto a CPU estiver com flag running = true
    // O programa deve ter um ECALL (Exit) no final para parar.
    // Adicionei um limite de segurança (1000 ciclos) para não travar seu PC se tiver bug.
    int max_cycles = 1000;
    int cycles = 0;

    // DEFINIÇÃO DO GRUPO (Requisito E/S Programada): 
    // Atualiza a tela a cada N instruções.
    // Para ver a "animação" do Fibonacci acontecendo, 5 é um bom número.
    int refresh_rate = 5; 

    while (my_cpu.is_running() && cycles < max_cycles) {
        my_cpu.step();
        cycles++;

        // --- ATUALIZAÇÃO DA TELA (DUMP REGULAR) ---
        if (cycles % refresh_rate == 0) {
            std::cout << "\n[REFRESH CYCLE " << std::dec << cycles << "]\n";
            Monitor::dump_vram(ram);
            
            // Dica: Se quiser ver acontecendo devagar, descomente a linha abaixo:
            // std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    if (cycles >= max_cycles) {
        std::cout << "\n[AVISO] Simulacao parada por limite de seguranca (loop infinito?)\n";
    }

    std::cout << "--- CPU HALTED ---\n";

    // 4. Mostra o resultado final da VRAM
    Monitor::dump_vram(ram);
    
    // 5. Mostra registradores finais
    my_cpu.dump_regs();

    return 0;
}