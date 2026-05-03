#ifndef _MEM_INST_H_
#define _MEM_INST_H_
#include <systemc.h>
#include <vector>

SC_MODULE(MemoriaInstrucoes) {
  sc_in<sc_uint<32>> endereco;
  sc_out<sc_uint<32>> instrucao;

  sc_uint<32> memoria[1024];

  void ler_instrucao() {
    sc_uint<32> indice = endereco.read() / 4;
    if (indice < 1024)
      instrucao.write(memoria[indice]);
    else
      instrucao.write(0);
  }

  // Usado apenas pelo Testbench para inicializar o chip[cite: 12]
  void carregar_programa(const std::vector<sc_uint<32>> &programa) {
    for (size_t i = 0; i < programa.size() && i < 1024; i++) {
      memoria[i] = programa[i];
    }
  }

  SC_CTOR(MemoriaInstrucoes) {
    SC_METHOD(ler_instrucao);
    sensitive << endereco;
    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
#endif
