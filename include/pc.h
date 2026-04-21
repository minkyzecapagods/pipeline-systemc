#include <systemc.h>

SC_MODULE(PC) {
  sc_in<bool> clk;
  sc_in<sc_uint<32>> pc_in;
  sc_out<sc_uint<32>> pc_out;

  void atualizar_pc() { pc_out.write(pc_in.read()); }

  SC_CTOR(PC) {
    SC_METHOD(atualizar_pc);
    sensitive << clk.pos();
  }
};
