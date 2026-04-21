#include <systemc.h>

SC_MODULE(MemoriaInstrucoes) {
  sc_in<sc_uint<32>> endereco;
  sc_out<sc_uint<32>> instrucao;

  sc_uint<32> memoria[1024]; // 4KB de memória (1024 palavras de 32 bits)

  void ler_instrucao() {
    // O endereço no MIPS é alinhado a byte, dividimos por 4 para acessar o
    // array
    sc_uint<32> indice = endereco.read() / 4;
    if (indice < 1024) {
      instrucao.write(memoria[indice]);
    } else {
      instrucao.write(0);
    }
  }

  SC_CTOR(MemoriaInstrucoes) {
    SC_METHOD(ler_instrucao);
    sensitive << endereco;

    // Inicialize a memória com 0 ou carregue seu algoritmo aqui
    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
