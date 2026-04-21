#include "mips_top.h"
#include <systemc.h>

int sc_main(int argc, char *argv[]) {
  // Verifica se o usuário passou um arquivo de programa
  if (argc < 2) {
    cout << "Uso: ./mips_sim <caminho_para_o_arquivo.txt>" << endl;
    return 1;
  }

  // 1. Instanciação do Hardware
  sc_clock clk("clk", 10, SC_NS);
  MIPS_Top mips("MeuProcessadorMIPS");
  mips.clk(clk);

  // 2. Carregar o Software no Hardware ANTES da simulação
  cout << "Carregando o programa: " << argv[1] << endl;
  mips.mem_inst->carregar_programa(argv[1]);

  // 3. Configuração do GTKWave
  sc_trace_file *wf = sc_create_vcd_trace_file("simulacao_mips");
  sc_trace(wf, clk, "clock");
  sc_trace(wf, mips.sig_pc_out, "PC");
  sc_trace(wf, mips.sig_instrucao, "Instrucao");
  // Dica: rastreie também a saída da ULA e os registradores para facilitar a
  // depuração

  // 4. Executar
  cout << "Simulando..." << endl;
  sc_start(200, SC_NS);

  sc_close_vcd_trace_file(wf);
  cout << "Simulacao concluida." << endl;

  return 0;
}
