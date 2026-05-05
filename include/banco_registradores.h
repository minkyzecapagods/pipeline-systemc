#ifndef _BANCO_REG_H_
#define _BANCO_REG_H_

#include <systemc.h>

SC_MODULE(BancoRegistradores) {
  // Entradas  
  sc_in<bool>       clk;
  sc_in<bool>       reg_write;
  sc_in<sc_uint<5>> read_reg1;
  sc_in<sc_uint<5>> read_reg2;
  sc_in<sc_uint<5>> write_reg;
  sc_in<sc_int<32>> write_data;

  // Saídas
  sc_out<sc_int<32>> read_data1;
  sc_out<sc_int<32>> read_data2;

  // Banco de registradores (32 registradores de 32 bits)
  sc_int<32> registradores[32];

  // Leitura combinacional dos dados dos registradores
  void ler_dados() {
    read_data1.write(registradores[read_reg1.read()]);
    read_data2.write(registradores[read_reg2.read()]);
  }

  // Escrita síncrona dos dados na borda de súbida do clock
  void escrever_dados() {
    if (reg_write.read() == true && write_reg.read() != 0) {
      registradores[write_reg.read()] = write_data.read();
    }
  }

  // Construtor do módulo
  SC_CTOR(BancoRegistradores) {

    // Leitura sensível aos endereços de leitura
    SC_METHOD(ler_dados);
    sensitive << read_reg1 << read_reg2;

    // Escrita sensível à subida do clock
    SC_METHOD(escrever_dados);
    sensitive << clk.pos();

    // Inicializa os registradores com zero 
    for (int i = 0; i < 32; i++)
      registradores[i] = 0;
  }
};
#endif // _BANCO_REG_H_
