/**
 * @file   imem.h
 * @brief  Memória de instruções (ROM) de 16 bits com 256 posições.
 */

#ifndef IMEM_H
#define IMEM_H

#include <systemc.h>

/**
 * @class IMem
 * @brief Memória de instruções somente leitura (combinacional).
 *
 * Fornece uma instrução de 16 bits a partir do endereço de 8 bits.
 * O conteúdo da memória deve ser carregado externamente através do array `memory`
 * antes do início da simulação.
 */
SC_MODULE(IMem) {
public:
    sc_in<sc_uint<8>>  address;
    sc_out<sc_uint<16>> instruction;

    sc_uint<16> memory[256];

    void read_process() {
        instruction.write(memory[address.read()]);
    }

    SC_CTOR(IMem) {
        // Inicializa toda a memória com zero (pode ser sobrescrito pelo testbench)
        for (int i = 0; i < 256; ++i) {
            memory[i] = 0;
        }

        SC_METHOD(read_process);
        sensitive << address;
    }
};

#endif // IMEM_H