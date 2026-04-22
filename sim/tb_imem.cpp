/**
 * @file   tb_imem.cpp
 * @brief  Testbench para a memória de instruções (IMem) combinacional.
 *
 * Verifica se a leitura da memória de instruções retorna o valor correto
 * para cada endereço, se reage imediatamente a mudanças de endereço e se
 * todos os 256 endereços estão acessíveis.
 * Gera relatório no console e formas de onda VCD opcionais.
 */

#include <systemc.h>
#include "imem.h"
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * @class IMem_Tester
 * @brief Módulo de teste para a IMem.
 */
SC_MODULE(IMem_Tester) {
private:
    // Sinais de conexão com o DUT
    sc_signal<sc_uint<8>>  address_sig;
    sc_signal<sc_uint<16>> instruction_sig;

    IMem* dut;

    /**
     * @brief Atraso para propagação combinacional.
     */
    static const sc_time PROPAGATION_DELAY;

    /**
     * @brief Preenche a memória com um padrão determinístico para testes.
     * 
     * Cada endereço `i` recebe o valor `0x1000 + i` (ou um padrão que permita
     * identificar facilmente se o endereço está correto).
     */
    void initialize_memory() {
        for (int i = 0; i < 256; ++i) {
            // Padrão: byte alto = 0x10, byte baixo = i (endereço)
            dut->memory[i] = 0x1000 | i;
        }
        // Sobrescreve alguns endereços com valores específicos para testes de borda
        dut->memory[0]   = 0x0000;
        dut->memory[255] = 0xFFFF;
        dut->memory[0x42] = 0x1234;
        dut->memory[0xAB] = 0xABCD;
    }

    /**
     * @brief Aplica um endereço e aguarda o tempo de propagação.
     * @param addr Endereço a ser aplicado.
     */
    void apply_address(sc_uint<8> addr) {
        address_sig.write(addr);
        wait(PROPAGATION_DELAY);
    }

    /**
     * @brief Verifica se a instrução lida corresponde ao valor esperado.
     * @param addr Endereço aplicado.
     * @param expected Valor esperado para instruction.
     * @param desc Descrição textual do teste.
     * @return true se o teste passou.
     */
    bool check_instruction(sc_uint<8> addr, sc_uint<16> expected, const string& desc) {
        sc_uint<16> actual = instruction_sig.read();
        bool pass = (actual == expected);

        cout << "@" << sc_time_stamp() << " " << desc << endl;
        cout << "  Endereço: 0x" << hex << setw(2) << setfill('0')
             << addr.to_uint() << dec << endl;
        cout << "  Instrução lida: 0x" << hex << setw(4) << setfill('0')
             << actual.to_uint() << dec;
        if (!pass) {
            cout << " (ESPERADO: 0x" << hex << setw(4) << setfill('0')
                 << expected.to_uint() << dec << ")";
        }
        cout << " -> " << (pass ? "OK" : "FALHA") << endl;
        cout << "----------------------------------------" << endl;
        return pass;
    }

    /**
     * @brief Processo principal de teste.
     */
    void run_all_tests() {
        wait(SC_ZERO_TIME);

        cout << "\n=== Iniciando Testes da IMem ===\n\n";

        int passed = 0;
        int total = 0;

        // Inicializa a memória com padrões conhecidos
        initialize_memory();

        // Teste 1: Leitura de endereços específicos com valores conhecidos
        cout << "--- Teste 1: Leitura de endereços pré-definidos ---\n";

        apply_address(0);
        bool ok1 = check_instruction(0, 0x0000, "Endereço 0 -> deve retornar 0x0000");
        total++; if (ok1) passed++;

        apply_address(255);
        bool ok2 = check_instruction(255, 0xFFFF, "Endereço 255 -> deve retornar 0xFFFF");
        total++; if (ok2) passed++;

        apply_address(0x42);
        bool ok3 = check_instruction(0x42, 0x1234, "Endereço 0x42 -> deve retornar 0x1234");
        total++; if (ok3) passed++;

        apply_address(0xAB);
        bool ok4 = check_instruction(0xAB, 0xABCD, "Endereço 0xAB -> deve retornar 0xABCD");
        total++; if (ok4) passed++;

        // Teste 2: Leitura de endereços preenchidos com padrão (0x1000 | addr)
        cout << "\n--- Teste 2: Leitura de endereços com padrão (0x1000 | addr) ---\n";

        // Seleciona alguns endereços não sobrescritos (evitando 0, 255, 0x42, 0xAB)
        sc_uint<8> test_addrs[] = {0x01, 0x10, 0x20, 0x7F, 0x80, 0xFE};
        for (auto addr : test_addrs) {
            apply_address(addr);
            sc_uint<16> expected = 0x1000 | addr;
            bool ok = check_instruction(addr, expected,
                        "Endereço 0x" + to_string(addr.to_uint()) +
                        " -> esperado 0x" + to_string(expected.to_uint()));
            total++; if (ok) passed++;
        }

        // Teste 3: Sensibilidade combinacional (mudança de endereço sem clock)
        cout << "\n--- Teste 3: Sensibilidade combinacional ---\n";

        // Altera endereço sem esperar por borda de clock (não há clock mesmo)
        address_sig.write(0x55);
        wait(PROPAGATION_DELAY);
        sc_uint<16> val1 = instruction_sig.read();

        address_sig.write(0xAA);
        wait(PROPAGATION_DELAY);
        sc_uint<16> val2 = instruction_sig.read();

        cout << "  Endereço 0x55 -> instrução 0x" << hex << setw(4) << setfill('0')
             << val1.to_uint() << dec << endl;
        cout << "  Endereço 0xAA -> instrução 0x" << hex << setw(4) << setfill('0')
             << val2.to_uint() << dec << endl;

        sc_uint<16> exp1 = 0x1000 | 0x55;
        sc_uint<16> exp2 = 0x1000 | 0xAA;
        bool ok5 = (val1 == exp1) && (val2 == exp2);
        cout << "  Mudança imediata de saída: " << (ok5 ? "OK" : "FALHA") << endl;
        cout << "----------------------------------------" << endl;
        total++; if (ok5) passed++;

        // Teste 4: Varredura completa de todos os 256 endereços
        cout << "\n--- Teste 4: Varredura completa (256 endereços) ---\n";

        int errors = 0;
        for (int addr = 0; addr < 256; ++addr) {
            apply_address(addr);
            sc_uint<16> actual = instruction_sig.read();
            sc_uint<16> expected;
            // Define o valor esperado baseado nos sobrescritos ou no padrão
            if (addr == 0) expected = 0x0000;
            else if (addr == 255) expected = 0xFFFF;
            else if (addr == 0x42) expected = 0x1234;
            else if (addr == 0xAB) expected = 0xABCD;
            else expected = 0x1000 | addr;

            if (actual != expected) {
                errors++;
                // Reporta apenas os primeiros erros para não poluir muito
                if (errors <= 5) {
                    cout << "  ERRO no endereço 0x" << hex << setw(2) << setfill('0')
                         << addr << dec << ": leu 0x" << hex << setw(4) << setfill('0')
                         << actual.to_uint() << dec << ", esperado 0x"
                         << hex << setw(4) << setfill('0') << expected.to_uint() << dec << endl;
                }
            }
        }

        bool ok6 = (errors == 0);
        cout << "  Varredura completa: " << errors << " erro(s) encontrado(s). "
             << (ok6 ? "OK" : "FALHA") << endl;
        cout << "----------------------------------------" << endl;
        total++; if (ok6) passed++;

        // ------------------------------------------------------------
        // Sumário
        // ------------------------------------------------------------
        cout << "\n=== Resultado: " << passed << "/" << total << " testes passaram ===\n";

        sc_stop();
    }

public:
    /**
     * @brief Construtor do testbench.
     */
    SC_CTOR(IMem_Tester) {
        dut = new IMem("IMem_DUT");

        // Conexões
        dut->address(address_sig);
        dut->instruction(instruction_sig);

        SC_THREAD(run_all_tests);
    }

    ~IMem_Tester() {
        delete dut;
    }

    // Métodos de acesso para tracing
    const sc_signal<sc_uint<8>>&  get_address()     const { return address_sig; }
    const sc_signal<sc_uint<16>>& get_instruction() const { return instruction_sig; }
};

// Definição do atraso de propagação (1 ns)
const sc_time IMem_Tester::PROPAGATION_DELAY(1, SC_NS);

/**
 * @brief Ponto de entrada da simulação SystemC.
 */
int sc_main(int argc, char* argv[]) {
    IMem_Tester tester("IMem_Tester");

    // Geração opcional de VCD
    sc_trace_file* tf = sc_create_vcd_trace_file("waves/imem_waves");
    if (tf) {
        sc_trace(tf, tester.get_address(),     "address");
        sc_trace(tf, tester.get_instruction(), "instruction");
    }

    sc_start(); // Executa até sc_stop()

    if (tf) {
        sc_close_vcd_trace_file(tf);
    }

    return 0;
}