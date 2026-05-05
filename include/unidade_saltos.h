#ifndef _UNIDADE_SALTOS_H_
#define _UNIDADE_SALTOS_H_

#include <systemc.h>

SC_MODULE(UnidadeSaltos) {
  // Entradas
  sc_in<bool> jump, jn, jz, flag_zero, flag_negativo;

  // Saída
  sc_out<bool> pc_src; // Manda o PC mudar de rota

  // Método combinacional para calcular se o salto deve ser tomado ou não
  void calcular() {
    bool deve_pular = jump.read() || (jn.read() && flag_negativo.read()) ||
                      (jz.read() && flag_zero.read());
    pc_src.write(deve_pular);
  }

  // Construtor do módulo
  SC_CTOR(UnidadeSaltos) {
    SC_METHOD(calcular);
    sensitive << jump << jn << jz << flag_zero << flag_negativo;
  }
};
#endif // _UNIDADE_SALTOS_H_
