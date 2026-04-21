#include <fstream>
#include <iomanip>
#include <iostream>
#include <systemc.h>

SC_MODULE(MemoriaInstrucoes) {
  sc_in<sc_uint<32>> endereco;
  sc_out<sc_uint<32>> instrucao;

  sc_uint<32> memoria[1024];

  void ler_instrucao() {
    sc_uint<32> indice = endereco.read() / 4;
    if (indice < 1024) {
      instrucao.write(memoria[indice]);
    } else {
      instrucao.write(0);
    }
  }

  // NOVO: Método para carregar código de máquina de um arquivo .txt
  void carregar_programa(const char *nome_arquivo) {
    std::ifstream arquivo(nome_arquivo);
    if (!arquivo.is_open()) {
      std::cerr << "Erro: Nao foi possivel abrir o arquivo " << nome_arquivo
                << std::endl;
      return;
    }

    std::string linha;
    int i = 0;
    // Lê linha por linha em hexadecimal
    while (std::getline(arquivo, linha) && i < 1024) {
      // Converte a string hex (ex: "00430820") para inteiro
      memoria[i] = std::stoul(linha, nullptr, 16);
      i++;
    }
    arquivo.close();
    std::cout << "Carregou " << i << " instrucoes na memoria." << std::endl;
  }

  SC_CTOR(MemoriaInstrucoes) {
    SC_METHOD(ler_instrucao);
    sensitive << endereco;

    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
