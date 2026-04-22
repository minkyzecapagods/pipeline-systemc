/**
 * @file   regfile.h
 * @brief  Banco de registradores 8x16 bits com R0 hardwired a zero.
 *         Leitura combinacional, escrita síncrona na borda de subida do clock.
 */

#ifndef REGFILE_H
#define REGFILE_H

#include <systemc.h>

/**
 * @class RegFile
 * @brief  Banco de registradores com 8 registradores de 16 bits cada.
 */

SC_MODULE(RegFile) {
public:
    static constexpr int NUM_REGS    = 8;
    static constexpr int DATA_WIDTH  = 16;
    static constexpr int ADDR_WIDTH  = 3;

    sc_in<sc_uint<ADDR_WIDTH>> read_addr1;
    sc_in<sc_uint<ADDR_WIDTH>> read_addr2;
    sc_in<sc_uint<ADDR_WIDTH>> write_addr;
    sc_in<sc_uint<DATA_WIDTH>> write_data;
    sc_in<bool>               write_enable;
    sc_in<bool>               clock;

    sc_out<sc_uint<DATA_WIDTH>> read_data1;
    sc_out<sc_uint<DATA_WIDTH>> read_data2;

    sc_vector<sc_signal<sc_uint<DATA_WIDTH>>> registers;

    void read_process() {
        sc_uint<ADDR_WIDTH> addr1 = read_addr1.read();
        sc_uint<ADDR_WIDTH> addr2 = read_addr2.read();

        read_data1.write( (addr1 == 0) ? sc_uint<DATA_WIDTH>(0) : registers[addr1].read() );
        read_data2.write( (addr2 == 0) ? sc_uint<DATA_WIDTH>(0) : registers[addr2].read() );
    }

    void write_process() {
        if (write_enable.read()) {
            sc_uint<ADDR_WIDTH> addr = write_addr.read();
            if (addr != 0) {
                registers[addr].write(write_data.read());
            }
        }
    }

    SC_CTOR(RegFile)
        : registers("reg", NUM_REGS)
    {
        for (auto& reg : registers) {
            reg.write(0);
        }

        SC_METHOD(read_process);
        sensitive << read_addr1 << read_addr2;
        for (auto& reg : registers) {
            sensitive << reg;
        }

        SC_METHOD(write_process);
        sensitive << clock.pos();
    }
};

#endif // REGFILE_H