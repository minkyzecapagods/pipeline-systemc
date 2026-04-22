/**
 * @file   tb_datapath.cpp
 * @brief  Testbench para validação do Datapath com Instruction Register (IR).
 *
 * Este testbench simula a execução manual de um pequeno programa (LD, ADD, ST, J)
 * aplicando diretamente os sinais de controle e verificando os resultados esperados.
 * Ele confirma que o Datapath está corretamente conectado e que os multiplexadores,
 * ALU, Register File e memórias funcionam conforme esperado.
 *
 * O programa carregado:
 *   Endereço 0: LD  R1, [R0 + 0]   (carrega o valor 5 da DMem[0] para R1)
 *   Endereço 1: ADD R2, R1, R1     (R2 = R1 + R1 = 10)
 *   Endereço 2: ST  R2, [R0 + 4]   (armazena 10 em DMem[4])
 *   Endereço 3: J   0              (salta para o endereço 0)
 */

#include <systemc.h>
#include "datapath.h"
#include <iomanip>

/**
 * @class DatapathTester
 * @brief Módulo de teste para o Datapath.
 *
 * Gera os estímulos de controle manualmente, ciclo a ciclo, e verifica
 * os valores internos do Datapath (PC, IR, barramentos, registradores e memória).
 */
SC_MODULE(DatapathTester) {
public:
    // Sinais de clock e reset
    sc_clock        clock;          ///< Clock do sistema (período 10 ns)
    sc_signal<bool> reset;          ///< Reset síncrono (ativo alto)

    // Sinais de controle (entradas do Datapath)
    sc_signal<bool>         pc_write;       ///< Habilita escrita no PC
    sc_signal<sc_uint<8>>   next_pc;        ///< Próximo PC sequencial (PC+1)
    sc_signal<bool>         branch_taken;   ///< Indica se um salto foi tomado
    sc_signal<sc_uint<8>>   branch_target;  ///< Endereço alvo do salto
    sc_signal<bool>         ir_write;       ///< Habilita carga do Instruction Register
    sc_signal<sc_uint<3>>   raddr1;         ///< Endereço de leitura 1 do Register File
    sc_signal<sc_uint<3>>   raddr2;         ///< Endereço de leitura 2 do Register File
    sc_signal<sc_uint<3>>   waddr;          ///< Endereço de escrita no Register File
    sc_signal<bool>         reg_write;      ///< Habilita escrita no Register File
    sc_signal<sc_uint<4>>   alu_op;         ///< Código da operação da ALU
    sc_signal<bool>         alu_srcB;       ///< Seleciona fonte do operando B (0=reg, 1=imm)
    sc_signal<bool>         mem_read;       ///< Habilita leitura da memória de dados
    sc_signal<bool>         mem_write;      ///< Habilita escrita na memória de dados
    sc_signal<bool>         mem_to_reg;     ///< Seleciona dado de write‑back (0=ALU, 1=Mem)

    // Saídas do Datapath (para observação e verificação)
    sc_signal<sc_uint<16>>  instr;          ///< Instrução lida da IMem
    sc_signal<bool>         n_flag;         ///< Flag Negative da ALU
    sc_signal<bool>         z_flag;         ///< Flag Zero da ALU
    sc_signal<sc_uint<8>>   pc_out;         ///< Valor atual do PC

    // Ponteiro para o Datapath sob teste
    Datapath* dp;

    // Contadores de testes
    int passed;     ///< Número de verificações bem‑sucedidas
    int total;      ///< Número total de verificações realizadas

    // Métodos auxiliares

    /**
     * @brief Avança um ciclo de clock e aguarda estabilização combinacional.
     */
    void tick() {
        wait(clock.posedge_event());
        wait(1, SC_NS);     // pequeno delay para propagação dos sinais
    }

    /**
     * @brief Aplica um reset ao Datapath e exibe o valor do PC após o reset.
     */
    void do_reset() {
        reset.write(1);
        tick();
        reset.write(0);
        tick();
        std::cout << "[Reset] PC = " << static_cast<int>(pc_out.read()) << "\n";
    }

    /**
     * @brief Verifica se um valor obtido é igual ao esperado e atualiza os contadores.
     * @tparam T      Tipo do valor a ser comparado.
     * @param test    Nome do teste atual (ex.: "LD FETCH").
     * @param sig     Nome do sinal sendo verificado.
     * @param actual  Valor lido do Datapath.
     * @param expected Valor esperado.
     */
    template<typename T>
    void check(const std::string& test, const std::string& sig, T actual, T expected) {
        total++;
        if (actual == expected) {
            passed++;
        } else {
            std::cout << "  FALHA em " << test << " - " << sig
                      << ": esperado=" << expected << ", obtido=" << actual << std::endl;
        }
    }

    // Thread principal de teste
    void test_thread() {
        passed = total = 0;
        do_reset();
        std::cout << "\n=== Teste do Datapath com IR ===\n\n";

        // ----------------------------------------------------------------
        // Instrução 0: LD R1, [R0 + 0]   (opcode = 0x7, rd = 1, rs1 = 0, imm6 = 0)
        // ----------------------------------------------------------------
        std::cout << "--- LD R1, [R0+0] ---\n";

        // FETCH: PC = 0, carrega IR com a instrução do endereço 0
        pc_write.write(true);
        next_pc.write(1);               // PC ← PC + 1
        branch_taken.write(false);
        ir_write.write(true);           // habilita escrita no IR
        wait(1, SC_NS);                 // estabiliza leitura da IMem
        check("LD FETCH", "Instr (IMem)", instr.read(), sc_uint<16>(0x7200));
        tick();                         // borda do clock → PC=1, IR carregado
        ir_write.write(false);
        check("LD FETCH", "PC", pc_out.read(), sc_uint<8>(1));
        check("LD FETCH", "IR out", dp->ir_out.read(), sc_uint<16>(0x7200));

        // DECODE: lê R0 (rs1) e rs2 (não usado)
        raddr1.write(0);
        raddr2.write(0);
        pc_write.write(false);
        tick();

        // EXECUTE: ADD para calcular endereço (R0 + 0)
        alu_op.write(5);                // ADD
        alu_srcB.write(true);           // usa imediato (imm6 = 0)
        raddr1.write(0);
        raddr2.write(0);
        tick();
        check("LD EXECUTE", "ALU result", dp->alu_result.read(), sc_uint<16>(0));

        // MEM: lê DMem[0] (valor 5)
        mem_read.write(true);
        mem_write.write(false);
        tick();
        check("LD MEM", "DMem rdata", dp->dmem_rdata.read(), sc_uint<16>(5));

        // WRITEBACK: escreve em R1
        reg_write.write(true);
        mem_to_reg.write(true);         // write‑back vem da memória
        waddr.write(1);
        tick();
        check("LD WB", "R1", dp->rf->registers[1].read(), sc_uint<16>(5));

        // Limpa sinais de controle para a próxima instrução
        reg_write.write(false);
        mem_read.write(false);
        mem_to_reg.write(false);

        // ----------------------------------------------------------------
        // Instrução 1: ADD R2, R1, R1   (opcode = 0x5, rd = 2, rs1 = 1, rs2 = 1)
        // ----------------------------------------------------------------
        std::cout << "\n--- ADD R2, R1, R1 ---\n";

        // FETCH
        pc_write.write(true);
        next_pc.write(2);
        ir_write.write(true);
        wait(1, SC_NS);
        check("ADD FETCH", "Instr (IMem)", instr.read(), sc_uint<16>(0x5448));
        tick();
        ir_write.write(false);
        check("ADD FETCH", "PC", pc_out.read(), sc_uint<8>(2));
        check("ADD FETCH", "IR out", dp->ir_out.read(), sc_uint<16>(0x5448));

        // DECODE: lê R1 em ambos os barramentos
        raddr1.write(1);
        raddr2.write(1);
        pc_write.write(false);
        tick();
        check("ADD DECODE", "busA", dp->rf_rdata1.read(), sc_uint<16>(5));
        check("ADD DECODE", "busB", dp->rf_rdata2.read(), sc_uint<16>(5));

        // EXECUTE: ADD (registrador)
        alu_op.write(5);
        alu_srcB.write(false);          // usa registrador (busB)
        tick();
        check("ADD EXECUTE", "ALU result", dp->alu_result.read(), sc_uint<16>(10));

        // WRITEBACK: escreve em R2
        reg_write.write(true);
        mem_to_reg.write(false);        // write‑back vem da ALU
        waddr.write(2);
        tick();
        check("ADD WB", "R2", dp->rf->registers[2].read(), sc_uint<16>(10));
        reg_write.write(false);

        // ----------------------------------------------------------------
        // Instrução 2: ST R2, [R0 + 4]  (opcode = 0x8, rd = 2, rs1 = 0, imm6 = 4)
        // ----------------------------------------------------------------
        std::cout << "\n--- ST R2, [R0+4] ---\n";

        // FETCH
        pc_write.write(true);
        next_pc.write(3);
        ir_write.write(true);
        wait(1, SC_NS);
        check("ST FETCH", "Instr (IMem)", instr.read(), sc_uint<16>(0x8204));
        tick();
        ir_write.write(false);
        check("ST FETCH", "PC", pc_out.read(), sc_uint<8>(3));
        check("ST FETCH", "IR out", dp->ir_out.read(), sc_uint<16>(0x8204));

        // DECODE: rs1 = R0 (base), rs2 = R2 (dado a armazenar)
        raddr1.write(0);
        raddr2.write(2);
        pc_write.write(false);
        tick();

        // EXECUTE: ADD com imediato 4 (cálculo do endereço)
        alu_op.write(5);
        alu_srcB.write(true);           // usa imediato
        tick();
        check("ST EXECUTE", "ALU result", dp->alu_result.read(), sc_uint<16>(4));

        // MEM: escrita na DMem[4]
        mem_write.write(true);
        tick();                         // borda do clock: memória escrita
        tick();                         // estabiliza leitura
        check("ST MEM", "DMem[4]", dp->dmem->memory[4].read(), sc_uint<16>(10));

        mem_write.write(false);
        reg_write.write(false);
        tick();

        // ----------------------------------------------------------------
        // Instrução 3: J 0               (opcode = 0x9, imm9 = 0)
        // ----------------------------------------------------------------
        std::cout << "\n--- J 0 ---\n";

        // FETCH
        pc_write.write(true);
        next_pc.write(4);
        ir_write.write(true);
        wait(1, SC_NS);
        check("J FETCH", "Instr (IMem)", instr.read(), sc_uint<16>(0x9000));
        tick();
        ir_write.write(false);
        check("J FETCH", "PC", pc_out.read(), sc_uint<8>(4));
        check("J FETCH", "IR out", dp->ir_out.read(), sc_uint<16>(0x9000));

        // Salto tomado: a Unidade de Controle geraria branch_taken=1 e target=0
        branch_taken.write(true);
        branch_target.write(0);
        tick();
        check("J Branch", "PC", pc_out.read(), sc_uint<8>(0));

        branch_taken.write(false);
        tick();

        // Relatório final
        std::cout << "\n========================================\n";
        std::cout << "Verificações: " << passed << " de " << total << " passaram.\n";
        if (passed == total) {
            std::cout << ">>> DATAPATH VALIDADO COM SUCESSO! <<<\n";
        } else {
            std::cout << ">>> HÁ FALHAS NO DATAPATH <<<\n";
        }
        std::cout << "========================================\n";

        sc_stop();
    }

    // Construtor
    SC_CTOR(DatapathTester) : clock("clk", 10, SC_NS) {
        // Instancia o Datapath
        dp = new Datapath("DP");

        // Conexões de clock e reset
        dp->clk(clock);
        dp->reset(reset);

        // Conexões dos sinais de controle
        dp->pc_write(pc_write);
        dp->next_pc(next_pc);
        dp->branch_taken(branch_taken);
        dp->branch_target(branch_target);
        dp->ir_write(ir_write);
        dp->raddr1(raddr1);
        dp->raddr2(raddr2);
        dp->waddr(waddr);
        dp->reg_write(reg_write);
        dp->alu_op(alu_op);
        dp->alu_srcB(alu_srcB);
        dp->mem_read(mem_read);
        dp->mem_write(mem_write);
        dp->mem_to_reg(mem_to_reg);

        // Conexões das saídas (para monitoramento)
        dp->instr(instr);
        dp->n_flag(n_flag);
        dp->z_flag(z_flag);
        dp->pc_out(pc_out);

        // Lança a thread de teste
        SC_THREAD(test_thread);
    }

    // Destrutor
    ~DatapathTester() {
        delete dp;
    }
};

