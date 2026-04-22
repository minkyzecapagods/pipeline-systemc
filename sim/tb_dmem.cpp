/**
 * @file   tb_dmem.cpp
 * @brief  Testbench para a memória de dados (DMem) de 16 bits com 256 posições.
 *
 * Verifica as funcionalidades de leitura combinacional e escrita síncrona,
 * o comportamento do sinal write_enable, a sensibilidade da leitura a mudanças
 * nos dados e a integridade de todos os endereços.
 * Gera relatório no console e formas de onda VCD opcionais.
 */

#include <systemc.h>
#include "dmem.h"
#include <iomanip>
#include <iostream>

using namespace std;

/**
 * @class DMem_Tester
 * @brief Módulo de teste para a DMem.
 */
SC_MODULE(DMem_Tester) {
private:
    // Sinais para conexão com o DUT
    sc_signal<sc_uint<DMem::ADDR_WIDTH>> address_sig;
    sc_signal<sc_uint<DMem::DATA_WIDTH>> write_data_sig;
    sc_signal<bool>                      write_enable_sig;
    sc_clock                             clk_sig;
    sc_signal<sc_uint<DMem::DATA_WIDTH>> read_data_sig;

    DMem* dut;

    /**
     * @brief Atraso para propagação combinacional.
     */
    static const sc_time PROPAGATION_DELAY;

    /**
     * @brief Aguarda uma borda de subida do clock.
     */
    void wait_cycle() {
        wait(clk_sig.posedge_event());
    }

    /**
     * @brief Aplica endereço e dados de escrita, com controle de write_enable,
     *        e opcionalmente aguarda a borda do clock para efetivar escrita.
     *
     * @param addr      Endereço a ser acessado.
     * @param wdata     Dado a ser escrito (usado se write_enable = true).
     * @param we        Habilitação de escrita.
     * @param do_cycle  Se true, aguarda a borda do clock (para escrita síncrona).
     *                  Se false, apenas atualiza os sinais combinacionalmente.
     */
    void apply_inputs(sc_uint<DMem::ADDR_WIDTH> addr,
                      sc_uint<DMem::DATA_WIDTH> wdata,
                      bool we,
                      bool do_cycle = true) {
        address_sig.write(addr);
        write_data_sig.write(wdata);
        write_enable_sig.write(we);
        if (do_cycle) {
            wait_cycle();
        }
        wait(PROPAGATION_DELAY); // estabilização
    }

    /**
     * @brief Realiza uma leitura combinacional no endereço especificado.
     * @param addr Endereço a ser lido.
     * @return Valor lido em read_data.
     */
    sc_uint<DMem::DATA_WIDTH> read_address(sc_uint<DMem::ADDR_WIDTH> addr) {
        address_sig.write(addr);
        wait(PROPAGATION_DELAY);
        return read_data_sig.read();
    }

    /**
     * @brief Verifica se a leitura no endereço atual corresponde ao esperado.
     * @param addr     Endereço sendo lido.
     * @param expected Valor esperado.
     * @param desc     Descrição do teste.
     * @return true se o teste passou.
     */
    bool check_read(sc_uint<DMem::ADDR_WIDTH> addr,
                    sc_uint<DMem::DATA_WIDTH> expected,
                    const string& desc) {
        sc_uint<DMem::DATA_WIDTH> actual = read_address(addr);
        bool pass = (actual == expected);

        cout << "@" << sc_time_stamp() << " " << desc << endl;
        cout << "  Endereço: 0x" << hex << setw(2) << setfill('0')
             << addr.to_uint() << dec << endl;
        cout << "  Dado lido: 0x" << hex << setw(4) << setfill('0')
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

        cout << "\n=== Iniciando Testes da DMem ===\n\n";

        int passed = 0;
        int total = 0;

        // Inicializa entradas
        address_sig.write(0);
        write_data_sig.write(0);
        write_enable_sig.write(false);
        wait_cycle();

        // Teste 1: Leitura inicial (todos zeros)
        cout << "--- Teste 1: Leitura inicial (esperado 0) ---\n";
        bool ok1 = check_read(0x00, 0x0000, "Endereço 0x00 inicial");
        total++; if (ok1) passed++;
        bool ok2 = check_read(0xFF, 0x0000, "Endereço 0xFF inicial");
        total++; if (ok2) passed++;
        bool ok3 = check_read(0x42, 0x0000, "Endereço 0x42 inicial");
        total++; if (ok3) passed++;

        // Teste 2: Escrita simples em endereços distintos
        cout << "\n--- Teste 2: Escrita síncrona ---\n";
        // Escreve 0x1234 no endereço 0x10
        apply_inputs(0x10, 0x1234, true, true); // com ciclo de clock
        bool ok4 = check_read(0x10, 0x1234, "Escrita em 0x10 -> leitura 0x1234");
        total++; if (ok4) passed++;

        // Escreve 0xABCD no endereço 0x20
        apply_inputs(0x20, 0xABCD, true, true);
        bool ok5 = check_read(0x20, 0xABCD, "Escrita em 0x20 -> leitura 0xABCD");
        total++; if (ok5) passed++;

        // Verifica que outro endereço não foi alterado
        bool ok6 = check_read(0x10, 0x1234, "Endereço 0x10 mantém 0x1234");
        total++; if (ok6) passed++;

        // Teste 3: Escrita com write_enable = 0 (não deve alterar)
        cout << "\n--- Teste 3: Escrita desabilitada ---\n";
        // Tenta escrever 0xFFFF no endereço 0x10 com write_enable=0
        apply_inputs(0x10, 0xFFFF, false, true);
        bool ok7 = check_read(0x10, 0x1234, "Write_enable=0 -> 0x10 mantém 0x1234");
        total++; if (ok7) passed++;

        // Teste 4: Sensibilidade combinacional da leitura
        cout << "\n--- Teste 4: Sensibilidade combinacional da leitura ---\n";
        // Escreve um valor conhecido
        apply_inputs(0x30, 0x55AA, true, true);
        // Lê imediatamente (sem esperar outro ciclo)
        sc_uint<DMem::DATA_WIDTH> val = read_address(0x30);
        cout << "@" << sc_time_stamp() << " Leitura combinacional do endereço 0x30 após escrita:\n";
        cout << "  Valor lido: 0x" << hex << setw(4) << setfill('0')
             << val.to_uint() << dec << " (esperado 0x55AA) -> "
             << ((val == 0x55AA) ? "OK" : "FALHA") << endl;
        bool ok8 = (val == 0x55AA);
        total++; if (ok8) passed++;

        // Muda o endereço sem clock e verifica resposta imediata
        address_sig.write(0x10);
        wait(PROPAGATION_DELAY);
        val = read_data_sig.read();
        cout << "  Mudança de endereço para 0x10 -> leitura 0x"
             << hex << setw(4) << setfill('0') << val.to_uint() << dec
             << " (esperado 0x1234) -> " << ((val == 0x1234) ? "OK" : "FALHA") << endl;
        bool ok9 = (val == 0x1234);
        total++; if (ok9) passed++;

        // Teste 5: Varredura de todos os 256 endereços
        cout << "\n--- Teste 5: Varredura de todos os 256 endereços ---\n";
        // Preenche cada endereço i com o valor 0xA000 | i
        for (int i = 0; i < DMem::MEM_SIZE; ++i) {
            sc_uint<DMem::DATA_WIDTH> wdata = 0xA000 | i;
            apply_inputs(i, wdata, true, true);
        }

        // desabilita escrita após preenchimento 
        write_enable_sig.write(false);
        wait_cycle(); // garante que nenhuma escrita indesejada ocorra durante a leitura

        // Verifica todos os endereços
        int errors = 0;
        for (int i = 0; i < DMem::MEM_SIZE; ++i) {
            sc_uint<DMem::DATA_WIDTH> expected = 0xA000 | i;
            sc_uint<DMem::DATA_WIDTH> actual = read_address(i);
            if (actual != expected) {
                errors++;
                if (errors <= 5) {
                    cout << "  ERRO no endereço 0x" << hex << setw(2) << setfill('0')
                        << i << dec << ": leu 0x" << hex << setw(4) << setfill('0')
                        << actual.to_uint() << dec << ", esperado 0x"
                        << hex << setw(4) << setfill('0') << expected.to_uint() << dec << endl;
                }
            }
        }
        bool ok10 = (errors == 0);
        cout << "  Varredura completa: " << errors << " erro(s) encontrado(s). "
            << (ok10 ? "OK" : "FALHA") << endl;
        total++; if (ok10) passed++;

        // Teste 6: Sobrescrita de endereço
        cout << "\n--- Teste 6: Sobrescrita de endereço ---\n";
        apply_inputs(0x42, 0x1111, true, true);
        apply_inputs(0x42, 0x2222, true, true);
        bool ok11 = check_read(0x42, 0x2222, "Sobrescrita em 0x42 -> último valor 0x2222");
        total++; if (ok11) passed++;

        // Teste 7: Escrita afeta leitura imediatamente (sensibilidade às células)
        cout << "\n--- Teste 7: Sensibilidade da leitura a alterações na memória ---\n";
        // Mantém o mesmo endereço de leitura e escreve nele
        address_sig.write(0x50);
        wait(PROPAGATION_DELAY);
        sc_uint<DMem::DATA_WIDTH> before = read_data_sig.read();
        cout << "  Antes da escrita, endereço 0x50 contém 0x"
             << hex << setw(4) << setfill('0') << before.to_uint() << dec << endl;

        // Realiza escrita síncrona (aguarda borda do clock)
        write_data_sig.write(0xBEEF);
        write_enable_sig.write(true);
        wait_cycle();
        write_enable_sig.write(false);
        wait(PROPAGATION_DELAY);

        sc_uint<DMem::DATA_WIDTH> after = read_data_sig.read();
        cout << "  Após escrita, endereço 0x50 contém 0x"
             << hex << setw(4) << setfill('0') << after.to_uint() << dec
             << " (esperado 0xBEEF) -> " << ((after == 0xBEEF) ? "OK" : "FALHA") << endl;
        bool ok12 = (after == 0xBEEF);
        total++; if (ok12) passed++;

        // Sumário
        cout << "\n=== Resultado: " << passed << "/" << total << " testes passaram ===\n";
        sc_stop();
    }

public:
    /**
     * @brief Construtor do testbench.
     */
    SC_CTOR(DMem_Tester) : clk_sig("clk", 10, SC_NS) {
        dut = new DMem("DMem_DUT");

        // Conexões
        dut->address(address_sig);
        dut->write_data(write_data_sig);
        dut->write_enable(write_enable_sig);
        dut->clock(clk_sig);
        dut->read_data(read_data_sig);

        SC_THREAD(run_all_tests);
        sensitive << clk_sig.posedge_event();
    }

    ~DMem_Tester() {
        delete dut;
    }

    // Métodos de acesso para tracing VCD
    const sc_signal<sc_uint<DMem::ADDR_WIDTH>>& get_address()      const { return address_sig; }
    const sc_signal<sc_uint<DMem::DATA_WIDTH>>& get_write_data()   const { return write_data_sig; }
    const sc_signal<bool>&                      get_write_enable() const { return write_enable_sig; }
    const sc_clock&                             get_clock()        const { return clk_sig; }
    const sc_signal<sc_uint<DMem::DATA_WIDTH>>& get_read_data()    const { return read_data_sig; }
};

// Definição do atraso de propagação (1 ns)
const sc_time DMem_Tester::PROPAGATION_DELAY(1, SC_NS);

/**
 * @brief Ponto de entrada da simulação SystemC.
 */
int sc_main(int argc, char* argv[]) {
    DMem_Tester tester("DMem_Tester");

    // Geração opcional de VCD
    sc_trace_file* tf = sc_create_vcd_trace_file("waves/dmem_waves");
    if (tf) {
        sc_trace(tf, tester.get_clock(),        "clock");
        sc_trace(tf, tester.get_address(),      "address");
        sc_trace(tf, tester.get_write_data(),   "write_data");
        sc_trace(tf, tester.get_write_enable(), "write_enable");
        sc_trace(tf, tester.get_read_data(),    "read_data");
    }

    sc_start(); // Executa até sc_stop()

    if (tf) {
        sc_close_vcd_trace_file(tf);
    }

    return 0;
}