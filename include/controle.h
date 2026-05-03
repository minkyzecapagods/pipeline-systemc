#ifndef _CONTROLE_H_
#define _CONTROLE_H_
#include <systemc.h>

SC_MODULE(Controle) {
  sc_in<sc_uint<6>> opcode;

  sc_out<bool> reg_dst;
  sc_out<bool> alu_src;
  sc_out<bool> mem_to_reg;
  sc_out<bool> reg_write;
  sc_out<bool> mem_read;
  sc_out<bool> mem_write;
  sc_out<bool> branch;
  sc_out<sc_uint<2>> alu_op;

  void decodificar() {
    sc_uint<6> op = opcode.read();

    reg_dst.write(false);
    alu_src.write(false);
    mem_to_reg.write(false);
    reg_write.write(false);
    mem_read.write(false);
    mem_write.write(false);
    branch.write(false);
    alu_op.write(0);

    if (op == 0) { // Tipo R
      reg_dst.write(true);
      reg_write.write(true);
      alu_op.write(2);
    } else if (op == 35) { // LD[cite: 11]
      alu_src.write(true);
      mem_to_reg.write(true);
      reg_write.write(true);
      mem_read.write(true);
      alu_op.write(0);
    } else if (op == 43) { // ST
      alu_src.write(true);
      mem_write.write(true);
      alu_op.write(0);
    }
  }

  SC_CTOR(Controle) {
    SC_METHOD(decodificar);
    sensitive << opcode;
  }
};
#endif