// Função principal (sc_main)
int sc_main(int argc, char* argv[]) {
    DatapathTester tester("DatapathTester");

    // Inicialização das memórias (ANTES do início da simulação)
    // Memória de instruções (programa)
    tester.dp->imem->memory[0] = 0x7200;   // LD  R1, [R0+0]
    tester.dp->imem->memory[1] = 0x5448;   // ADD R2, R1, R1
    tester.dp->imem->memory[2] = 0x8204;   // ST  R2, [R0+4]
    tester.dp->imem->memory[3] = 0x9000;   // J   0

    // Memória de dados (valores iniciais)
    tester.dp->dmem->memory[0].write(5);   // DMem[0] = 5
    tester.dp->dmem->memory[4].write(0);   // será sobrescrito pelo ST

    // Geração de arquivo VCD para depuração
    sc_trace_file* tf = sc_create_vcd_trace_file("waves/datapath_waves");
    sc_trace(tf, tester.clock, "clk");
    sc_trace(tf, tester.reset, "reset");
    sc_trace(tf, tester.pc_out, "PC");
    sc_trace(tf, tester.instr, "Instr_IMem");
    sc_trace(tf, tester.dp->ir_out, "IR_out");
    sc_trace(tf, tester.dp->rf_rdata1, "busA");
    sc_trace(tf, tester.dp->rf_rdata2, "busB");
    sc_trace(tf, tester.dp->alu_result, "ALU_result");
    sc_trace(tf, tester.dp->dmem_rdata, "DMem_rdata");
    sc_trace(tf, tester.dp->wb_data, "WB_data");

    for (int i = 0; i < 8; i++) {
        sc_trace(tf, tester.dp->rf->registers[i], std::string("R") + std::to_string(i));
    }
    for (int i = 0; i < 8; i++) {
        sc_trace(tf, tester.dp->dmem->memory[i], std::string("dmem") + std::to_string(i));
    }

    // Início da simulação
    sc_start();

    // Fechamento do arquivo VCD
    sc_close_vcd_trace_file(tf);

    return 0;
}