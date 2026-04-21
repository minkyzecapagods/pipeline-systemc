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
