#ifndef _CONTROLE_H_
#define _CONTROLE_H_

#include <systemc.h>

SC_MODULE(Controle) {
  // Entrada
  sc_in<sc_uint<6>> opcode; // Código de operação da instrução (6 bits)

  // Saídas de controle
  sc_out<bool> reg_dst, alu_src, mem_to_reg;
  sc_out<bool> reg_write, mem_read, mem_write;
  sc_out<bool> jump, jn, jz;
  sc_out<sc_uint<2>> alu_op; // Código de operação para a ULA (2 bits)

  // Método combinacional para decodificar o opcode e gerar os sinais de controle apropriados
  void decodificar() {
    sc_uint<6> op = opcode.read();

    // Inicializa todas as saídas de controle com valores padrão (0 ou false)
    reg_dst.write(false);
    alu_src.write(false);
    mem_to_reg.write(false);
    reg_write.write(false);
    mem_read.write(false);
    mem_write.write(false);
    jump.write(false);
    jn.write(false);
    jz.write(false);
    alu_op.write(0);

    if (op == 0) {  // Tipo R (ADD, SUB, AND, OR, XOR, NOT, CMP)
      reg_dst.write(true);
      reg_write.write(true);
      alu_op.write(2);    // Operação definida por funct

    } else if (op == 2) { // J (Jump Incondicional)
      jump.write(true);

    } else if (op == 3) { // JN (Jump se Negativo)
      jn.write(true);
      alu_op.write(1);    // CMP

    } else if (op == 4) { // JZ (Jump se Zero)
      jz.write(true);
      alu_op.write(1);    // CMP

    } else if (op == 8) { // LRI
      alu_src.write(true);
      reg_write.write(true);
      alu_op.write(0);    // ADD

    } else if (op == 35) { // LD
      alu_src.write(true);
      mem_to_reg.write(true);
      reg_write.write(true);
      mem_read.write(true);
      alu_op.write(0);    // ADD 

    } else if (op == 43) { // ST
      alu_src.write(true);
      mem_write.write(true);
      alu_op.write(0);    // ADD
    }
  }

  // Construtor do módulo
  SC_CTOR(Controle) {
    SC_METHOD(decodificar);
    sensitive << opcode;
  }
};
#endif // _CONTROLE_H_
