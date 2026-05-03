#ifndef _REG_PIPELINE_H_
#define _REG_PIPELINE_H_
#include <systemc.h>

// --- Barreira IF / ID (Já existia) ---
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

// --- NOVA Barreira ID / EX ---
SC_MODULE(Reg_ID_EX) {
  sc_in<bool> clk;

  // Dados que passam de fase
  sc_in<sc_int<32>> reg_data1_in, reg_data2_in;
  sc_in<sc_int<32>> imediato32_in;
  sc_in<sc_uint<5>> rs_in, rt_in, rd_in;

  // Sinais de Controle que viajam com a instrução
  sc_in<bool> ctrl_reg_write_in, ctrl_mem_to_reg_in; // Vão até o final (WB)
  sc_in<bool> ctrl_mem_read_in, ctrl_mem_write_in;   // Vão até a Memória (MEM)
  sc_in<bool> ctrl_reg_dst_in, ctrl_alu_src_in;      // Ficam na Execução (EX)
  sc_in<sc_uint<2>> ctrl_alu_op_in;

  // Saídas
  sc_out<sc_int<32>> reg_data1_out, reg_data2_out;
  sc_out<sc_int<32>> imediato32_out;
  sc_out<sc_uint<5>> rs_out, rt_out, rd_out;

  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out;
  sc_out<bool> ctrl_mem_read_out, ctrl_mem_write_out;
  sc_out<bool> ctrl_reg_dst_out, ctrl_alu_src_out;
  sc_out<sc_uint<2>> ctrl_alu_op_out;

  void clock_tick() {
    // Copia tudo da entrada para a saída na borda do clock
    reg_data1_out.write(reg_data1_in.read());
    reg_data2_out.write(reg_data2_in.read());
    imediato32_out.write(imediato32_in.read());
    rs_out.write(rs_in.read());
    rt_out.write(rt_in.read());
    rd_out.write(rd_in.read());

    ctrl_reg_write_out.write(ctrl_reg_write_in.read());
    ctrl_mem_to_reg_out.write(ctrl_mem_to_reg_in.read());
    ctrl_mem_read_out.write(ctrl_mem_read_in.read());
    ctrl_mem_write_out.write(ctrl_mem_write_in.read());
    ctrl_reg_dst_out.write(ctrl_reg_dst_in.read());
    ctrl_alu_src_out.write(ctrl_alu_src_in.read());
    ctrl_alu_op_out.write(ctrl_alu_op_in.read());
  }

  SC_CTOR(Reg_ID_EX) {
    SC_METHOD(clock_tick);
    sensitive << clk.pos();
  }
};

// --- NOVA Barreira EX / MEM ---
SC_MODULE(Reg_EX_MEM) {
  sc_in<bool> clk;

  // Sinais de Controle que ainda importam
  sc_in<bool> ctrl_reg_write_in,
      ctrl_mem_to_reg_in; // Vão para o último estágio (WB)
  sc_in<bool> ctrl_mem_read_in,
      ctrl_mem_write_in; // Usados agora neste estágio (MEM)

  // Dados
  sc_in<sc_int<32>> ula_resultado_in;
  sc_in<sc_int<32>> reg_data2_in; // O dado que será gravado na memória (no caso
                                  // do comando ST)
  sc_in<sc_uint<5>>
      reg_dst_in; // O registrador onde salvaremos o resultado final

  // Saídas
  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out;
  sc_out<bool> ctrl_mem_read_out, ctrl_mem_write_out;
  sc_out<sc_int<32>> ula_resultado_out;
  sc_out<sc_int<32>> reg_data2_out;
  sc_out<sc_uint<5>> reg_dst_out;

  void clock_tick() {
    ctrl_reg_write_out.write(ctrl_reg_write_in.read());
    ctrl_mem_to_reg_out.write(ctrl_mem_to_reg_in.read());
    ctrl_mem_read_out.write(ctrl_mem_read_in.read());
    ctrl_mem_write_out.write(ctrl_mem_write_in.read());

    ula_resultado_out.write(ula_resultado_in.read());
    reg_data2_out.write(reg_data2_in.read());
    reg_dst_out.write(reg_dst_in.read());
  }

  SC_CTOR(Reg_EX_MEM) {
    SC_METHOD(clock_tick);
    sensitive << clk.pos();
  }
};

// --- NOVA Barreira MEM / WB ---
SC_MODULE(Reg_MEM_WB) {
  sc_in<bool> clk;

  // Sinais de Controle que vão para o Write-Back
  sc_in<bool> ctrl_reg_write_in, ctrl_mem_to_reg_in;

  // Dados para salvar
  sc_in<sc_int<32>> mem_dado_lido_in; // Dado que veio da RAM de Dados (LD)
  sc_in<sc_int<32>> ula_resultado_in; // Dado que veio da ULA (ADD, SUB...)
  sc_in<sc_uint<5>> reg_dst_in;       // Em qual registrador vamos salvar?

  // Saídas
  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out;
  sc_out<sc_int<32>> mem_dado_lido_out;
  sc_out<sc_int<32>> ula_resultado_out;
  sc_out<sc_uint<5>> reg_dst_out;

  void clock_tick() {
    ctrl_reg_write_out.write(ctrl_reg_write_in.read());
    ctrl_mem_to_reg_out.write(ctrl_mem_to_reg_in.read());

    mem_dado_lido_out.write(mem_dado_lido_in.read());
    ula_resultado_out.write(ula_resultado_in.read());
    reg_dst_out.write(reg_dst_in.read());
  }

  SC_CTOR(Reg_MEM_WB) {
    SC_METHOD(clock_tick);
    sensitive << clk.pos();
  }
};

#endif
