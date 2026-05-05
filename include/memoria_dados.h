#ifndef _MEM_DADOS_H_
#define _MEM_DADOS_H_

#include <systemc.h>

SC_MODULE(MemoriaDados) {
  // Entradas
  sc_in<bool>       clk;
  sc_in<sc_int<32>> endereco;
  sc_in<sc_int<32>> write_data;
  sc_in<bool>       mem_write;
  sc_in<bool>       mem_read;

  // Saída
  sc_out<sc_int<32>> read_data;

  // Memória de dados (1024 palavras de 32 bits)
  sc_int<32> memoria[1024];

  // Método combinacional para leitura da memória
  void ler() {
    if (mem_read.read() == true) {
      sc_uint<32> indice = endereco.read() / 4;
      if (indice < 1024)
        read_data.write(memoria[indice]);
    } else {
      read_data.write(0);
    }
  }

  // Método síncrono para escrita na memória
  void escrever() {
    if (mem_write.read() == true) {
      sc_uint<32> indice = endereco.read() / 4;
      if (indice < 1024)
        memoria[indice] = write_data.read();
    }
  }

  // Construtor do módulo
  SC_CTOR(MemoriaDados) {
    // Leitura combinacional sensível ao endereço e ao sinal de leitura
    SC_METHOD(ler);
    sensitive << endereco << mem_read;

    // Escrita síncrona sensível à borda de subida do clock
    SC_METHOD(escrever);
    sensitive << clk.pos();

    // Inicializa a memória com zeros
    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
#endif // _MEM_DADOS_H_
