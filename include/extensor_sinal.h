#include <systemc.h>

SC_MODULE(ExtensorSinal) {
  sc_in<sc_uint<16>> entrada;
  sc_out<sc_int<32>> saida;

  void estender() {
    sc_int<16> valor_com_sinal = entrada.read().to_int();
    saida.write((sc_int<32>)valor_com_sinal);
  }

  SC_CTOR(ExtensorSinal) {
    SC_METHOD(estender);
    sensitive << entrada;
  }
};
