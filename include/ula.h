#include <systemc.h>

SC_MODULE(ULA) {
  sc_in<sc_int<32>> op_a;
  sc_in<sc_int<32>> op_b;
  sc_in<sc_uint<4>> ula_ctrl;

  sc_out<sc_int<32>> resultado;
  sc_out<bool> flag_zero;

  void calcular() {
    sc_int<32> a = op_a.read();
    sc_int<32> b = op_b.read();
    sc_int<32> res = 0;

    switch (ula_ctrl.read()) {
    case 0:
      res = a & b;
      break; // AND
    case 1:
      res = a | b;
      break; // OR
    case 2:
      res = a + b;
      break; // ADD
    case 6:
      res = a - b;
      break; // SUB ou CMP
    // Implemente XOR (ex: case 3) e NOT (ex: case 4) conforme sua Unidade de
    // Controle
    default:
      res = 0;
      break;
    }

    resultado.write(res);
    flag_zero.write(res == 0);
  }

  SC_CTOR(ULA) {
    SC_METHOD(calcular);
    sensitive << op_a << op_b << ula_ctrl;
  }
};
