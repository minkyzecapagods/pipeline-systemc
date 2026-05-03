#ifndef _ULA_H_
#define _ULA_H_
#include <systemc.h>

SC_MODULE(ULA) {
  sc_in<sc_int<32>> op_a;
  sc_in<sc_int<32>> op_b;
  sc_in<sc_uint<4>> ula_ctrl;
  sc_out<sc_int<32>> resultado;
  sc_out<bool> flag_zero;
  sc_out<bool> flag_negativo; // Necessário para o JN

  void calcular() {
    sc_int<32> a = op_a.read();
    sc_int<32> b = op_b.read();
    sc_int<32> res = 0;

    switch (ula_ctrl.read().to_uint()) {
    case 0:
      res = a & b;
      break; // AND
    case 1:
      res = a | b;
      break; // OR
    case 2:
      res = a + b;
      break; // ADD
    case 3:
      res = a ^ b;
      break; // XOR
    case 4:
      res = ~a;
      break; // NOT (Inverte os bits de A)
    case 6:
      res = a - b;
      break; // SUB
    case 7:
      res = a - b;
      break; // CMP (Faz subtração para gerar flags)
    default:
      res = 0;
      break;
    }
    resultado.write(res);
    flag_zero.write(res == 0);
    flag_negativo.write(res < 0);
  }

  SC_CTOR(ULA) {
    SC_METHOD(calcular);
    sensitive << op_a << op_b << ula_ctrl;
  }
};
#endif
