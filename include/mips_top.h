#include "banco_registradores.h"
#include "controle.h"
#include "extensor_sinal.h"
#include "memoria_dados.h"
#include "memoria_instrucoes.h"
#include "mux2.h"
#include "pc.h"
#include "registradores_pipeline.h"
#include "somador.h"
#include "ula.h"
#include <systemc.h>

SC_MODULE(MIPS_Top) {
  // Porta de clock externa
  sc_in<bool> clk;

  // Sinais Internos (Os "fios" que conectam tudo)
  sc_signal<sc_uint<32>> sig_pc_in, sig_pc_out, sig_pc_4;
  sc_signal<sc_uint<32>> sig_instrucao;
  sc_signal<sc_uint<32>> sig_quatro;
  // ... adicione todos os sinais para conectar ULA, Banco de Reg, etc.

  // Instâncias dos Componentes
  PC *pc;
  MemoriaInstrucoes *mem_inst;
  Somador *add_pc;
  ULA *ula_principal;
  // ... instancie todos os outros módulos

  SC_CTOR(MIPS_Top) {
    // 1. Instanciar PC e Memória de Instruções (Estágio IF)
    pc = new PC("ProgramCounter");
    pc->clk(clk);
    pc->pc_in(sig_pc_in);
    pc->pc_out(sig_pc_out);

    mem_inst = new MemoriaInstrucoes("MemoriaInst");
    mem_inst->endereco(sig_pc_out);
    mem_inst->instrucao(sig_instrucao);

    add_pc = new Somador("SomadorPC");
    add_pc->op_a(sig_pc_out);
    add_pc->op_b(sig_quatro); // Soma 4 bytes
    add_pc->resultado(sig_pc_4);

    // Escreva o valor constante no sinal
    sig_quatro.write(4);

    // 2. Conexões do restante do Datapath (ID, EX, MEM, WB)
    // Aqui você conectará a ULA, Banco de Reg e os Registradores de Pipeline
    // conforme o diagrama do MIPS.
  }
};
