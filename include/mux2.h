#ifndef _MUX2_H_
#define _MUX2_H_
#include <systemc.h>

SC_MODULE(Mux2_Int) {
  sc_in<sc_int<32>> entrada0;
  sc_in<sc_int<32>> entrada1;
  sc_in<bool> selecao;
  sc_out<sc_int<32>> saida;
  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }
  SC_CTOR(Mux2_Int) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};

SC_MODULE(Mux2_Uint) {
  sc_in<sc_uint<5>> entrada0;
  sc_in<sc_uint<5>> entrada1;
  sc_in<bool> selecao;
  sc_out<sc_uint<5>> saida;
  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }
  SC_CTOR(Mux2_Uint) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};

// NOVO: Mux para o Program Counter (PC)
SC_MODULE(Mux2_Uint32) {
  sc_in<sc_uint<32>> entrada0;
  sc_in<sc_uint<32>> entrada1;
  sc_in<bool> selecao;
  sc_out<sc_uint<32>> saida;
  void rotear() {
    if (selecao.read() == false)
      saida.write(entrada0.read());
    else
      saida.write(entrada1.read());
  }
  SC_CTOR(Mux2_Uint32) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};
#endif
