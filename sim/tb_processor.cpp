// tb_processor.cpp
#include <systemc.h>
#include "processor.h"

#define OP_LD   0b0111
#define OP_ADD  0b0101
#define OP_ST   0b1000
#define OP_J    0b1001

sc_uint<16> make_r_instr(sc_uint<4> op, sc_uint<3> rd, sc_uint<3> rs1, sc_uint<3> rs2) {
    sc_uint<16> instr = 0;
    instr.range(15,12) = op;
    instr.range(11,9)  = rd;
    instr.range(8,6)   = rs1;
    instr.range(5,3)   = rs2;
    return instr;
}

sc_uint<16> make_i_instr(sc_uint<4> op, sc_uint<3> rd, sc_uint<3> rs1, sc_uint<6> imm) {
    sc_uint<16> instr = 0;
    instr.range(15,12) = op;
    instr.range(11,9)  = rd;
    instr.range(8,6)   = rs1;
    instr.range(5,0)   = imm;
    return instr;
}

sc_uint<16> make_j_instr(sc_uint<4> op, sc_int<9> imm) {
    sc_uint<16> instr = 0;
    instr.range(15,12) = op;
    instr.range(8,0)   = imm;
    return instr;
}

SC_MODULE(TbProcessor) {
    sc_clock        clk;
    sc_signal<bool> reset;
    Processor       proc;

    void gen_reset() {
        reset.write(true);
        wait(2, SC_NS);
        reset.write(false);
    }

    void monitor() {
        static int cycle = 0;
        cout << "[" << sc_time_stamp() << "] Cycle " << dec << cycle
             << " | PC=0x" << hex << proc.get_pc()
             << " | Instr=0x" << proc.get_instr()
             << " | n=" << proc.get_n_flag() << " z=" << proc.get_z_flag()
             << endl;
        cycle++;
    }

    SC_CTOR(TbProcessor)
        : clk("clk", 10, SC_NS), proc("Processor")
    {
        proc.clk(clk);
        proc.reset(reset);
        SC_THREAD(gen_reset);
        SC_METHOD(monitor);
        sensitive << clk.posedge_event();
        dont_initialize();
    }
};

int sc_main(int argc, char* argv[]) {
    TbProcessor tb("TbProcessor");

    sc_uint<16> init_data[2] = { 0x0042, 0x0024 };
    tb.proc.initialize_dmem(init_data, 2);

    sc_uint<16> program[5];
    program[0] = make_i_instr(OP_LD,  1, 0, 0);
    program[1] = make_i_instr(OP_LD,  2, 0, 1);
    program[2] = make_r_instr(OP_ADD, 3, 1, 2);
    program[3] = make_i_instr(OP_ST,  3, 0, 2);
    program[4] = make_j_instr(OP_J,   0);

    tb.proc.load_program(program, 5);

    sc_start(400, SC_NS);  // tempo suficiente para todas as instruções

    sc_uint<16> result = tb.proc.read_dmem(2);
    cout << "\n=== Resultado da Simulação ===" << endl;
    cout << "mem[2] = 0x" << hex << result << endl;
    if (result == 0x0066)
        cout << "TESTE PASSOU! (0x42 + 0x24 = 0x66)" << endl;
    else
        cout << "TESTE FALHOU! Esperado 0x66, obtido 0x" << result << endl;

    return 0;
}