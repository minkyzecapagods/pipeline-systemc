#ifndef _EXTENSOR_H_
#define _EXTENSOR_H_

#include <systemc.h>

SC_MODULE(ExtensorSinal) {
  // Entrada e saída
  sc_in<sc_uint<16>> entrada;
  sc_out<sc_int<32>> saida;

  // Método combinacional para estender o sinal de 16 bits para 32 bits com extensão de sinal
  void estender() {
    sc_int<16> valor_com_sinal = entrada.read().to_int();
    saida.write((sc_int<32>)valor_com_sinal);
  }

  // Construtor do módulo
  SC_CTOR(ExtensorSinal) {
    SC_METHOD(estender);
    sensitive << entrada;
  }
};
#endif // _EXTENSOR_H_
