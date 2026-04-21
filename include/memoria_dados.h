#include <systemc.h>

SC_MODULE(MemoriaDados) {
  sc_in<bool> clk;
  sc_in<sc_uint<32>> endereco;
  sc_in<sc_int<32>> write_data;
  sc_in<bool> mem_write;
  sc_in<bool> mem_read;

  sc_out<sc_int<32>> read_data;

  sc_int<32> memoria[1024];

  void ler() {
    if (mem_read.read() == true) {
      sc_uint<32> indice = endereco.read() / 4;
      if (indice < 1024)
        read_data.write(memoria[indice]);
    } else {
      read_data.write(0);
    }
  }

  void escrever() {
    if (mem_write.read() == true) {
      sc_uint<32> indice = endereco.read() / 4;
      if (indice < 1024)
        memoria[indice] = write_data.read();
    }
  }

  SC_CTOR(MemoriaDados) {
    SC_METHOD(ler);
    sensitive << endereco << mem_read;

    SC_METHOD(escrever);
    sensitive << clk.pos();

    for (int i = 0; i < 1024; i++)
      memoria[i] = 0;
  }
};
