#include "mips_top.h"
#include <systemc.h>

int sc_main(int argc, char *argv[]) {
  // 1. Criação do sinal de Clock (Período de 10ns)
  sc_clock clk("clk", 10, SC_NS);

  // 2. Instanciação do Processador Top-Level
  MIPS_Top mips("MeuProcessadorMIPS");
  mips.clk(clk);

  // 3. Configuração do arquivo de visualização (GTKWave)
  sc_trace_file *wf = sc_create_vcd_trace_file("simulacao_mips");
  sc_trace(wf, clk, "clock");
  // Você pode rastrear sinais internos aqui para o relatório:
  // sc_trace(wf, mips.sig_pc_out, "PC");
  // sc_trace(wf, mips.sig_instrucao, "Instrucao_Atual");

  // 4. Rodar a simulação
  cout << "Simulando processador MIPS..." << endl;
  sc_start(200, SC_NS); // Roda por 20 ciclos de 10ns

  sc_close_vcd_trace_file(wf);
  cout << "Simulacao concluida. Abra 'simulacao_mips.vcd' no GTKWave." << endl;

  return 0;
}
