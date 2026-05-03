#ifndef _MIPS_TOP_H_
#define _MIPS_TOP_H_
#include "banco_registradores.h"
#include "memoria_instrucoes.h"
#include "pc.h"
#include "registradores_pipeline.h"
#include "somador.h"
#include <systemc.h>

SC_MODULE(MIPS_Top) {
  // Pinos de I/O do processador[cite: 12]
  sc_in<bool> clk;
  sc_in<bool> reset;

  // Fios da placa (Sinais Internos)
  sc_signal<sc_uint<32>> sig_quatro;
  sc_signal<sc_uint<32>> sig_pc_atual, sig_pc_mais_4;
  sc_signal<sc_uint<32>> sig_instrucao_bruta;

  // Fios na saída do Registrador IF/ID
  sc_signal<sc_uint<32>> sig_if_id_pc4;
  sc_signal<sc_uint<32>> sig_if_id_inst;

  // Instâncias
  PC *pc;
  Somador *add_pc;
  MemoriaInstrucoes *mem_inst;
  Reg_IF_ID *reg_if_id;

  SC_CTOR(MIPS_Top) {
    sig_quatro.write(4); // Constante 4

    // --- ESTÁGIO IF (Busca) ---
    pc = new PC("ProgramCounter");
    pc->clk(clk);
    pc->reset(reset);
    pc->pc_in(
        sig_pc_mais_4); // Por enquanto, liga direto no PC+4 (sem jumps ainda)
    pc->pc_out(sig_pc_atual);

    add_pc = new Somador("SomadorPC");
    add_pc->op_a(sig_pc_atual);
    add_pc->op_b(sig_quatro);
    add_pc->resultado(sig_pc_mais_4);

    mem_inst = new MemoriaInstrucoes("ROM");
    mem_inst->endereco(sig_pc_atual);
    mem_inst->instrucao(sig_instrucao_bruta);

    // --- BARREIRA PIPELINE IF/ID ---
    reg_if_id = new Reg_IF_ID("Pipeline_IF_ID");
    reg_if_id->clk(clk);
    reg_if_id->pc_plus_4_in(sig_pc_mais_4);
    reg_if_id->inst_in(sig_instrucao_bruta);
    reg_if_id->pc_plus_4_out(sig_if_id_pc4);
    reg_if_id->inst_out(sig_if_id_inst);

    // O estágio ID e os demais componentes entrariam daqui para baixo...
  }

  ~MIPS_Top() {
    delete pc;
    delete add_pc;
    delete mem_inst;
    delete reg_if_id;
  }
};
#endif
