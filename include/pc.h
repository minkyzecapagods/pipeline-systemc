#ifndef _PC_H_
#define _PC_H_

#include <systemc.h>

SC_MODULE(PC) {
  // Entradas
  sc_in<bool>        clk;
  sc_in<bool>        reset;
  sc_in<sc_uint<32>> pc_in;

  // Saída
  sc_out<sc_uint<32>> pc_out;

  // Método combinacional para atualizar o PC
  void atualizar_pc() {
    if (reset.read() == true) {
      pc_out.write(0);
    } else {
      pc_out.write(pc_in.read());
    }
  }

  // Módulo de construção
  SC_CTOR(PC) {
    // Atualiza o PC na borda de subida do clock ou quando reset é ativado
    SC_METHOD(atualizar_pc);
    sensitive << clk.pos() << reset;
  }
};
#endif // _PC_H_
