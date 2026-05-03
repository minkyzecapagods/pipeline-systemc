#include "../include/mips_top.h"
#include <systemc.h>
#include <vector>

int sc_main(int argc, char *argv[]) {
  std::cout << "Preparando Testbench do Processador RISC..." << std::endl;

  // 1. Criação dos sinais externos[cite: 12]
  sc_clock clk("clk", 10, SC_NS);
  sc_signal<bool> reset;

  // 2. Instanciando o Microprocessador
  MIPS_Top mips("MeuMIPS");
  mips.clk(clk);
  mips.reset(reset);

  // 3. Algoritmo em C++ puro (Ex: ADD, SUB, AND)[cite: 11]
  // Traduzido para código de máquina (Hexadecimal)
  std::vector<sc_uint<32>> algoritmo = {
      0x00430820, // ADD $1, $2, $3
      0x00252022, // SUB $4, $1, $5
      0x00243024  // AND $6, $1, $4
  };

  // Carregando na memória ROM do chip antes de dar a partida
  std::cout << "Carregando instrucoes na ROM..." << std::endl;
  mips.mem_inst->carregar_programa(algoritmo);

  // 4. Configurando a visualização para o GTKWave
  sc_trace_file *wf = sc_create_vcd_trace_file("simulacao_mips");
  sc_trace(wf, clk, "Clock");
  sc_trace(wf, reset, "Reset");
  sc_trace(wf, mips.sig_pc_atual, "Endereco_PC_IF");
  sc_trace(wf, mips.sig_instrucao_bruta, "Instrucao_Saida_ROM");
  sc_trace(wf, mips.sig_if_id_inst, "Instrucao_Dentro_ID");

  // 5. Simulação
  std::cout << "Aplicando Reset..." << std::endl;
  reset.write(true);
  sc_start(10, SC_NS); // Pulsa o reset
  reset.write(false);

  std::cout << "Rodando o clock..." << std::endl;
  sc_start(100, SC_NS); // Roda 10 ciclos do processador

  sc_close_vcd_trace_file(wf);
  std::cout << "Simulacao concluida. Verifique o GTKWave!" << std::endl;

  return 0;
}
