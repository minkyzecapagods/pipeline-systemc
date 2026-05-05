#ifndef _MUX2_H_
#define _MUX2_H_

#include <systemc.h>

// Muxes com métodos combinacionais para rotear os sinais de acordo com a seleção

// Módulo para multiplexar entre dois sinais de 32 bits (usado para ALU Src)
SC_MODULE(Mux2_Int) {
  // Entradas
  sc_in<sc_int<32>> entrada0;
  sc_in<sc_int<32>> entrada1;
  sc_in<bool>       selecao;

  // Saída
  sc_out<sc_int<32>> saida;

  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }

  // Construtor do módulo
  SC_CTOR(Mux2_Int) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};

// Módulo para multiplexar entre dois sinais de 5 bits (usado para RegDst)
SC_MODULE(Mux2_Uint) {
  // Entradas
  sc_in<sc_uint<5>> entrada0;
  sc_in<sc_uint<5>> entrada1;
  sc_in<bool>       selecao;

  // Saída
  sc_out<sc_uint<5>> saida;

  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }

  // Construtor do módulo
  SC_CTOR(Mux2_Uint) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};

// Módulo para multiplexar entre dois sinais de 32 bits (usado para o PC)
SC_MODULE(Mux2_Uint32) {
  // Entradas
  sc_in<sc_uint<32>> entrada0;
  sc_in<sc_uint<32>> entrada1;
  sc_in<bool>        selecao;

  // Saída
  sc_out<sc_uint<32>> saida;

  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }

  // Construtor do módulo
  SC_CTOR(Mux2_Uint32) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};
#endif // _MUX2_H_