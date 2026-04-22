/**
 * @file   pc.h
 * @brief  Contador de programa (PC) síncrono de 8 bits com reset e habilitação de carga.
 */

#ifndef PC_H
#define PC_H

#include <systemc.h>

/**
 * @class ProgramCounter
 * @brief Registrador de 8 bits para o contador de programa.
 *
 * Atualiza sua saída na borda de subida do clock:
 * - Se `reset` estiver ativo, `pc` é zerado.
 * - Senão, se `pc_write` estiver ativo, carrega `next_pc`.
 * - Caso contrário, mantém o valor atual.
 *
 * @note O reset tem prioridade sobre a escrita.
 */
SC_MODULE(ProgramCounter) {
public:
    sc_in<bool>        clk;
    sc_in<bool>        reset;
    sc_in<bool>        pc_write;
    sc_in<sc_uint<8>>  next_pc;
    sc_out<sc_uint<8>> pc;

    void update_process() {
        if (reset.read()) {
            pc.write(0);
        } else if (pc_write.read()) {
            pc.write(next_pc.read());
        }
        // else: mantém valor (implícito)
    }

    SC_CTOR(ProgramCounter) {
        SC_METHOD(update_process);
        sensitive << clk.pos();
    }
};

#endif // PC_H