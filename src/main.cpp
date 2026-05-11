#include "../include/mips_top.h"
#include <fstream>
#include <iostream>
#include <string>
#include <systemc.h>
#include <vector>

using namespace std;

int sc_main(int argc, char *argv[]) {
  cout << "Inicializando Simulacao Profissional do Pipeline MIPS..." << endl;

  // Verificacao do argumento de linha de comando
  if (argc < 2) {
    cerr << "Erro: Nenhum arquivo de teste fornecido." << endl;
    cerr << "Uso: ./mips_sim <caminho_para_arquivo.txt>" << endl;
    return 1;
  }

  string caminho_arquivo = argv[1];
  cout << "Lendo arquivo de instrucoes: " << caminho_arquivo << endl;

  // Leitura do arquivo de texto
  vector<sc_uint<32>> algoritmo;
  ifstream arquivo(caminho_arquivo);

  if (!arquivo.is_open()) {
    cerr << "Erro: Nao foi possivel abrir o arquivo " << caminho_arquivo << endl;
    return 1;
  }

  string linha;
  while (getline(arquivo, linha)) {
    // Ignora linhas vazias ou comentários (iniciados com # ou //)
    if (linha.empty() || linha[0] == '#' || linha[0] == '/') {
      continue;
    }

    // Converte a string em hexadecimal para um inteiro de 32 bits
    try {
      sc_uint<32> instrucao = stoul(linha, nullptr, 16);
      algoritmo.push_back(instrucao);
    } catch (const exception &e) {
      cerr << "Aviso: Linha ignorada por formato invalido -> " << linha << endl;
    }
  }

  arquivo.close();

  cout << "Foram carregadas " << algoritmo.size() << " instrucoes." << endl;

  // Instanciando o Microprocessador
  sc_clock clk("clk", 10, SC_NS);
  sc_signal<bool> reset;

  MIPS_Top mips("MeuProcessador");
  mips.clk(clk);
  mips.reset(reset);

  // Carrega algoritmo na memoria de instrução
  mips.mem_inst->carregar_programa(algoritmo);

  // Configuração do GTKWave
  sc_trace_file *wf = sc_create_vcd_trace_file("simulacao");
  sc_trace(wf, clk, "1_Clock");
  sc_trace(wf, mips.sig_if_id_inst, "2_IF_Instrucao");
  sc_trace(wf, mips.sig_rs, "3_ID_RS");
  sc_trace(wf, mips.sig_rt, "4_ID_RT");
  sc_trace(wf, mips.sig_ula_resultado, "5_EX_Resultado_ULA");
  sc_trace(wf, mips.sig_wb_dado_final, "6_WB_Dado_Escrito");
  sc_trace(wf, mips.sig_mem_wb_reg_dst, "7_WB_Reg_Destino");

  // Execução do Pipeline
  reset.write(true);
  sc_start(10, SC_NS); // Reset inicial
  reset.write(false);

  // Roda os ciclos necessários (cap pro tamanho do algoritmo)
  sc_start(150, SC_NS);

  sc_close_vcd_trace_file(wf);

  cout << "\nSimulacao concluida com sucesso!" << endl;
  cout << "Verifique o arquivo 'simulacao.vcd' no GTKWave." << endl;

  return 0;
}
