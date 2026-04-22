#ifndef CONTROL_H
#define CONTROL_H

#include <systemc.h>

/**
 * @file   control.h
 * @brief  Unidade de Controle do processador RISC 16-bit.
 * @details Implementa uma FSM de 5 estágios que decodifica as instruções
 *          e gera os sinais de controle para o datapath.
 */

/**
 * @class ControlUnit
 * @brief Unidade de Controle baseada em máquina de estados finitos (FSM).
 * 
 * A FSM possui os seguintes estados:
 * - FETCH    : Busca da instrução na memória de programa.
 * - DECODE   : Decodificação e leitura dos registradores fonte.
 * - EXECUTE  : Execução da operação na ALU ou cálculo de endereço/salto.
 * - MEM      : Acesso à memória de dados (apenas para LD e ST).
 * - WRITEBACK: Escrita do resultado no banco de registradores.
 * 
 * As instruções suportadas seguem o formato definido no projeto.
 */
SC_MODULE(ControlUnit) {
public:
    // ========================================================================
    // Portas de Entrada
    // ========================================================================
    sc_in<bool>         clk;            ///< Clock do sistema
    sc_in<bool>         reset;          ///< Reset síncrono (ativo em alto)
    sc_in<sc_uint<16>>  instr;          ///< Instrução atual (16 bits)
    sc_in<bool>         n_flag;         ///< Flag Negative (resultado < 0)
    sc_in<bool>         z_flag;         ///< Flag Zero (resultado == 0)
    sc_in<sc_uint<8>>   pc;             ///< Program Counter atual

    // ========================================================================
    // Portas de Saída (Sinais de Controle para o Datapath)
    // ========================================================================
    sc_out<bool>        pc_write;       ///< Habilita atualização do PC
    sc_out<sc_uint<8>>  next_pc;        ///< Próximo valor do PC
    sc_out<sc_uint<3>>  raddr1;         ///< Endereço de leitura 1 do Register File
    sc_out<sc_uint<3>>  raddr2;         ///< Endereço de leitura 2 do Register File
    sc_out<sc_uint<3>>  waddr;          ///< Endereço de escrita no Register File
    sc_out<bool>        reg_write;      ///< Habilita escrita no Register File
    sc_out<sc_uint<4>>  alu_op;         ///< Código da operação da ALU
    sc_out<bool>        alu_srcB;       ///< Seleciona fonte do operando B (0=reg, 1=imm)
    sc_out<bool>        mem_read;       ///< Habilita leitura da memória de dados
    sc_out<bool>        mem_write;      ///< Habilita escrita na memória de dados
    sc_out<bool>        mem_to_reg;     ///< Seleciona dado para write-back (0=ALU, 1=Mem)
    sc_out<bool>        branch_taken;   ///< Indica que um salto foi tomado
    sc_out<sc_uint<8>>  branch_target;  ///< Endereço alvo do salto

    // ========================================================================
    // Estados da FSM (públicos para monitoramento/testbench)
    // ========================================================================
    enum State {
        FETCH,      ///< Busca da instrução
        DECODE,     ///< Decodificação e leitura de registradores
        EXECUTE,    ///< Execução na ALU / Cálculo de endereço / Resolução de salto
        MEM,        ///< Acesso à memória de dados (LD/ST)
        WRITEBACK   ///< Escrita do resultado no Register File
    };

    sc_signal<State>    state;          ///< Estado atual da FSM
    sc_signal<State>    next_state;     ///< Próximo estado (calculado combinatorialmente)

private:
    // ========================================================================
    // Códigos de Operação (Opcode) - 4 bits mais significativos
    // (Usamos unsigned int para permitir uso em switch/case)
    // ========================================================================
    static const unsigned int OP_AND = 0b0000;
    static const unsigned int OP_OR  = 0b0001;
    static const unsigned int OP_XOR = 0b0010;
    static const unsigned int OP_NOT = 0b0011;
    static const unsigned int OP_CMP = 0b0100;
    static const unsigned int OP_ADD = 0b0101;
    static const unsigned int OP_SUB = 0b0110;
    static const unsigned int OP_LD  = 0b0111;
    static const unsigned int OP_ST  = 0b1000;
    static const unsigned int OP_J   = 0b1001;
    static const unsigned int OP_JN  = 0b1010;
    static const unsigned int OP_JZ  = 0b1011;

    // Códigos de Operação da ALU (valores enviados ao sinal alu_op)
    static const unsigned int ALU_AND = 0;
    static const unsigned int ALU_OR  = 1;
    static const unsigned int ALU_XOR = 2;
    static const unsigned int ALU_NOT = 3;
    static const unsigned int ALU_CMP = 4;
    static const unsigned int ALU_ADD = 5;
    static const unsigned int ALU_SUB = 6;

    // Campos decodificados da instrução (extraídos em decode_fields())
    sc_uint<4> opcode;      ///< Opcode (bits 15..12)
    sc_uint<3> rd;          ///< Registrador destino (bits 11..9)
    sc_uint<3> rs1;         ///< Registrador fonte 1 (bits 8..6)
    sc_uint<3> rs2;         ///< Registrador fonte 2 (bits 5..3)
    sc_uint<6> imm6;        ///< Imediato de 6 bits (bits 5..0)
    sc_int<9>  imm9;        ///< Imediato de 9 bits com sinal (bits 8..0) para saltos

    /**
     * @brief Decodifica os campos da instrução atual.
     */
    void decode_fields() {
        const sc_uint<16> ir = instr.read();
        opcode = ir.range(15, 12);
        rd     = ir.range(11, 9);
        rs1    = ir.range(8, 6);
        rs2    = ir.range(5, 3);
        imm6   = ir.range(5, 0);
        imm9   = static_cast<sc_int<9>>(ir.range(8, 0));
    }

    /**
     * @brief Atualiza o estado da FSM na borda de subida do clock.
     */
    void fsm_transition() {
        if (reset.read()) {
            state.write(FETCH);
        } else {
            state.write(next_state.read());
        }
    }

    /**
     * @brief Gera os sinais de controle e o próximo estado.
     */
    void fsm_output() {
        reset_control_signals();

        const State curr = state.read();
        const sc_uint<8> current_pc = pc.read();

        switch (curr) {
            case FETCH:     handle_fetch(current_pc);      break;
            case DECODE:    handle_decode();               break;
            case EXECUTE:   handle_execute(current_pc);    break;
            case MEM:       handle_memory();               break;
            case WRITEBACK: handle_writeback();            break;
        }
    }

    /**
     * @brief Reinicia todos os sinais de controle para seus valores inativos.
     */
    void reset_control_signals() {
        pc_write.write(false);
        next_pc.write(0);
        raddr1.write(0);
        raddr2.write(0);
        waddr.write(0);
        reg_write.write(false);
        alu_op.write(0);
        alu_srcB.write(false);
        mem_read.write(false);
        mem_write.write(false);
        mem_to_reg.write(false);
        branch_taken.write(false);
        branch_target.write(0);
    }

    /**
     * @brief Lógica do estágio FETCH.
     */
    void handle_fetch(sc_uint<8> current_pc) {
        pc_write.write(true);
        next_pc.write(current_pc + 1);      // PC ← PC + 1
        next_state.write(DECODE);
    }

    /**
     * @brief Lógica do estágio DECODE.
     */
    void handle_decode() {
        raddr1.write(rs1);
        raddr2.write(rs2);
        next_state.write(EXECUTE);
    }

    /**
     * @brief Lógica do estágio EXECUTE.
     */
    void handle_execute(sc_uint<8> current_pc) {
        raddr1.write(rs1);
        raddr2.write(rs2);

        if (is_branch_instruction()) {
            handle_branch(current_pc);
            return;
        }

        configure_alu_for_instruction();

        if (is_memory_instruction()) {
            next_state.write(MEM);
        } else {
            next_state.write(WRITEBACK);
        }
    }

    /**
     * @brief Lógica do estágio MEM.
     */
    void handle_memory() {
        raddr1.write(rs1);
        if (opcode.to_uint() == OP_LD) {
            mem_read.write(true);
            mem_to_reg.write(true);
        } else if (opcode.to_uint() == OP_ST) {
            mem_write.write(true);
            raddr2.write(rd);
        }
        next_state.write(WRITEBACK);
    }

    /**
     * @brief Lógica do estágio WRITEBACK.
     */
    void handle_writeback() {
        if (opcode.to_uint() == OP_LD) {
            mem_to_reg.write(true);
        }
        if (produces_register_result()) {
            reg_write.write(true);
            waddr.write(rd);
        }
        next_state.write(FETCH);
    }

    // Funções Auxiliares de Suporte
    bool is_branch_instruction() const {
        unsigned int op = opcode.to_uint();
        return (op == OP_J) || (op == OP_JN) || (op == OP_JZ);
    }

    bool is_memory_instruction() const {
        unsigned int op = opcode.to_uint();
        return (op == OP_LD) || (op == OP_ST);
    }

    bool produces_register_result() const {
        unsigned int op = opcode.to_uint();
        return (op != OP_CMP) && (op != OP_ST) && !is_branch_instruction();
    }

    void handle_branch(sc_uint<8> current_pc) {
        bool take_branch = false;
        unsigned int op = opcode.to_uint();

        if (op == OP_J) {
            take_branch = true;
        } else if (op == OP_JN) {
            take_branch = n_flag.read();
        } else if (op == OP_JZ) {
            take_branch = z_flag.read();
        }

        if (take_branch) {
            const sc_uint<8> target = current_pc + imm9;
            branch_taken.write(true);
            branch_target.write(target);
            pc_write.write(true);
            next_pc.write(target);
        }
        next_state.write(FETCH);
    }

    void configure_alu_for_instruction() {
        unsigned int op = opcode.to_uint();
        switch (op) {
            case OP_AND: alu_op.write(ALU_AND); alu_srcB.write(false); break;
            case OP_OR:  alu_op.write(ALU_OR);  alu_srcB.write(false); break;
            case OP_XOR: alu_op.write(ALU_XOR); alu_srcB.write(false); break;
            case OP_NOT: alu_op.write(ALU_NOT); alu_srcB.write(false); break;
            case OP_CMP: alu_op.write(ALU_CMP); alu_srcB.write(false); break;
            case OP_ADD: alu_op.write(ALU_ADD); alu_srcB.write(false); break;
            case OP_SUB: alu_op.write(ALU_SUB); alu_srcB.write(false); break;
            case OP_LD:
            case OP_ST:  alu_op.write(ALU_ADD); alu_srcB.write(true);  break;
            default:     alu_op.write(0);       alu_srcB.write(false); break;
        }
    }

public:
    SC_CTOR(ControlUnit) {
        SC_METHOD(decode_fields);
        sensitive << instr;

        SC_METHOD(fsm_transition);
        sensitive << clk.pos() << reset;

        SC_METHOD(fsm_output);
        sensitive << state << instr << n_flag << z_flag << pc;
    }
};

#endif // CONTROL_H