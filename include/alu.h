/**
 * @file   alu.h
 * @brief Unidade Lógica e Aritmética combinacional de 16 bits.
 * 
 * Executa uma operação definida por `operation_code` sobre `operand_a` e `operand_b`.
 * As saídas são atualizadas imediatamente quando qualquer entrada muda.
 */

#ifndef ALU_H
#define ALU_H

#include <systemc.h>

namespace {
    /**
     * @brief Códigos de operação suportados pela ALU.
     * @note  A Unidade de Controle é responsável por decidir se o resultado
     *        será efetivamente escrito em registrador (sinal reg_write).
     */
    enum OperationCode {
        OP_AND = 0,
        OP_OR  = 1,
        OP_XOR = 2,
        OP_NOT = 3,
        OP_CMP = 4,   // Subtração usada para comparação (flags apenas)
        OP_ADD = 5,
        OP_SUB = 6    // Subtração que normalmente terá reg_write ativo
    };
}

/**
 * @class ALU
 * @brief Unidade Lógica e Aritmética (ALU) de 16 bits.
 *
 * Realiza operações aritméticas e lógicas entre dois operandos de 16 bits,
 * controladas por um código de operação de 4 bits. As saídas incluem o
 * resultado da operação e duas flags: negative (n) e zero (z).
 *
 * A lógica é puramente combinacional: qualquer alteração nas entradas
 * a, b ou op dispara a atualização das saídas.
 */
SC_MODULE(ALU) {
public:
    sc_in<sc_uint<16>> operand_a;
    sc_in<sc_uint<16>> operand_b;
    sc_in<sc_uint<4>>  operation_code;
    sc_out<sc_uint<16>> result;
    sc_out<bool>        negative_flag;
    sc_out<bool>        zero_flag;

    void execute_operation() {
        const sc_uint<16> a_val = operand_a.read();
        const sc_uint<16> b_val = operand_b.read();
        const sc_uint<4>  op_val = operation_code.read();
        const unsigned int op_int = op_val.to_uint();

        sc_uint<16> result_val = 0;

        switch (op_int) {
            case OP_AND: result_val = a_val & b_val;  break;
            case OP_OR:  result_val = a_val | b_val;  break;
            case OP_XOR: result_val = a_val ^ b_val;  break;
            case OP_NOT: result_val = ~a_val;         break;
            case OP_CMP:                               // CMP e SUB têm a mesma lógica combinacional
            case OP_SUB: result_val = a_val - b_val;  break;
            case OP_ADD: result_val = a_val + b_val;  break;
            default:     result_val = 0;              break; // Operação inválida
        }

        // Atualiza as saídas da ALU (resultado e flags)
        result.write(result_val);
        negative_flag.write(result_val[15] == 1);
        zero_flag.write(result_val == 0);

        // Nota: A Unidade de Controle usará os sinais result, n e z conforme
        // o estágio de pipeline (ex.: em CMP, reg_write = 0 no estágio WRITEBACK).
    }

    SC_CTOR(ALU) {
        SC_METHOD(execute_operation);
        sensitive << operand_a << operand_b << operation_code;
    }
};

#endif // ALU_H