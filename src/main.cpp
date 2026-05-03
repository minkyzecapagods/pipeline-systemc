#include "../include/mips_top.h"
#include <fstream>
#include <iostream>
#include <string>
#include <systemc.h>
#include <vector>

int sc_main(int argc, char *argv[]) {
  std::cout << "Inicializando Simulacao Profissional do Pipeline MIPS..."
            << std::endl;

  // 1. Verificacao do argumento de linha de comando
  if (argc < 2) {
    std::cerr << "Erro: Nenhum arquivo de teste fornecido." << std::endl;
    std::cerr << "Uso: ./mips_sim <caminho_para_arquivo.txt>" << std::endl;
    return 1;
  }

  std::string caminho_arquivo = argv[1];
  std::cout << "Lendo arquivo de instrucoes: " << caminho_arquivo << std::endl;

  // 2. Leitura do arquivo de texto
  std::vector<sc_uint<32>> algoritmo;
  std::ifstream arquivo(caminho_arquivo);

  if (!arquivo.is_open()) {
    std::cerr << "Erro: Nao foi possivel abrir o arquivo " << caminho_arquivo
              << std::endl;
    return 1;
  }

  std::string linha;
  while (std::getline(arquivo, linha)) {
    // Ignorar linhas vazias ou comentários (iniciados com # ou //)
    if (linha.empty() || linha[0] == '#' || linha[0] == '/') {
      continue;
    }

    // Converter a string em hexadecimal para um inteiro de 32 bits
    try {
      sc_uint<32> instrucao = std::stoul(linha, nullptr, 16);
      algoritmo.push_back(instrucao);
    } catch (const std::exception &e) {
      std::cerr << "Aviso: Linha ignorada por formato invalido -> " << linha
                << std::endl;
    }
  }
  arquivo.close();

  std::cout << "Foram carregadas " << algoritmo.size() << " instrucoes."
            << std::endl;

  // 3. Instanciando o Microprocessador
  sc_clock clk("clk", 10, SC_NS);
  sc_signal<bool> reset;

  MIPS_Top mips("MeuProcessador");
  mips.clk(clk);
  mips.reset(reset);

  // 4. Carregando na memoria ROM do chip
  mips.mem_inst->carregar_programa(algoritmo);

  // 5. Configuração do GTKWave
  sc_trace_file *wf = sc_create_vcd_trace_file("simulacao_mips_final");
  sc_trace(wf, clk, "1_Clock");
  sc_trace(wf, mips.sig_if_id_inst, "2_IF_Instrucao");
  sc_trace(wf, mips.sig_rs, "3_ID_RS");
  sc_trace(wf, mips.sig_rt, "4_ID_RT");
  sc_trace(wf, mips.sig_ula_resultado, "5_EX_Resultado_ULA");
  sc_trace(wf, mips.sig_wb_dado_final, "6_WB_Dado_Escrito");
  sc_trace(wf, mips.sig_mem_wb_reg_dst, "7_WB_Reg_Destino");

  // 6. Execução do Pipeline
  reset.write(true);
  sc_start(10, SC_NS); // Reset inicial
  reset.write(false);

  // Roda os ciclos necessários (pode aumentar se o arquivo for grande)
  sc_start(150, SC_NS);

  sc_close_vcd_trace_file(wf);

  std::cout << "\nSimulacao concluida com sucesso!" << std::endl;
  std::cout << "Verifique o arquivo 'simulacao_mips_final.vcd' no GTKWave."
            << std::endl;

  return 0;
}
