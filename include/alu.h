#ifndef ALU_H
#define ALU_H

#include <systemc.h>

/**
 * @brief Unidade Lógica e Aritmética combinacional de 16 bits.
 * 
 * Executa uma operação definida por `operation_code` sobre `operand_a` e `operand_b`.
 * As saídas são atualizadas imediatamente quando qualquer entrada muda.
 */
SC_MODULE(ALU) {
public:
    sc_in<sc_uint<16>> operand_a;
    sc_in<sc_uint<16>> operand_b;
    sc_in<sc_uint<4>>  operation_code;
    sc_out<sc_uint<16>> result;
    sc_out<bool>        negative_flag;
    sc_out<bool>        zero_flag;

    void execute_operation();

    SC_CTOR(ALU) {
        // Registrar o método de execução da operação, sensível a mudanças nas entradas (sem clock)
        SC_METHOD(execute_operation);
        sensitive << operand_a << operand_b << operation_code;
    }
};

#endif // ALU_H