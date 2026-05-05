#ifndef _MIPS_TOP_H_
#define _MIPS_TOP_H_

#include "banco_registradores.h"
#include "controle.h"
#include "controle_ula.h"
#include "extensor_sinal.h"
#include "memoria_dados.h"
#include "memoria_instrucoes.h"
#include "mux2.h"
#include "pc.h"
#include "registradores_pipeline.h"
#include "separador_instrucao.h"
#include "somador.h"
#include "ula.h"
#include "unidade_saltos.h"
#include <systemc.h>

SC_MODULE(MIPS_Top) {
  sc_in<bool> clk;
  sc_in<bool> reset;

  // SINAIS IF
  sc_signal<sc_uint<32>> sig_quatro, sig_pc_atual, sig_pc_mais_4, sig_instrucao_bruta;
  sc_signal<sc_uint<32>> sig_if_id_pc4, sig_if_id_inst;
  sc_signal<sc_uint<32>> sig_branch_target, sig_pc_proximo;

  // SINAIS ID
  sc_signal<sc_uint<6>>  sig_opcode;
  sc_signal<sc_uint<5>>  sig_rs, sig_rt, sig_rd;
  sc_signal<sc_uint<16>> sig_imediato16;
  sc_signal<sc_int<32>>  sig_imediato32;
  sc_signal<bool>        sig_ctrl_reg_dst, sig_ctrl_alu_src, sig_ctrl_mem_to_reg;
  sc_signal<bool>        sig_ctrl_reg_write, sig_ctrl_mem_read, sig_ctrl_mem_write;
  sc_signal<bool>        sig_ctrl_jump, sig_ctrl_jn, sig_ctrl_jz;
  sc_signal<sc_uint<2>>  sig_ctrl_alu_op;
  sc_signal<sc_int<32>>  sig_reg_data1, sig_reg_data2;

  // SINAIS EX
  sc_signal<sc_uint<32>> sig_id_ex_pc4;
  sc_signal<sc_int<32>>  sig_id_ex_data1, sig_id_ex_data2, sig_id_ex_imediato32;
  sc_signal<sc_uint<5>>  sig_id_ex_rs, sig_id_ex_rt, sig_id_ex_rd;
  sc_signal<bool>        sig_id_ex_reg_write, sig_id_ex_mem_to_reg, sig_id_ex_mem_read, sig_id_ex_mem_write;
  sc_signal<bool>        sig_id_ex_reg_dst, sig_id_ex_alu_src;
  sc_signal<bool>        sig_id_ex_jump, sig_id_ex_jn, sig_id_ex_jz;
  sc_signal<sc_uint<2>>  sig_id_ex_alu_op;

  sc_signal<sc_int<32>> sig_mux_alu_src_out;
  sc_signal<sc_uint<5>> sig_mux_reg_dst_out;
  sc_signal<sc_int<32>> sig_ula_resultado;
  sc_signal<bool>       sig_ula_zero, sig_ula_negativo, sig_pc_src;
  sc_signal<sc_uint<4>> sig_ula_controle_real;

  // SINAIS MEM
  sc_signal<sc_int<32>> sig_ex_mem_ula_res, sig_ex_mem_data2;
  sc_signal<sc_uint<5>> sig_ex_mem_reg_dst;
  sc_signal<bool>       sig_ex_mem_reg_write, sig_ex_mem_mem_to_reg, sig_ex_mem_mem_read, sig_ex_mem_mem_write;
  sc_signal<sc_int<32>> sig_mem_dado_lido;

  // SINAIS WB
  sc_signal<bool>       sig_mem_wb_reg_write, sig_mem_wb_mem_to_reg;
  sc_signal<sc_int<32>> sig_mem_wb_dado_lido, sig_mem_wb_ula_res, sig_wb_dado_final;
  sc_signal<sc_uint<5>> sig_mem_wb_reg_dst;

  // INSTÂNCIAS
  Mux2_Uint32        *mux_pc;
  PC                 *pc;
  Somador            *add_pc;
  MemoriaInstrucoes  *mem_inst;
  Reg_IF_ID          *reg_if_id;
  SeparadorInstrucao *separador;
  Controle           *controle;
  BancoRegistradores *banco_reg;
  ExtensorSinal      *extensor;
  Reg_ID_EX          *reg_id_ex;
  Mux2_Int           *mux_alu_src;
  Mux2_Uint          *mux_reg_dst;
  ULA                *ula;
  ControleULA        *ctrl_ula;
  UnidadeSaltos      *uni_saltos;
  SomadorBranch      *add_branch;
  Reg_EX_MEM         *reg_ex_mem;
  MemoriaDados       *mem_dados;
  Reg_MEM_WB         *reg_mem_wb;
  Mux2_Int           *mux_wb;

  SC_CTOR(MIPS_Top) {
    sig_quatro.write(4);

    // MUX do PC
    mux_pc = new Mux2_Uint32("Mux_PC");
    mux_pc->entrada0(sig_pc_mais_4);
    mux_pc->entrada1(sig_branch_target);
    mux_pc->selecao(sig_pc_src);
    mux_pc->saida(sig_pc_proximo);

    // PC
    pc = new PC("ProgramCounter");
    pc->clk(clk);
    pc->reset(reset);
    pc->pc_in(sig_pc_proximo);
    pc->pc_out(sig_pc_atual);

    // Somador para calcular PC+4
    add_pc = new Somador("SomadorPC");
    add_pc->op_a(sig_pc_atual);
    add_pc->op_b(sig_quatro);
    add_pc->resultado(sig_pc_mais_4);

    // Memória de Instruções
    mem_inst = new MemoriaInstrucoes("ROM");
    mem_inst->endereco(sig_pc_atual);
    mem_inst->instrucao(sig_instrucao_bruta);

    // IF/ID
    reg_if_id = new Reg_IF_ID("Pipeline_IF_ID");
    reg_if_id->clk(clk);
    reg_if_id->pc_plus_4_in(sig_pc_mais_4);
    reg_if_id->inst_in(sig_instrucao_bruta);
    reg_if_id->pc_plus_4_out(sig_if_id_pc4);
    reg_if_id->inst_out(sig_if_id_inst);

    // Separador de Instrução
    separador = new SeparadorInstrucao("Fatiador");
    separador->instrucao32(sig_if_id_inst);
    separador->opcode(sig_opcode);
    separador->rs(sig_rs);
    separador->rt(sig_rt);
    separador->rd(sig_rd);
    separador->imediato(sig_imediato16);

    // Unidade de Controle
    controle = new Controle("UnidadeControle");
    controle->opcode(sig_opcode);
    controle->reg_dst(sig_ctrl_reg_dst);
    controle->alu_src(sig_ctrl_alu_src);
    controle->mem_to_reg(sig_ctrl_mem_to_reg);
    controle->reg_write(sig_ctrl_reg_write);
    controle->mem_read(sig_ctrl_mem_read);
    controle->mem_write(sig_ctrl_mem_write);
    controle->jump(sig_ctrl_jump);
    controle->jn(sig_ctrl_jn);
    controle->jz(sig_ctrl_jz);
    controle->alu_op(sig_ctrl_alu_op);
    
    // Extensor de Sinal
    extensor = new ExtensorSinal("Extensor");
    extensor->entrada(sig_imediato16);
    extensor->saida(sig_imediato32);

    // Banco de Registradores
    banco_reg = new BancoRegistradores("BancoRegs");
    banco_reg->clk(clk);
    banco_reg->read_reg1(sig_rs);
    banco_reg->read_reg2(sig_rt);
    banco_reg->read_data1(sig_reg_data1);
    banco_reg->read_data2(sig_reg_data2);
    banco_reg->reg_write(sig_mem_wb_reg_write);
    banco_reg->write_reg(sig_mem_wb_reg_dst);
    banco_reg->write_data(sig_wb_dado_final);

    // ID/EX
    reg_id_ex = new Reg_ID_EX("Pipeline_ID_EX");
    reg_id_ex->clk(clk);
    reg_id_ex->pc_plus_4_in(sig_if_id_pc4);
    reg_id_ex->reg_data1_in(sig_reg_data1);
    reg_id_ex->reg_data2_in(sig_reg_data2);
    reg_id_ex->imediato32_in(sig_imediato32);
    reg_id_ex->rs_in(sig_rs);
    reg_id_ex->rt_in(sig_rt);
    reg_id_ex->rd_in(sig_rd);
    reg_id_ex->ctrl_reg_write_in(sig_ctrl_reg_write);
    reg_id_ex->ctrl_mem_to_reg_in(sig_ctrl_mem_to_reg);
    reg_id_ex->ctrl_mem_read_in(sig_ctrl_mem_read);
    reg_id_ex->ctrl_mem_write_in(sig_ctrl_mem_write);
    reg_id_ex->ctrl_reg_dst_in(sig_ctrl_reg_dst);
    reg_id_ex->ctrl_alu_src_in(sig_ctrl_alu_src);
    reg_id_ex->ctrl_alu_op_in(sig_ctrl_alu_op);
    reg_id_ex->ctrl_jump_in(sig_ctrl_jump);
    reg_id_ex->ctrl_jn_in(sig_ctrl_jn);
    reg_id_ex->ctrl_jz_in(sig_ctrl_jz);
    reg_id_ex->pc_plus_4_out(sig_id_ex_pc4);
    reg_id_ex->reg_data1_out(sig_id_ex_data1);
    reg_id_ex->reg_data2_out(sig_id_ex_data2);
    reg_id_ex->imediato32_out(sig_id_ex_imediato32);
    reg_id_ex->rs_out(sig_id_ex_rs);
    reg_id_ex->rt_out(sig_id_ex_rt);
    reg_id_ex->rd_out(sig_id_ex_rd);
    reg_id_ex->ctrl_reg_write_out(sig_id_ex_reg_write);
    reg_id_ex->ctrl_mem_to_reg_out(sig_id_ex_mem_to_reg);
    reg_id_ex->ctrl_mem_read_out(sig_id_ex_mem_read);
    reg_id_ex->ctrl_mem_write_out(sig_id_ex_mem_write);
    reg_id_ex->ctrl_reg_dst_out(sig_id_ex_reg_dst);
    reg_id_ex->ctrl_alu_src_out(sig_id_ex_alu_src);
    reg_id_ex->ctrl_alu_op_out(sig_id_ex_alu_op);
    reg_id_ex->ctrl_jump_out(sig_id_ex_jump);
    reg_id_ex->ctrl_jn_out(sig_id_ex_jn);
    reg_id_ex->ctrl_jz_out(sig_id_ex_jz);

    // MUX do ALU Src
    mux_alu_src = new Mux2_Int("Mux_ALUSrc");
    mux_alu_src->entrada0(sig_id_ex_data2);
    mux_alu_src->entrada1(sig_id_ex_imediato32);
    mux_alu_src->selecao(sig_id_ex_alu_src);
    mux_alu_src->saida(sig_mux_alu_src_out);

    // MUX do RegDst
    mux_reg_dst = new Mux2_Uint("Mux_RegDst");
    mux_reg_dst->entrada0(sig_id_ex_rt);
    mux_reg_dst->entrada1(sig_id_ex_rd);
    mux_reg_dst->selecao(sig_id_ex_reg_dst);
    mux_reg_dst->saida(sig_mux_reg_dst_out);

    // Controle da ULA
    ctrl_ula = new ControleULA("ControleDaULA");
    ctrl_ula->alu_op(sig_id_ex_alu_op);
    ctrl_ula->imediato32(sig_id_ex_imediato32);
    ctrl_ula->operacao_ula(sig_ula_controle_real);

    // ULA
    ula = new ULA("ULA_Principal");
    ula->op_a(sig_id_ex_data1);
    ula->op_b(sig_mux_alu_src_out);
    ula->ula_ctrl(sig_ula_controle_real);
    ula->resultado(sig_ula_resultado);
    ula->flag_zero(sig_ula_zero);
    ula->flag_negativo(sig_ula_negativo);

    // Somador para cálculo de saltos (Branch)
    add_branch = new SomadorBranch("SomadorBranch");
    add_branch->op_a(sig_id_ex_pc4);
    add_branch->op_b(sig_id_ex_imediato32);
    add_branch->resultado(sig_branch_target);

    // Unidade de Saltos
    uni_saltos = new UnidadeSaltos("CalculoDePulo");
    uni_saltos->jump(sig_id_ex_jump);
    uni_saltos->jn(sig_id_ex_jn);
    uni_saltos->jz(sig_id_ex_jz);
    uni_saltos->flag_zero(sig_ula_zero);
    uni_saltos->flag_negativo(sig_ula_negativo);
    uni_saltos->pc_src(sig_pc_src); 

    // EX/MEM
    reg_ex_mem = new Reg_EX_MEM("Pipeline_EX_MEM");
    reg_ex_mem->clk(clk);
    reg_ex_mem->ula_resultado_in(sig_ula_resultado);
    reg_ex_mem->reg_data2_in(sig_id_ex_data2);
    reg_ex_mem->reg_dst_in(sig_mux_reg_dst_out);
    reg_ex_mem->ctrl_reg_write_in(sig_id_ex_reg_write);
    reg_ex_mem->ctrl_mem_to_reg_in(sig_id_ex_mem_to_reg);
    reg_ex_mem->ctrl_mem_read_in(sig_id_ex_mem_read);
    reg_ex_mem->ctrl_mem_write_in(sig_id_ex_mem_write);
    reg_ex_mem->ula_resultado_out(sig_ex_mem_ula_res);
    reg_ex_mem->reg_data2_out(sig_ex_mem_data2);
    reg_ex_mem->reg_dst_out(sig_ex_mem_reg_dst);
    reg_ex_mem->ctrl_reg_write_out(sig_ex_mem_reg_write);
    reg_ex_mem->ctrl_mem_to_reg_out(sig_ex_mem_mem_to_reg);
    reg_ex_mem->ctrl_mem_read_out(sig_ex_mem_mem_read);
    reg_ex_mem->ctrl_mem_write_out(sig_ex_mem_mem_write);

    // Memória de Dados
    mem_dados = new MemoriaDados("RAM_Dados");
    mem_dados->clk(clk);
    mem_dados->endereco(sig_ex_mem_ula_res);
    mem_dados->write_data(sig_ex_mem_data2);
    mem_dados->mem_write(sig_ex_mem_mem_write);
    mem_dados->mem_read(sig_ex_mem_mem_read);
    mem_dados->read_data(sig_mem_dado_lido);

    // MEM/WB
    reg_mem_wb = new Reg_MEM_WB("Pipeline_MEM_WB");
    reg_mem_wb->clk(clk);
    reg_mem_wb->ctrl_reg_write_in(sig_ex_mem_reg_write);
    reg_mem_wb->ctrl_mem_to_reg_in(sig_ex_mem_mem_to_reg);
    reg_mem_wb->mem_dado_lido_in(sig_mem_dado_lido);
    reg_mem_wb->ula_resultado_in(sig_ex_mem_ula_res);
    reg_mem_wb->reg_dst_in(sig_ex_mem_reg_dst);
    reg_mem_wb->ctrl_reg_write_out(sig_mem_wb_reg_write);
    reg_mem_wb->ctrl_mem_to_reg_out(sig_mem_wb_mem_to_reg);
    reg_mem_wb->mem_dado_lido_out(sig_mem_wb_dado_lido);
    reg_mem_wb->ula_resultado_out(sig_mem_wb_ula_res);
    reg_mem_wb->reg_dst_out(sig_mem_wb_reg_dst);

    // MUX do Write Back
    mux_wb = new Mux2_Int("Mux_WB");
    mux_wb->entrada0(sig_mem_wb_ula_res);
    mux_wb->entrada1(sig_mem_wb_dado_lido);
    mux_wb->selecao(sig_mem_wb_mem_to_reg);
    mux_wb->saida(sig_wb_dado_final);
  }

  // Destrutor
  ~MIPS_Top() {
    delete mux_pc;
    delete pc;
    delete add_pc;
    delete mem_inst;
    delete reg_if_id;
    delete separador;
    delete controle;
    delete banco_reg;
    delete extensor;
    delete reg_id_ex;
    delete mux_alu_src;
    delete mux_reg_dst;
    delete ula;
    delete ctrl_ula;
    delete uni_saltos;
    delete add_branch;
    delete reg_ex_mem;
    delete mem_dados;
    delete reg_mem_wb;
    delete mux_wb;
  }
};

#endif // _MIPS_TOP_H_