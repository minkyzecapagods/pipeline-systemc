#ifndef _UNIDADE_SALTOS_H_
#define _UNIDADE_SALTOS_H_
#include <systemc.h>

SC_MODULE(UnidadeSaltos) {
  sc_in<bool> jump;
  sc_in<bool> jn;
  sc_in<bool> jz;
  sc_in<bool> flag_zero;
  sc_in<bool> flag_negativo;

  sc_out<bool> pc_src; // Manda o PC mudar de rota

  void calcular() {
    bool deve_pular = jump.read() || (jn.read() && flag_negativo.read()) ||
                      (jz.read() && flag_zero.read());
    pc_src.write(deve_pular);
  }

  SC_CTOR(UnidadeSaltos) {
    SC_METHOD(calcular);
    sensitive << jump << jn << jz << flag_zero << flag_negativo;
  }
};
#endif
