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
#include <systemc.h>

SC_MODULE(MIPS_Top) {
  sc_in<bool> clk;
  sc_in<bool> reset;

  // --- SINAIS IF ---
  sc_signal<sc_uint<32>> sig_quatro, sig_pc_atual, sig_pc_mais_4,
      sig_instrucao_bruta;
  sc_signal<sc_uint<32>> sig_if_id_pc4, sig_if_id_inst;

  // --- SINAIS ID ---
  sc_signal<sc_uint<6>> sig_opcode;
  sc_signal<sc_uint<5>> sig_rs, sig_rt, sig_rd;
  sc_signal<sc_uint<16>> sig_imediato16;
  sc_signal<sc_int<32>> sig_imediato32;
  sc_signal<bool> sig_ctrl_reg_dst, sig_ctrl_alu_src, sig_ctrl_mem_to_reg;
  sc_signal<bool> sig_ctrl_reg_write, sig_ctrl_mem_read, sig_ctrl_mem_write;
  sc_signal<bool> sig_ctrl_branch;
  sc_signal<sc_uint<2>> sig_ctrl_alu_op;
  sc_signal<sc_int<32>> sig_reg_data1, sig_reg_data2;

  // --- SINAIS EX ---
  sc_signal<sc_int<32>> sig_id_ex_data1, sig_id_ex_data2, sig_id_ex_imediato32;
  sc_signal<sc_uint<5>> sig_id_ex_rs, sig_id_ex_rt, sig_id_ex_rd;
  sc_signal<bool> sig_id_ex_reg_write, sig_id_ex_mem_to_reg, sig_id_ex_mem_read,
      sig_id_ex_mem_write;
  sc_signal<bool> sig_id_ex_reg_dst, sig_id_ex_alu_src;
  sc_signal<sc_uint<2>> sig_id_ex_alu_op;
  sc_signal<sc_int<32>> sig_mux_alu_src_out;
  sc_signal<sc_uint<5>> sig_mux_reg_dst_out;
  sc_signal<sc_int<32>> sig_ula_resultado;
  sc_signal<bool> sig_ula_zero;
  sc_signal<sc_uint<4>> sig_ula_controle_simulado;

  // --- SINAIS MEM ---
  sc_signal<sc_int<32>> sig_ex_mem_ula_res, sig_ex_mem_data2;
  sc_signal<sc_uint<5>> sig_ex_mem_reg_dst;
  sc_signal<bool> sig_ex_mem_reg_write, sig_ex_mem_mem_to_reg,
      sig_ex_mem_mem_read, sig_ex_mem_mem_write;
  sc_signal<sc_int<32>> sig_mem_dado_lido;

  // --- SINAIS WB (A Volta!) ---
  sc_signal<bool> sig_mem_wb_reg_write, sig_mem_wb_mem_to_reg;
  sc_signal<sc_int<32>> sig_mem_wb_dado_lido, sig_mem_wb_ula_res;
  sc_signal<sc_uint<5>>
      sig_mem_wb_reg_dst; // Volta para ligar na porta write_reg do Banco
  sc_signal<sc_int<32>>
      sig_wb_dado_final; // Volta para ligar na porta write_data do Banco  //

  // --- INSTÂNCIAS ---
  PC *pc;
  Somador *add_pc;
  MemoriaInstrucoes *mem_inst;
  Reg_IF_ID *reg_if_id;
  SeparadorInstrucao *separador;
  Controle *controle;
  BancoRegistradores *banco_reg;
  ExtensorSinal *extensor;
  Reg_ID_EX *reg_id_ex;
  Mux2_Int *mux_alu_src;
  Mux2_Uint *mux_reg_dst;
  ULA *ula;
  Reg_EX_MEM *reg_ex_mem;
  MemoriaDados *mem_dados;
  Reg_MEM_WB *reg_mem_wb;
  Mux2_Int *mux_wb;
  ControleULA *ctrl_ula;
  sc_signal<sc_uint<6>> sig_ex_funct;

  SC_CTOR(MIPS_Top) {
    sig_quatro.write(4);
    sig_ula_controle_simulado.write(2);

    // ================= IF, IF/ID =================
    pc = new PC("ProgramCounter");
    pc->clk(clk);
    pc->reset(reset);
    pc->pc_in(sig_pc_mais_4);
    pc->pc_out(sig_pc_atual);
    add_pc = new Somador("SomadorPC");
    add_pc->op_a(sig_pc_atual);
    add_pc->op_b(sig_quatro);
    add_pc->resultado(sig_pc_mais_4);
    mem_inst = new MemoriaInstrucoes("ROM");
    mem_inst->endereco(sig_pc_atual);
    mem_inst->instrucao(sig_instrucao_bruta);
    reg_if_id = new Reg_IF_ID("Pipeline_IF_ID");
    reg_if_id->clk(clk);
    reg_if_id->pc_plus_4_in(sig_pc_mais_4);
    reg_if_id->inst_in(sig_instrucao_bruta);
    reg_if_id->pc_plus_4_out(sig_if_id_pc4);
    reg_if_id->inst_out(sig_if_id_inst);

    // ================= ID =================
    separador = new SeparadorInstrucao("Fatiador");
    separador->instrucao32(sig_if_id_inst);
    separador->opcode(sig_opcode);
    separador->rs(sig_rs);
    separador->rt(sig_rt);
    separador->rd(sig_rd);
    separador->imediato(sig_imediato16);
    controle = new Controle("UnidadeControle");
    controle->opcode(sig_opcode);
    controle->reg_dst(sig_ctrl_reg_dst);
    controle->alu_src(sig_ctrl_alu_src);
    controle->mem_to_reg(sig_ctrl_mem_to_reg);
    controle->reg_write(sig_ctrl_reg_write);
    controle->mem_read(sig_ctrl_mem_read);
    controle->mem_write(sig_ctrl_mem_write);
    controle->branch(sig_ctrl_branch);
    controle->alu_op(sig_ctrl_alu_op);
    extensor = new ExtensorSinal("Extensor");
    extensor->entrada(sig_imediato16);
    extensor->saida(sig_imediato32);

    // ***************** O GRANDE ENCONTRO: BANCO DE REGISTRADORES
    // *****************
    banco_reg = new BancoRegistradores("BancoRegs");
    banco_reg->clk(clk);
    // Lado da Leitura (Estágio ID)
    banco_reg->read_reg1(sig_rs);
    banco_reg->read_reg2(sig_rt);
    banco_reg->read_data1(sig_reg_data1);
    banco_reg->read_data2(sig_reg_data2);
    // Lado da Escrita (Os Fios do Estágio WB deram a volta e ligaram aqui!)
    banco_reg->reg_write(sig_mem_wb_reg_write); // Ativa gravação
    banco_reg->write_reg(sig_mem_wb_reg_dst);   // Registrador que salvará
    banco_reg->write_data(sig_wb_dado_final);   // Dado para gravar

    // ================= ID/EX, EX, EX/MEM, MEM =================
    reg_id_ex = new Reg_ID_EX("Pipeline_ID_EX");
    reg_id_ex->clk(clk);
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
    mux_alu_src = new Mux2_Int("Mux_ALUSrc");
    mux_alu_src->entrada0(sig_id_ex_data2);
    mux_alu_src->entrada1(sig_id_ex_imediato32);
    mux_alu_src->selecao(sig_id_ex_alu_src);
    mux_alu_src->saida(sig_mux_alu_src_out);
    mux_reg_dst = new Mux2_Uint("Mux_RegDst");
    mux_reg_dst->entrada0(sig_id_ex_rt);
    mux_reg_dst->entrada1(sig_id_ex_rd);
    mux_reg_dst->selecao(sig_id_ex_reg_dst);
    mux_reg_dst->saida(sig_mux_reg_dst_out);
    ctrl_ula = new ControleULA("ControleDaULA");
    ctrl_ula->alu_op(sig_id_ex_alu_op);
    ctrl_ula->imediato32(sig_id_ex_imediato32);
    ctrl_ula->operacao_ula(
        sig_ula_controle_simulado); // Substitui a constante pela decisão real
    ula = new ULA("ULA_Principal");
    ula->op_a(sig_id_ex_data1);
    ula->op_b(sig_mux_alu_src_out);
    ula->ula_ctrl(sig_ula_controle_simulado);
    ula->resultado(sig_ula_resultado);
    ula->flag_zero(sig_ula_zero);
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
    mem_dados = new MemoriaDados("RAM_Dados");
    mem_dados->clk(clk);
    mem_dados->endereco(sig_ex_mem_ula_res);
    mem_dados->write_data(sig_ex_mem_data2);
    mem_dados->mem_write(sig_ex_mem_mem_write);
    mem_dados->mem_read(sig_ex_mem_mem_read);
    mem_dados->read_data(sig_mem_dado_lido);

    // ================= BARREIRA PIPELINE: MEM / WB =================
    reg_mem_wb = new Reg_MEM_WB("Pipeline_MEM_WB");
    reg_mem_wb->clk(clk);
    reg_mem_wb->ctrl_reg_write_in(sig_ex_mem_reg_write);
    reg_mem_wb->ctrl_mem_to_reg_in(sig_ex_mem_mem_to_reg);
    reg_mem_wb->mem_dado_lido_in(sig_mem_dado_lido);
    reg_mem_wb->ula_resultado_in(sig_ex_mem_ula_res);
    reg_mem_wb->reg_dst_in(sig_ex_mem_reg_dst);

    reg_mem_wb->ctrl_reg_write_out(
        sig_mem_wb_reg_write); // Este sinal vai direto para a porta
                               // write_reg_en do Banco!
    reg_mem_wb->ctrl_mem_to_reg_out(sig_mem_wb_mem_to_reg);
    reg_mem_wb->mem_dado_lido_out(sig_mem_wb_dado_lido);
    reg_mem_wb->ula_resultado_out(sig_mem_wb_ula_res);
    reg_mem_wb->reg_dst_out(
        sig_mem_wb_reg_dst); // Este vai para a porta de endereço do Banco!

    // ================= ESTÁGIO 5: WRITE-BACK (WB) =================
    // Mux Final: Decide qual dado volta para o Banco (da ULA ou da Memória)
    mux_wb = new Mux2_Int("Mux_WB");
    mux_wb->entrada0(sig_mem_wb_ula_res); // Se 0, salva o resultado da ULA
    mux_wb->entrada1(
        sig_mem_wb_dado_lido); // Se 1, salva o que puxou da RAM (LD)
    mux_wb->selecao(sig_mem_wb_mem_to_reg);
    mux_wb->saida(sig_wb_dado_final); // Este fio se liga lá em cima na porta
                                      // write_data do Banco!
  }
  ~MIPS_Top() {
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
    delete reg_ex_mem;
    delete mem_dados;
    delete reg_mem_wb;
    delete mux_wb;
    delete ctrl_ula;
  }
};
#endif
