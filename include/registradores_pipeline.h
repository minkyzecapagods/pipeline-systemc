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

SC_MODULE(Reg_ID_EX) {
  sc_in<bool> clk;
  sc_in<sc_uint<32>> pc_plus_4_in; // NOVO: Traz o PC para calcular o salto
  sc_in<sc_int<32>> reg_data1_in, reg_data2_in, imediato32_in;
  sc_in<sc_uint<5>> rs_in, rt_in, rd_in;
  sc_in<bool> ctrl_reg_write_in, ctrl_mem_to_reg_in, ctrl_mem_read_in,
      ctrl_mem_write_in;
  sc_in<bool> ctrl_reg_dst_in, ctrl_alu_src_in;
  sc_in<sc_uint<2>> ctrl_alu_op_in;
  sc_in<bool> ctrl_jump_in, ctrl_jn_in,
      ctrl_jz_in; // NOVO: Sinais de Salto viajam

  sc_out<sc_uint<32>> pc_plus_4_out;
  sc_out<sc_int<32>> reg_data1_out, reg_data2_out, imediato32_out;
  sc_out<sc_uint<5>> rs_out, rt_out, rd_out;
  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out, ctrl_mem_read_out,
      ctrl_mem_write_out;
  sc_out<bool> ctrl_reg_dst_out, ctrl_alu_src_out;
  sc_out<sc_uint<2>> ctrl_alu_op_out;
  sc_out<bool> ctrl_jump_out, ctrl_jn_out, ctrl_jz_out;

  void clock_tick() {
    pc_plus_4_out.write(pc_plus_4_in.read());
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
    ctrl_jump_out.write(ctrl_jump_in.read());
    ctrl_jn_out.write(ctrl_jn_in.read());
    ctrl_jz_out.write(ctrl_jz_in.read());
  }
  SC_CTOR(Reg_ID_EX) {
    SC_METHOD(clock_tick);
    sensitive << clk.pos();
  }
};

SC_MODULE(Reg_EX_MEM) {
  sc_in<bool> clk;
  sc_in<bool> ctrl_reg_write_in, ctrl_mem_to_reg_in, ctrl_mem_read_in,
      ctrl_mem_write_in;
  sc_in<sc_int<32>> ula_resultado_in, reg_data2_in;
  sc_in<sc_uint<5>> reg_dst_in;
  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out, ctrl_mem_read_out,
      ctrl_mem_write_out;
  sc_out<sc_int<32>> ula_resultado_out, reg_data2_out;
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

SC_MODULE(Reg_MEM_WB) {
  sc_in<bool> clk;
  sc_in<bool> ctrl_reg_write_in, ctrl_mem_to_reg_in;
  sc_in<sc_int<32>> mem_dado_lido_in, ula_resultado_in;
  sc_in<sc_uint<5>> reg_dst_in;
  sc_out<bool> ctrl_reg_write_out, ctrl_mem_to_reg_out;
  sc_out<sc_int<32>> mem_dado_lido_out, ula_resultado_out;
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
