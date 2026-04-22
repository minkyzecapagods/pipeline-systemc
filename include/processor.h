// processor.h
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <systemc.h>
#include "datapath.h"
#include "control.h"

SC_MODULE(Processor) {
public:
    sc_in<bool>  clk;
    sc_in<bool>  reset;

    // Sinais internos
    sc_signal<sc_uint<16>> instr;
    sc_signal<bool>        n_flag;
    sc_signal<bool>        z_flag;
    sc_signal<sc_uint<8>>  pc_out;

    sc_signal<bool>        pc_write;
    sc_signal<sc_uint<8>>  next_pc;
    sc_signal<bool>        branch_taken;
    sc_signal<sc_uint<8>>  branch_target;
    sc_signal<sc_uint<3>>  raddr1;
    sc_signal<sc_uint<3>>  raddr2;
    sc_signal<sc_uint<3>>  waddr;
    sc_signal<bool>        reg_write;
    sc_signal<sc_uint<4>>  alu_op;
    sc_signal<bool>        alu_srcB;
    sc_signal<bool>        mem_read;
    sc_signal<bool>        mem_write;
    sc_signal<bool>        mem_to_reg;
    sc_signal<bool>        ir_write;

    Datapath      datapath_inst;
    ControlUnit   control_inst;

    SC_CTOR(Processor)
        : datapath_inst("Datapath"),
          control_inst("ControlUnit")
    {
        // ** CORREÇÃO: habilita o registrador de instrução **
        ir_write.write(true);

        // Conexões do Datapath
        datapath_inst.clk(clk);
        datapath_inst.reset(reset);
        datapath_inst.pc_write(pc_write);
        datapath_inst.next_pc(next_pc);
        datapath_inst.branch_taken(branch_taken);
        datapath_inst.branch_target(branch_target);
        datapath_inst.raddr1(raddr1);
        datapath_inst.raddr2(raddr2);
        datapath_inst.waddr(waddr);
        datapath_inst.reg_write(reg_write);
        datapath_inst.alu_op(alu_op);
        datapath_inst.alu_srcB(alu_srcB);
        datapath_inst.mem_read(mem_read);
        datapath_inst.mem_write(mem_write);
        datapath_inst.mem_to_reg(mem_to_reg);
        datapath_inst.ir_write(ir_write);
        datapath_inst.instr(instr);
        datapath_inst.n_flag(n_flag);
        datapath_inst.z_flag(z_flag);
        datapath_inst.pc_out(pc_out);

        // Conexões da ControlUnit
        control_inst.clk(clk);
        control_inst.reset(reset);
        control_inst.instr(instr);
        control_inst.n_flag(n_flag);
        control_inst.z_flag(z_flag);
        control_inst.pc(pc_out);
        control_inst.pc_write(pc_write);
        control_inst.next_pc(next_pc);
        control_inst.raddr1(raddr1);
        control_inst.raddr2(raddr2);
        control_inst.waddr(waddr);
        control_inst.reg_write(reg_write);
        control_inst.alu_op(alu_op);
        control_inst.alu_srcB(alu_srcB);
        control_inst.mem_read(mem_read);
        control_inst.mem_write(mem_write);
        control_inst.mem_to_reg(mem_to_reg);
        control_inst.branch_taken(branch_taken);
        control_inst.branch_target(branch_target);
    }

    // Métodos de acesso
    void load_program(const sc_uint<16>* program, size_t size) {
        datapath_inst.load_program(program, size);
    }
    void initialize_dmem(const sc_uint<16>* data, size_t size) {
        datapath_inst.initialize_dmem(data, size);
    }
    sc_uint<16> read_dmem(sc_uint<8> addr) const {
        return datapath_inst.read_dmem(addr);
    }
    sc_uint<8> get_pc() const { return pc_out.read(); }
    sc_uint<16> get_instr() const { return instr.read(); }
    bool get_n_flag() const { return n_flag.read(); }
    bool get_z_flag() const { return z_flag.read(); }
};

#endif // PROCESSOR_H