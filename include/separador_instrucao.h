#ifndef _SEPARADOR_INST_H_
#define _SEPARADOR_INST_H_
#include <systemc.h>

SC_MODULE(SeparadorInstrucao) {
  // Entrada: Instrução completa de 32 bits
  sc_in<sc_uint<32>> instrucao32;

  // Saídas: Fatias da instrução
  sc_out<sc_uint<6>> opcode;
  sc_out<sc_uint<5>> rs;
  sc_out<sc_uint<5>> rt;
  sc_out<sc_uint<5>> rd;
  sc_out<sc_uint<16>> imediato;

  void fatiar() {
    sc_uint<32> inst = instrucao32.read();

    // .range(bit_mais_significativo, bit_menos_significativo)
    opcode.write(inst.range(31, 26));
    rs.write(inst.range(25, 21));
    rt.write(inst.range(20, 16));
    rd.write(inst.range(15, 11));
    imediato.write(inst.range(15, 0));
  }

  SC_CTOR(SeparadorInstrucao) {
    SC_METHOD(fatiar);
    sensitive << instrucao32; // Puramente combinacional, reage instantaneamente
  }
};
#endif
