#ifndef _CONTROLE_ULA_H_
#define _CONTROLE_ULA_H_

#include <systemc.h>

SC_MODULE(ControleULA) {
  // Entradas
  sc_in<sc_uint<2>> alu_op;     // Código de operação da ULA vindo da unidade de controle principal
  sc_in<sc_int<32>> imediato32; // Imediato de 32 bits, usado para extrair o campo funct em instruções tipo R

  // Saída
  sc_out<sc_uint<4>> operacao_ula; // Operação enviada para a ULA

  void decodificar() {
    sc_uint<2> op = alu_op.read();
    sc_uint<6> f = imediato32.read().range(5, 0).to_uint();
    sc_uint<4> res = 0;

    if (op == 0) { // Operação de memória ou LRI (load immediate) 
      res = 2;     // ADD
    } else if (op == 1) { // Branchs/Saltos condicionais
      res = 7;            // CMP
    } else if (op == 2) { // Instrução tipo R, operação definida por funct
      switch (f) {
      case 32: res = 2; break; // ADD
      case 34: res = 6; break; // SUB
      case 36: res = 0; break; // AND
      case 37: res = 1; break; // OR
      case 38: res = 3; break; // XOR
      case 39: res = 4; break; // NOT
      case 40: res = 7; break; // CMP
      default: res = 0; break;
      }
    }

    operacao_ula.write(res);
  }

  // Construtor do módulo
  SC_CTOR(ControleULA) {

    // Decodificação combacional sensível a operação e ao immediate
    SC_METHOD(decodificar);
    sensitive << alu_op << imediato32;
  }
};

#endif // _CONTROL_ULA_H_
