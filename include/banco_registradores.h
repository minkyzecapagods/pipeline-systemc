#include <systemc.h>

SC_MODULE(BancoRegistradores) {
  sc_in<bool> clk;
  sc_in<bool> reg_write;
  sc_in<sc_uint<5>> read_reg1;
  sc_in<sc_uint<5>> read_reg2;
  sc_in<sc_uint<5>> write_reg;
  sc_in<sc_int<32>> write_data;

  sc_out<sc_int<32>> read_data1;
  sc_out<sc_int<32>> read_data2;

  sc_int<32> registradores[32];

  void ler_dados() {
    read_data1.write(registradores[read_reg1.read()]);
    read_data2.write(registradores[read_reg2.read()]);
  }

  void escrever_dados() {
    if (reg_write.read() == true && write_reg.read() != 0) {
      registradores[write_reg.read()] = write_data.read();
    }
  }

  SC_CTOR(BancoRegistradores) {
    SC_METHOD(ler_dados);
    sensitive << read_reg1 << read_reg2; // Sensível aos endereços de leitura

    SC_METHOD(escrever_dados);
    sensitive << clk.pos(); // Escrita síncrona

    for (int i = 0; i < 32; i++)
      registradores[i] = 0; // Inicializa com 0
  }
};
