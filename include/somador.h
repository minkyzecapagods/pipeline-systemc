#ifndef _SOMADOR_H_
#define _SOMADOR_H_
#include <systemc.h>

SC_MODULE(Somador) {
  sc_in<sc_uint<32>> op_a;
  sc_in<sc_uint<32>> op_b;
  sc_out<sc_uint<32>> resultado;
  void somar() { resultado.write(op_a.read() + op_b.read()); }
  SC_CTOR(Somador) {
    SC_METHOD(somar);
    sensitive << op_a << op_b;
  }
};

// --- NOVO: Somador Exclusivo para Cálculo de Saltos (Branch) ---
SC_MODULE(SomadorBranch) {
  sc_in<sc_uint<32>> op_a; // Entrada do PC+4 (Sem sinal)
  sc_in<sc_int<32>> op_b;  // Entrada do Imediato (Com sinal!)
  sc_out<sc_uint<32>> resultado;

  void somar() {
    // O MIPS multiplica o offset por 4 para alinhar com a memória!
    sc_int<32> deslocamento = op_b.read() * 4;
    resultado.write(op_a.read() + deslocamento);
  }

  SC_CTOR(SomadorBranch) {
    SC_METHOD(somar);
    sensitive << op_a << op_b;
  }
};
#endif
