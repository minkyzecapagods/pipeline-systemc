#include <systemc.h>

// Registrador entre Busca (IF) e Decodificação (ID)
SC_MODULE(Reg_IF_ID) {
  sc_in<bool> clk;
  sc_in<sc_uint<32>> pc_plus_4_in;
  sc_in<sc_uint<32>> inst_in;

  sc_out<sc_uint<32>> pc_plus_4_out;
  sc_out<sc_uint<32>> inst_out;

  void process() {
    pc_plus_4_out.write(pc_plus_4_in.read());
    inst_out.write(inst_in.read());
  }

  SC_CTOR(Reg_IF_ID) {
    SC_METHOD(process);
    sensitive << clk.pos();
  }
};

// Nota: Você precisará criar blocos semelhantes para ID_EX, EX_MEM e MEM_WB
// seguindo a mesma lógica de "entrada recebe, saída entrega no próximo clock".
