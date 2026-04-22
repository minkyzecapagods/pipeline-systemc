/**
 * @file   dmem.h
 * @brief  Memória de dados de 16 bits com 256 posições.
 *         Leitura combinacional, escrita síncrona na borda de subida do clock.
 */

#ifndef DMEM_H
#define DMEM_H

#include <systemc.h>

/**
 * @class DMem
 * @brief Memória de dados (256 x 16 bits) com uma porta de leitura e uma de escrita.
 *
 * A leitura é combinacional e reflete imediatamente qualquer alteração na memória.
 * A escrita é síncrona e ocorre na borda positiva do clock quando `write_enable` está ativo.
 *
 * @note O conteúdo da memória pode ser inicializado externamente através do vetor `memory`.
 */
SC_MODULE(DMem) {
public:
    static constexpr int MEM_SIZE    = 256;
    static constexpr int DATA_WIDTH  = 16;
    static constexpr int ADDR_WIDTH  = 8;

    sc_in<sc_uint<ADDR_WIDTH>> address;
    sc_in<sc_uint<DATA_WIDTH>> write_data;
    sc_in<bool>                write_enable;
    sc_in<bool>                clock;
    sc_out<sc_uint<DATA_WIDTH>> read_data;

    // Cada célula da memória é um sinal para permitir sensibilidade adequada na leitura
    sc_vector<sc_signal<sc_uint<DATA_WIDTH>>> memory;

    void read_process() {
        read_data.write(memory[address.read()].read());
    }

    void write_process() {
        if (write_enable.read()) {
            memory[address.read()].write(write_data.read());
        }
    }

    SC_CTOR(DMem)
        : memory("mem_cell", MEM_SIZE)
    {
        // Inicializa toda a memória com zero
        for (auto& cell : memory) {
            cell.write(0);
        }

        SC_METHOD(read_process);
        sensitive << address;
        for (auto& cell : memory) {
            sensitive << cell;   // leitura reage a mudanças em qualquer posição
        }

        SC_METHOD(write_process);
        sensitive << clock.pos();
    }
};

#endif // DMEM_H