#ifndef _MEM_INST_H_
#define _MEM_INST_H_

#include <systemc.h>
#include <vector>

SC_MODULE(MemoriaInstrucoes) {
  // Entradas e saídas
  sc_in<sc_uint<32>>  endereco;  // Endereço de instrução (PC)
  sc_out<sc_uint<32>> instrucao; // Instrução lida

  // Memória de instruções (1024 palavras de 32 bits)
  sc_uint<32> memoria[1024];

  // Método para ler a instrução da memória com base no endereço
  void ler_instrucao() {
    // Converte o endereço de byte para índice de palavra
    sc_uint<32> indice = endereco.read() / 4;

    if (indice < 1024)
      instrucao.write(memoria[indice]);
    else
      instrucao.write(0);
  }

  // Método usado apenas por testbench para inicializar as instruções na memória
  void carregar_programa(const std::vector<sc_uint<32>> &programa) {
    for (size_t i = 0; i < programa.size() && i < 1024; i++) {
      memoria[i] = programa[i];
    }
  }

  // Construtor do módulo
  SC_CTOR(MemoriaInstrucoes) {

    // Leitura combinacional sensível ao endereço
    SC_METHOD(ler_instrucao);
    sensitive << endereco;

    // Inicializa a memória com zeros
    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
#endif // _MEM_INST_H_
