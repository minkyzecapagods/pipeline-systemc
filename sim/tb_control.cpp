#include <systemc.h>
#include "control.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// ============================================================================
// Estrutura para armazenar um caso de teste
// ============================================================================
struct TestCase {
    std::string     name;               // Nome do teste
    sc_uint<16>     instr;              // Instrução
    bool            n_flag;             // Flag Negative
    bool            z_flag;             // Flag Zero
    sc_uint<8>      initial_pc;         // PC no início do FETCH

    // Valores esperados em cada estado (usamos -1 para "não se aplica")
    struct ExpectedState {
        ControlUnit::State state;
        bool        pc_write;
        sc_uint<8>  next_pc;
        sc_uint<4>  alu_op;
        bool        alu_srcB;
        bool        reg_write;
        bool        mem_read;
        bool        mem_write;
        bool        mem_to_reg;
        bool        branch_taken;
        sc_uint<8>  branch_target;
    };

    ExpectedState expected[5];  // até 5 estados (índice = ordem de ocorrência)
    int           num_steps;   // quantos estados são esperados
};

// ============================================================================
// Módulo de teste
// ============================================================================
SC_MODULE(ControlTester) {
    sc_clock            clk;
    sc_signal<bool>     reset;
    sc_signal<sc_uint<16>> instr;
    sc_signal<bool>     n_flag;
    sc_signal<bool>     z_flag;
    sc_signal<sc_uint<8>>  pc;

    sc_signal<bool>         pc_write;
    sc_signal<sc_uint<8>>   next_pc;
    sc_signal<sc_uint<3>>   raddr1, raddr2, waddr;
    sc_signal<bool>         reg_write;
    sc_signal<sc_uint<4>>   alu_op;
    sc_signal<bool>         alu_srcB;
    sc_signal<bool>         mem_read, mem_write;
    sc_signal<bool>         mem_to_reg;
    sc_signal<bool>         branch_taken;
    sc_signal<sc_uint<8>>   branch_target;

    ControlUnit* cu;

    // Contador de testes bem sucedidos
    int passed_tests;
    int total_tests;

    // ------------------------------------------------------------------------
    // Executa um caso de teste, verifica estado a estado
    // ------------------------------------------------------------------------
    bool run_test_case(const TestCase& tc) {
        std::cout << "\n>>> Testando: " << tc.name << " <<<\n";
        std::cout << "    Instrução: 0x" << std::hex << tc.instr << std::dec << "\n";
        std::cout << "    Flags iniciais: N=" << tc.n_flag << " Z=" << tc.z_flag << "\n";
        std::cout << "    PC inicial: " << (int)tc.initial_pc << "\n";

        // Configura entradas
        instr.write(tc.instr);
        n_flag.write(tc.n_flag);
        z_flag.write(tc.z_flag);
        pc.write(tc.initial_pc);

        // Aguarda um ciclo para estabilizar decodificação
        wait(1, SC_NS);

        bool test_passed = true;

        // Percorre os estados esperados
        for (int step = 0; step < tc.num_steps; step++) {
            const TestCase::ExpectedState& exp = tc.expected[step];
            
            // Espera até que o estado atual da FSM seja o esperado (ou timeout)
            int timeout = 20;
            while (cu->state.read() != exp.state && timeout-- > 0) {
                wait(clk.posedge_event());
                // Atualiza PC durante o teste (simula datapath)
                pc.write(tc.initial_pc); // simplificado, mas para verificação de saídas é suficiente
            }

            if (cu->state.read() != exp.state) {
                std::cout << "    ERRO: Timeout esperando estado " << exp.state << "\n";
                test_passed = false;
                break;
            }

            // Lê as saídas atuais
            bool        pc_w   = pc_write.read();
            sc_uint<8>  npc    = next_pc.read();
            sc_uint<4>  alu    = alu_op.read();
            bool        srcB   = alu_srcB.read();
            bool        rw     = reg_write.read();
            bool        mr     = mem_read.read();
            bool        mw     = mem_write.read();
            bool        mtr    = mem_to_reg.read();
            bool        bt     = branch_taken.read();
            sc_uint<8>  btgt   = branch_target.read();

            // Compara com esperado
            bool step_ok = true;
            std::ostringstream err_msg;

            auto check = [&](const std::string& sig, auto actual, auto expected) {
                if (actual != expected) {
                    err_msg << "  " << sig << ": esperado=" << expected << ", obtido=" << actual << "\n";
                    step_ok = false;
                }
            };

            check("pc_write", pc_w, exp.pc_write);
            if (exp.pc_write) check("next_pc", (int)npc, (int)exp.next_pc);
            check("alu_op", (int)alu, (int)exp.alu_op);
            check("alu_srcB", srcB, exp.alu_srcB);
            check("reg_write", rw, exp.reg_write);
            check("mem_read", mr, exp.mem_read);
            check("mem_write", mw, exp.mem_write);
            check("mem_to_reg", mtr, exp.mem_to_reg);
            check("branch_taken", bt, exp.branch_taken);
            if (exp.branch_taken) check("branch_target", (int)btgt, (int)exp.branch_target);

            if (!step_ok) {
                std::cout << "    FALHA no estado " << exp.state << ":\n" << err_msg.str();
                test_passed = false;
                break;
            }

            // Avança um ciclo para o próximo estado (exceto no último)
            if (step < tc.num_steps - 1) {
                wait(clk.posedge_event());
            }
        }

        if (test_passed) {
            std::cout << "    Resultado: PASSOU\n";
        } else {
            std::cout << "    Resultado: FALHOU\n";
        }

        // Reseta a UC para o próximo teste (força FETCH)
        reset.write(1);
        wait(2, SC_NS);
        reset.write(0);
        wait(1, SC_NS);

        return test_passed;
    }

    // ------------------------------------------------------------------------
    // Thread principal que dispara todos os testes
    // ------------------------------------------------------------------------
    void tester_thread() {
        // Reset inicial
        reset.write(1);
        wait(2, SC_NS);
        reset.write(0);
        wait(1, SC_NS);

        passed_tests = 0;
        total_tests = 0;

        // ----------------------------------------------------------------
        // Definição dos casos de teste com valores esperados
        // ----------------------------------------------------------------
        std::vector<TestCase> tests;

        // Teste 1: ADD R1, R2, R3
        {
            TestCase tc;
            tc.name = "ADD R1, R2, R3";
            tc.instr = 0x5498; // op=0101, rd=001, rs1=010, rs2=011
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 0;
            tc.num_steps = 4;
            tc.expected[0] = {ControlUnit::FETCH,    true, 1, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 5, false, false, false, false, false, false, 0};
            tc.expected[3] = {ControlUnit::WRITEBACK,false,0, 0, false, true,  false, false, false, false, 0};
            tests.push_back(tc);
        }

        // Teste 2: SUB R4, R5, R6
        {
            TestCase tc;
            tc.name = "SUB R4, R5, R6";
            tc.instr = 0x6568; // op=0110, rd=100, rs1=101, rs2=110
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 1;
            tc.num_steps = 4;
            tc.expected[0] = {ControlUnit::FETCH,    true, 2, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 6, false, false, false, false, false, false, 0};
            tc.expected[3] = {ControlUnit::WRITEBACK,false,0, 0, false, true,  false, false, false, false, 0};
            tests.push_back(tc);
        }

        // Teste 3: CMP R2, R3
        {
            TestCase tc;
            tc.name = "CMP R2, R3";
            tc.instr = 0x40A0; // op=0100, rd=000, rs1=010, rs2=011
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 2;
            tc.num_steps = 4;
            tc.expected[0] = {ControlUnit::FETCH,    true, 3, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 4, false, false, false, false, false, false, 0};
            tc.expected[3] = {ControlUnit::WRITEBACK,false,0, 0, false, false, false, false, false, false, 0};
            tests.push_back(tc);
        }

        // Teste 4: LD R1, [R2 + 10]
        {
            TestCase tc;
            tc.name = "LD R1, [R2 + 10]";
            tc.instr = 0x722A; // op=0111, rd=001, rs1=010, imm6=001010 (10)
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 3;
            tc.num_steps = 5;
            tc.expected[0] = {ControlUnit::FETCH,    true, 4, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 5, true,  false, false, false, false, false, 0};
            tc.expected[3] = {ControlUnit::MEM,      false,0, 0, false, false, true,  false, true,  false, 0};
            tc.expected[4] = {ControlUnit::WRITEBACK,false,0, 0, false, true,  false, false, true,  false, 0};
            tests.push_back(tc);
        }

        // Teste 5: ST R3, [R4 + 5]
        {
            TestCase tc;
            tc.name = "ST R3, [R4 + 5]";
            tc.instr = 0x8C85; // op=1000, rd=011, rs1=100, imm6=000101 (5)
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 4;
            tc.num_steps = 5;
            tc.expected[0] = {ControlUnit::FETCH,    true, 5, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 5, true,  false, false, false, false, false, 0};
            tc.expected[3] = {ControlUnit::MEM,      false,0, 0, false, false, false, true,  false, false, 0};
            tc.expected[4] = {ControlUnit::WRITEBACK,false,0, 0, false, false, false, false, false, false, 0};
            tests.push_back(tc);
        }

        // Teste 6: J +3
        {
            TestCase tc;
            tc.name = "J +3";
            tc.instr = 0x9003; // op=1001, imm9 = +3
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 5;
            tc.num_steps = 3;
            tc.expected[0] = {ControlUnit::FETCH,    true, 6, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  true, 8, 0, false, false, false, false, false, true,  8}; // PC=5 + 3 = 8
            tests.push_back(tc);
        }

        // Teste 7: JZ -2 com Z=1 (deve saltar)
        {
            TestCase tc;
            tc.name = "JZ -2 (Z=1)";
            tc.instr = 0xBFFE; // op=1011, imm9 = -2 (0x1FE em 9 bits)
            tc.n_flag = 0; tc.z_flag = 1;
            tc.initial_pc = 6;
            tc.num_steps = 3;
            tc.expected[0] = {ControlUnit::FETCH,    true, 7, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  true, 4, 0, false, false, false, false, false, true,  4}; // PC=6 + (-2) = 4
            tests.push_back(tc);
        }

        // Teste 8: JN -1 com N=0 (não deve saltar)
        {
            TestCase tc;
            tc.name = "JN -1 (N=0)";
            tc.instr = 0xAFFF; // op=1010, imm9 = -1 (0x1FF)
            tc.n_flag = 0; tc.z_flag = 0;
            tc.initial_pc = 7;
            tc.num_steps = 3;
            tc.expected[0] = {ControlUnit::FETCH,    true, 8, 0, false, false, false, false, false, false, 0};
            tc.expected[1] = {ControlUnit::DECODE,   false,0, 0, false, false, false, false, false, false, 0};
            tc.expected[2] = {ControlUnit::EXECUTE,  false,0, 0, false, false, false, false, false, false, 0}; // branch_taken = 0
            tests.push_back(tc);
        }

        // Executa todos os testes
        for (const auto& tc : tests) {
            total_tests++;
            if (run_test_case(tc))
                passed_tests++;
        }

        // Relatório final
        std::cout << "\n========================================\n";
        std::cout << "RESULTADO FINAL: " << passed_tests << " de " << total_tests << " testes passaram.\n";
        if (passed_tests == total_tests)
            std::cout << ">>> UNIDADE DE CONTROLE VALIDADA COM SUCESSO! <<<\n";
        else
            std::cout << ">>> ATENÇÃO: HÁ FALHAS NOS TESTES <<<\n";
        std::cout << "========================================\n";

        sc_stop();
    }

    SC_CTOR(ControlTester) : clk("clk", 10, SC_NS) {
        cu = new ControlUnit("CU");

        // Conexões
        cu->clk(clk);
        cu->reset(reset);
        cu->instr(instr);
        cu->n_flag(n_flag);
        cu->z_flag(z_flag);
        cu->pc(pc);

        cu->pc_write(pc_write);
        cu->next_pc(next_pc);
        cu->raddr1(raddr1);
        cu->raddr2(raddr2);
        cu->waddr(waddr);
        cu->reg_write(reg_write);
        cu->alu_op(alu_op);
        cu->alu_srcB(alu_srcB);
        cu->mem_read(mem_read);
        cu->mem_write(mem_write);
        cu->mem_to_reg(mem_to_reg);
        cu->branch_taken(branch_taken);
        cu->branch_target(branch_target);

        SC_THREAD(tester_thread);
    }

    ~ControlTester() {
        delete cu;
    }
};

int sc_main(int argc, char* argv[]) {
    ControlTester tester("ControlTester");

    // Geração de formas de onda (opcional)
    sc_trace_file* tf = sc_create_vcd_trace_file("control_waves");
    sc_trace(tf, tester.clk, "clk");
    sc_trace(tf, tester.reset, "reset");
    sc_trace(tf, tester.pc, "pc");
    sc_trace(tf, tester.instr, "instr");
    sc_trace(tf, tester.cu->state, "state");
    sc_trace(tf, tester.pc_write, "pc_write");
    sc_trace(tf, tester.reg_write, "reg_write");
    sc_trace(tf, tester.alu_op, "alu_op");
    sc_trace(tf, tester.mem_read, "mem_read");
    sc_trace(tf, tester.mem_write, "mem_write");
    sc_trace(tf, tester.branch_taken, "branch_taken");

    sc_start();

    sc_close_vcd_trace_file(tf);
    return 0;
}