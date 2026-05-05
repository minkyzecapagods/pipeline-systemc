#ifndef _SOMADOR_H_
#define _SOMADOR_H_

#include <systemc.h>

SC_MODULE(Somador) {
  // Entradas
  sc_in<sc_uint<32>> op_a;
  sc_in<sc_uint<32>> op_b;

  // Saída
  sc_out<sc_uint<32>> resultado;

  // Método combinacional para somar os dois operandos
  void somar() { resultado.write(op_a.read() + op_b.read()); }

  // Construtor do módulo
  SC_CTOR(Somador) {
    SC_METHOD(somar);
    sensitive << op_a << op_b;
  }
};

// Somador cálculo de saltos (Branch)
SC_MODULE(SomadorBranch) {
  // Entradas
  sc_in<sc_uint<32>> op_a; // Entrada do PC+4 (sem sinal)
  sc_in<sc_int<32>> op_b;  // Entrada do Imediato (com sinal)

  // Saída
  sc_out<sc_uint<32>> resultado;

  // Método combinacional para somar o PC+4 com o imediato estendido, multiplicado por 4 
  void somar() {
    sc_int<32> deslocamento = op_b.read() * 4;
    resultado.write(op_a.read() + deslocamento);
  }

  // Construtor do módulo
  SC_CTOR(SomadorBranch) {
    SC_METHOD(somar);
    sensitive << op_a << op_b;
  }
};
#endif // _SOMADOR_H_
