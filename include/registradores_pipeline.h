#ifndef _REG_PIPELINE_H_
#define _REG_PIPELINE_H_
#include <systemc.h>

SC_MODULE(Reg_IF_ID) {
  sc_in<bool> clk;
  sc_in<sc_uint<32>> pc_plus_4_in;
  sc_in<sc_uint<32>> inst_in;

  sc_out<sc_uint<32>> pc_plus_4_out;
  sc_out<sc_uint<32>> inst_out;

  void clock_tick() {
    pc_plus_4_out.write(pc_plus_4_in.read());
    inst_out.write(inst_in.read());
  }

  SC_CTOR(Reg_IF_ID) {
    SC_METHOD(clock_tick);
    sensitive << clk.pos();
  }
};
#endif
