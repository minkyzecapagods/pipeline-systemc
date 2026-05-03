#ifndef _PC_H_
#define _PC_H_
#include <systemc.h>

SC_MODULE(PC) {
  sc_in<bool> clk;
  sc_in<bool> reset;
  sc_in<sc_uint<32>> pc_in;
  sc_out<sc_uint<32>> pc_out;

  void atualizar_pc() {
    if (reset.read() == true) {
      pc_out.write(0);
    } else {
      pc_out.write(pc_in.read());
    }
  }

  SC_CTOR(PC) {
    SC_METHOD(atualizar_pc);
    sensitive << clk.pos() << reset;
  }
};
#endif
