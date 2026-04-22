/**
 * @file   tb_pc.cpp
 * @brief  Testbench para o contador de programa (ProgramCounter) de 8 bits.
 *
 * Aplica estímulos para verificar as funcionalidades de reset síncrono,
 * carga condicional (pc_write) e manutenção de estado.
 * Gera relatório no console e formas de onda VCD opcionais.
 */

#include <systemc.h>
#include "pc.h"
#include <iomanip>
#include <iostream>

using namespace std;

/**
 * @class PC_Tester
 * @brief Módulo de teste para o ProgramCounter.
 */
SC_MODULE(PC_Tester) {
private:
    // Sinais para conexão com o DUT
    sc_signal<bool>        reset_sig;
    sc_signal<bool>        pc_write_sig;
    sc_signal<sc_uint<8>>  next_pc_sig;
    sc_signal<sc_uint<8>>  pc_sig;
    sc_clock               clk_sig;

    ProgramCounter* dut;

    /**
     * @brief Atraso para estabilização da lógica combinacional (se houvesse).
     * Neste módulo puramente síncrono, não é estritamente necessário, mas
     * mantemos para consistência com outros testbenches.
     */
    static const sc_time PROPAGATION_DELAY;

    /**
     * @brief Aguarda uma borda de subida do clock.
     */
    void wait_cycle() {
        wait(clk_sig.posedge_event());
    }

    /**
     * @brief Aplica valores de entrada e aguarda a próxima borda do clock.
     * @param reset Valor para o sinal de reset.
     * @param write Habilitação de escrita (pc_write).
     * @param next Valor para next_pc.
     */
    void apply_inputs(bool reset, bool write, sc_uint<8> next) {
        reset_sig.write(reset);
        pc_write_sig.write(write);
        next_pc_sig.write(next);
        wait_cycle();
        wait(PROPAGATION_DELAY); // pequeno atraso para estabilidade da saída
    }

    /**
     * @brief Verifica se a saída atual do PC corresponde ao valor esperado.
     * @param expected Valor esperado para pc.
     * @param desc Descrição textual do teste.
     * @return true se o teste passou.
     */
    bool check_pc(sc_uint<8> expected, const string& desc) {
        sc_uint<8> actual = pc_sig.read();
        bool pass = (actual == expected);

        cout << "@" << sc_time_stamp() << " " << desc << endl;
        cout << "  Entradas: reset=" << reset_sig.read()
             << ", pc_write=" << pc_write_sig.read()
             << ", next_pc=0x" << hex << setw(2) << setfill('0')
             << next_pc_sig.read().to_uint() << dec << endl;
        cout << "  PC = 0x" << hex << setw(2) << setfill('0')
             << actual.to_uint() << dec;
        if (!pass) {
            cout << " (ESPERADO: 0x" << hex << setw(2) << setfill('0')
                 << expected.to_uint() << dec << ")";
        }
        cout << " -> " << (pass ? "OK" : "FALHA") << endl;
        cout << "----------------------------------------" << endl;
        return pass;
    }

    /**
     * @brief Processo principal: executa a sequência de testes.
     */
    void run_all_tests() {
        wait(SC_ZERO_TIME);

        cout << "\n=== Iniciando Testes do ProgramCounter ===\n\n";

        int passed = 0;
        int total = 0;

        // Inicializa entradas
        reset_sig.write(false);
        pc_write_sig.write(false);
        next_pc_sig.write(0);
        wait_cycle(); // aguarda primeira borda para estabilizar reset

        // Teste 1: Reset síncrono
        cout << "--- Teste 1: Reset síncrono ---\n";
        // Aplica reset e valor arbitrário em next_pc
        apply_inputs(true, true, 0xAB);
        bool ok1 = check_pc(0x00, "Reset ativo -> PC deve ser 0x00");
        total++; if (ok1) passed++;

        // Verifica que reset tem prioridade sobre pc_write
        apply_inputs(true, false, 0xCD);
        bool ok2 = check_pc(0x00, "Reset ativo (pc_write=0) -> PC deve ser 0x00");
        total++; if (ok2) passed++;

        // Teste 2: Carga condicional (pc_write = 1)
        cout << "\n--- Teste 2: Carga com pc_write=1 ---\n";
        // Libera reset, ativa escrita
        apply_inputs(false, true, 0x55);
        bool ok3 = check_pc(0x55, "pc_write=1, next_pc=0x55 -> PC deve carregar 0x55");
        total++; if (ok3) passed++;

        // Outro valor
        apply_inputs(false, true, 0xAA);
        bool ok4 = check_pc(0xAA, "pc_write=1, next_pc=0xAA -> PC deve carregar 0xAA");
        total++; if (ok4) passed++;

        // Teste 3: Manutenção de estado (pc_write = 0)
        cout << "\n--- Teste 3: Manutenção com pc_write=0 ---\n";
        // Mantém pc_write=0 e muda next_pc; PC não deve alterar
        apply_inputs(false, false, 0xFF);
        bool ok5 = check_pc(0xAA, "pc_write=0, next_pc=0xFF -> PC deve manter 0xAA");
        total++; if (ok5) passed++;

        // Repete com outro valor de next_pc
        apply_inputs(false, false, 0x11);
        bool ok6 = check_pc(0xAA, "pc_write=0, next_pc=0x11 -> PC deve manter 0xAA");
        total++; if (ok6) passed++;

        // Teste 4: Interação reset > pc_write
        cout << "\n--- Teste 4: Prioridade do reset ---\n";
        // Com reset ativo, pc_write=1 e next_pc diferente de zero
        apply_inputs(true, true, 0x77);
        bool ok7 = check_pc(0x00, "Reset=1, pc_write=1 -> PC deve ser 0x00 (reset prioritário)");
        total++; if (ok7) passed++;

        // Após liberar reset, PC deve permanecer 0 (não carrega next_pc automaticamente sem pc_write)
        apply_inputs(false, false, 0x77);
        bool ok8 = check_pc(0x00, "Após reset, pc_write=0 -> PC mantém 0x00");
        total++; if (ok8) passed++;

        // Agora ativa pc_write para carregar um novo valor
        apply_inputs(false, true, 0x42);
        bool ok9 = check_pc(0x42, "pc_write=1 após reset -> PC carrega 0x42");
        total++; if (ok9) passed++;

        // Teste 5: Sequência de operações (simulação de pequeno programa)
        cout << "\n--- Teste 5: Sequência típica ---\n";
        // Simula uma sequência de incrementos (feitos externamente)
        apply_inputs(false, true, 0x01);
        bool ok10 = check_pc(0x01, "Carga 0x01");
        total++; if (ok10) passed++;

        apply_inputs(false, true, 0x02);
        bool ok11 = check_pc(0x02, "Carga 0x02");
        total++; if (ok11) passed++;

        apply_inputs(false, true, 0x03);
        bool ok12 = check_pc(0x03, "Carga 0x03");
        total++; if (ok12) passed++;

        // Mantém último valor por um ciclo sem escrita
        apply_inputs(false, false, 0xFF);
        bool ok13 = check_pc(0x03, "Manutenção do valor 0x03");
        total++; if (ok13) passed++;

        // Aplica reset no meio da operação
        apply_inputs(true, false, 0x00);
        bool ok14 = check_pc(0x00, "Reset durante operação -> PC=0x00");
        total++; if (ok14) passed++;

        // Sumário
        cout << "\n=== Resultado: " << passed << "/" << total << " testes passaram ===\n";

        sc_stop();
    }

public:
    /**
     * @brief Construtor do testbench.
     */
    SC_CTOR(PC_Tester) : clk_sig("clk", 10, SC_NS) {
        dut = new ProgramCounter("PC_DUT");

        // Conexões
        dut->clk(clk_sig);
        dut->reset(reset_sig);
        dut->pc_write(pc_write_sig);
        dut->next_pc(next_pc_sig);
        dut->pc(pc_sig);

        SC_THREAD(run_all_tests);
        sensitive << clk_sig.posedge_event();
    }

    ~PC_Tester() {
        delete dut;
    }

    // Métodos de acesso para tracing
    const sc_clock&               get_clock()     const { return clk_sig; }
    const sc_signal<bool>&        get_reset()     const { return reset_sig; }
    const sc_signal<bool>&        get_pc_write()  const { return pc_write_sig; }
    const sc_signal<sc_uint<8>>&  get_next_pc()   const { return next_pc_sig; }
    const sc_signal<sc_uint<8>>&  get_pc()        const { return pc_sig; }
};

// Definição do atraso de propagação (1 ns)
const sc_time PC_Tester::PROPAGATION_DELAY(1, SC_NS);

/**
 * @brief Ponto de entrada da simulação SystemC.
 */
int sc_main(int argc, char* argv[]) {
    PC_Tester tester("PC_Tester");

    // Geração opcional de VCD
    sc_trace_file* tf = sc_create_vcd_trace_file("waves/pc_waves");
    if (tf) {
        sc_trace(tf, tester.get_clock(),    "clk");
        sc_trace(tf, tester.get_reset(),    "reset");
        sc_trace(tf, tester.get_pc_write(), "pc_write");
        sc_trace(tf, tester.get_next_pc(),  "next_pc");
        sc_trace(tf, tester.get_pc(),       "pc");
    }

    sc_start(); // Executa até sc_stop()

    if (tf) {
        sc_close_vcd_trace_file(tf);
    }

    return 0;
}