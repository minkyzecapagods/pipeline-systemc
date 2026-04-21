#include <systemc.h>

SC_MODULE(Mux2) {
  sc_in<sc_int<32>> entrada0;
  sc_in<sc_int<32>> entrada1;
  sc_in<bool> selecao;
  sc_out<sc_int<32>> saida;

  void rotear() {
    if (selecao.read() == false) {
      saida.write(entrada0.read());
    } else {
      saida.write(entrada1.read());
    }
  }

  SC_CTOR(Mux2) {
    SC_METHOD(rotear);
    sensitive << entrada0 << entrada1 << selecao;
  }
};
