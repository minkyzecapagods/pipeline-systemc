// ir.h
#ifndef IR_H
#define IR_H

#include <systemc.h>

SC_MODULE(InstructionRegister) {
public:
    sc_in<bool>         clk;
    sc_in<bool>         reset;
    sc_in<bool>         write_enable;
    sc_in<sc_uint<16>>  data_in;
    sc_out<sc_uint<16>> data_out;

    void update() {
        if (reset.read()) {
            data_out.write(0);
        } else if (write_enable.read()) {
            data_out.write(data_in.read());
        }
    }

    SC_CTOR(InstructionRegister) {
        SC_METHOD(update);
        sensitive << clk.pos();
    }
};

#endif // IR_H