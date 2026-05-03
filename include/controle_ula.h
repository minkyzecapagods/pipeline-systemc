#ifndef _CONTROLE_ULA_H_
#define _CONTROLE_ULA_H_
#include <systemc.h>

SC_MODULE(ControleULA) {
  sc_in<sc_uint<2>> alu_op;
  sc_in<sc_int<32>> imediato32; // Recebe o cabo inteiro
  sc_out<sc_uint<4>> operacao_ula;

  void decodificar() {
    sc_uint<2> op = alu_op.read();
    // Fatiamento dinamico a cada batida do relogio!
    sc_uint<6> f = imediato32.read().range(5, 0).to_uint();
    sc_uint<4> res = 0;

    if (op == 0) {
      res = 2; // LD/ST/LRI forcam a ULA a Somar (ADD)
    } else if (op == 1) {
      res = 6; // Branchs forcam Subtracao (SUB)
    } else if (op == 2) {
      switch (f) {
      case 32:
        res = 2;
        break; // ADD
      case 34:
        res = 6;
        break; // SUB
      case 36:
        res = 0;
        break; // AND
      case 37:
        res = 1;
        break; // OR
      default:
        res = 0;
        break;
      }
    }
    operacao_ula.write(res);
  }

  SC_CTOR(ControleULA) {
    SC_METHOD(decodificar);
    sensitive << alu_op << imediato32; // Reage imediatamente!
  }
};
#endif
