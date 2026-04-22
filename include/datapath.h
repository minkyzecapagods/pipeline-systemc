/**
 * @file   datapath.h
 * @brief  Parte Operativa do processador RISC 16‑bit.
 * @details Integra os componentes: Register File, ALU, Program Counter,
 *          Memória de Instruções e Memória de Dados. Inclui multiplexadores
 *          para seleção do operando B da ALU, próximo PC e dado de write‑back.
 */

#ifndef DATAPATH_H
#define DATAPATH_H

#include <systemc.h>
#include "regfile.h"
#include "alu.h"
#include "pc.h"
#include "imem.h"
#include "dmem.h"
#include "ir.h"

/**
 * @class Datapath
 * @brief Parte Operativa (PO) do processador.
 *
 * Recebe os sinais de controle da Unidade de Controle e executa as operações
 * sobre os dados. Todas as conexões entre submódulos são feitas internamente.
 */
SC_MODULE(Datapath) {
public:
    // ========================================================================
    // Portas de Entrada (sinais de controle)
    // ========================================================================
    sc_in<bool>         clk;            ///< Clock do sistema
    sc_in<bool>         reset;          ///< Reset síncrono (ativo alto)

    // Controle do Program Counter
    sc_in<bool>         pc_write;       ///< Habilita escrita no PC
    sc_in<sc_uint<8>>   next_pc;        ///< Próximo PC sequencial (PC+1)
    sc_in<bool>         branch_taken;   ///< Indica se salto foi tomado
    sc_in<sc_uint<8>>   branch_target;  ///< Endereço alvo do salto

    // Controle do Register File
    sc_in<sc_uint<3>>   raddr1;         ///< Endereço de leitura 1
    sc_in<sc_uint<3>>   raddr2;         ///< Endereço de leitura 2
    sc_in<sc_uint<3>>   waddr;          ///< Endereço de escrita
    sc_in<bool>         reg_write;      ///< Habilita escrita no RF

    // Controle da ALU
    sc_in<sc_uint<4>>   alu_op;         ///< Código da operação da ALU
    sc_in<bool>         alu_srcB;       ///< Seleciona operando B (0=reg, 1=imm)

    // Controle da Memória de Dados
    sc_in<bool>         mem_read;       ///< Habilita leitura da DMem (não usado diretamente)
    sc_in<bool>         mem_write;      ///< Habilita escrita na DMem
    sc_in<bool>         mem_to_reg;     ///< Seleciona dado de write‑back (0=ALU, 1=Mem)

    sc_in<bool>         ir_write;

    // ========================================================================
    // Portas de Saída (enviadas para a Unidade de Controle)
    // ========================================================================
    sc_out<sc_uint<16>> instr;          ///< Instrução lida da IMem
    sc_out<bool>        n_flag;         ///< Flag Negative da ALU
    sc_out<bool>        z_flag;         ///< Flag Zero da ALU
    sc_out<sc_uint<8>>  pc_out;         ///< Valor atual do PC

    // ========================================================================
    // Sinais internos para interconexão
    // ========================================================================
    // Register File
    sc_signal<sc_uint<16>> rf_rdata1;   ///< Saída de leitura 1 (busA)
    sc_signal<sc_uint<16>> rf_rdata2;   ///< Saída de leitura 2 (busB)

    // ALU
    sc_signal<sc_uint<16>> alu_result;      ///< Resultado da ALU
    sc_signal<sc_uint<16>> alu_operand_b;   ///< Segundo operando da ALU (mux)

    // PC
    sc_signal<sc_uint<8>>  pc_value;        ///< Valor interno do PC
    sc_signal<sc_uint<8>>  pc_next_muxed;   ///< Saída do mux de próximo PC

    // DMem
    sc_signal<sc_uint<16>> dmem_rdata;      ///< Dado lido da memória de dados
    sc_signal<sc_uint<8>>  dmem_addr;       ///< Endereço para DMem (8 bits LSB da ALU)

    // Extensão de imediato
    sc_signal<sc_uint<16>> imm_extended;    ///< Imediato de 6 bits zero‑estendido

    // Write‑back
    sc_signal<sc_uint<16>> wb_data;         ///< Dado a ser escrito no RF (mux)

    sc_signal<sc_uint<16>> ir_out;

    // ========================================================================
    // Submódulos
    // ========================================================================
    RegFile*        rf;
    ALU*            alu;
    ProgramCounter* pc;
    IMem*           imem;
    DMem*           dmem;
    InstructionRegister* ir;

    // ========================================================================
    // Métodos Combinacionais
    // ========================================================================

    /**
     * @brief Extensão de imediato.
     * Extrai os 6 bits menos significativos da instrução e os estende com zeros.
     */
    void immediate_extend() {
        sc_uint<16> ir_val = ir_out.read();
        sc_uint<6> imm6 = ir_val.range(5, 0);
        imm_extended.write(imm6);
    }

    /**
     * @brief Mux para seleção do próximo PC.
     * Se branch_taken = 1, usa branch_target; senão, usa next_pc (PC+1).
     */
    void mux_pc_next() {
        if (branch_taken.read()) {
            pc_next_muxed.write(branch_target.read());
        } else {
            pc_next_muxed.write(next_pc.read());
        }
    }

    /**
     * @brief Mux para seleção do segundo operando da ALU.
     * Se alu_srcB = 1, usa imediato estendido; senão, usa rf_rdata2 (busB).
     */
    void mux_alu_srcB() {
        if (alu_srcB.read()) {
            alu_operand_b.write(imm_extended.read());
        } else {
            alu_operand_b.write(rf_rdata2.read());
        }
    }

    /**
     * @brief Geração do endereço da memória de dados.
     * Utiliza os 8 bits menos significativos do resultado da ALU.
     */
    void drive_mem_addr() {
        dmem_addr.write(alu_result.read().range(7, 0));
    }

    /**
     * @brief Mux para seleção do dado de write‑back.
     * Se mem_to_reg = 1, usa dado lido da DMem; senão, usa resultado da ALU.
     */
    void mux_writeback() {
        if (mem_to_reg.read()) {
            wb_data.write(dmem_rdata.read());
        } else {
            wb_data.write(alu_result.read());
        }
    }

    /**
     * @brief Propaga o valor interno do PC para a saída pc_out.
     * Evita múltiplos drivers no sinal.
     */
    void drive_pc_out() {
        pc_out.write(pc_value.read());
    }

    // ========================================================================
    // Construtor
    // ========================================================================
    SC_CTOR(Datapath) {
        // Instanciação dos submódulos
        rf   = new RegFile("RF");
        alu  = new ALU("ALU");
        pc   = new ProgramCounter("PC");
        imem = new IMem("IMEM");
        dmem = new DMem("DMEM");

        // --------------------------------------------------------------------
        // Conexões do Program Counter
        // --------------------------------------------------------------------
        pc->clk(clk);
        pc->reset(reset);
        pc->pc_write(pc_write);
        pc->next_pc(pc_next_muxed);
        pc->pc(pc_value);
        // pc_out é dirigido pelo método drive_pc_out (sem conflito de drivers)

        // --------------------------------------------------------------------
        // Conexões da Memória de Instruções
        // --------------------------------------------------------------------
        imem->address(pc_value);
        imem->instruction(instr);

        // --------------------------------------------------------------------
        // Conexões do Register File
        // --------------------------------------------------------------------
        rf->clock(clk);
        rf->read_addr1(raddr1);
        rf->read_addr2(raddr2);
        rf->write_addr(waddr);
        rf->write_enable(reg_write);
        rf->write_data(wb_data);
        rf->read_data1(rf_rdata1);
        rf->read_data2(rf_rdata2);

        // --------------------------------------------------------------------
        // Conexões da ALU
        // --------------------------------------------------------------------
        alu->operand_a(rf_rdata1);
        alu->operand_b(alu_operand_b);
        alu->operation_code(alu_op);
        alu->result(alu_result);
        alu->negative_flag(n_flag);
        alu->zero_flag(z_flag);

        // --------------------------------------------------------------------
        // Conexões da Memória de Dados
        // --------------------------------------------------------------------
        dmem->clock(clk);
        dmem->address(dmem_addr);
        dmem->write_data(rf_rdata2);      // Em ST, o dado vem de rs2
        dmem->write_enable(mem_write);
        dmem->read_data(dmem_rdata);
        // O sinal mem_read não é usado diretamente, pois a leitura é combinacional.

               ir = new InstructionRegister("IR");
        ir->clk(clk);
        ir->reset(reset);
        ir->write_enable(ir_write);
        ir->data_in(instr);
        ir->data_out(ir_out);
        
        // --------------------------------------------------------------------
        // Registro dos processos combinacionais
        // --------------------------------------------------------------------
        SC_METHOD(immediate_extend);
        sensitive << ir_out;   // <-- sensível a ir_out, não a instr

        SC_METHOD(mux_pc_next);
        sensitive << next_pc << branch_target << branch_taken;

        SC_METHOD(mux_alu_srcB);
        sensitive << rf_rdata2 << imm_extended << alu_srcB;

        SC_METHOD(drive_mem_addr);
        sensitive << alu_result;

        SC_METHOD(mux_writeback);
        sensitive << alu_result << dmem_rdata << mem_to_reg;

        SC_METHOD(drive_pc_out);
        sensitive << pc_value;
    }

    // ========================================================================
    // Destrutor
    // ========================================================================
    ~Datapath() {
        delete rf;
        delete alu;
        delete pc;
        delete imem;
        delete dmem;
        delete ir;
    }
    // Métodos para acesso às memórias (uso pelo testbench)
    void load_program(const sc_uint<16>* program, size_t size) {
        for (size_t i = 0; i < size && i < 256; ++i) {
            imem->memory[i] = program[i];
        }
    }

    void initialize_dmem(const sc_uint<16>* data, size_t size) {
        for (size_t i = 0; i < size && i < 256; ++i) {
            dmem->memory[i].write(data[i]);
        }
    }

    sc_uint<16> read_dmem(sc_uint<8> addr) const {
        return dmem->memory[addr].read();
    }
};

#endif // DATAPATH_H