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

    // Inicializa tudo em 0
    reg_dst.write(false);
    alu_src.write(false);
    mem_to_reg.write(false);
    reg_write.write(false);
    mem_read.write(false);
    mem_write.write(false);
    branch.write(false);
    alu_op.write(0);

    // Exemplo: Opcode 0 = Tipo R (ADD, SUB, AND, OR)
    if (op == 0) {
      reg_dst.write(true);
      reg_write.write(true);
      alu_op.write(2); // Sinaliza para a ULA Control checar o campo Funct
    }
    // Exemplo: Opcode 35 (lw / LD)
    else if (op == 35) {
      alu_src.write(true);
      mem_to_reg.write(true);
      reg_write.write(true);
      mem_read.write(true);
      alu_op.write(0); // Sinaliza para a ULA somar endereço base + deslocamento
    }
    // Adicione os outros Opcodes do seu ISA aqui (SW, BEQ, J, etc)
  }

  SC_CTOR(Controle) {
    SC_METHOD(decodificar);
    sensitive << opcode;
  }
};
