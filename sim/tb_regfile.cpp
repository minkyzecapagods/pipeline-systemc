/**
 * @file   tb_regfile.cpp
 * @brief  Testbench para o banco de registradores RegFile.
 */

#include <systemc.h>
#include "regfile.h"
#include <vector>
#include <iomanip>
#include <functional>
#include <string>

// Constantes da simulação
static const sc_time COMB_DELAY    = sc_time(1, SC_NS);   // Atraso combinacional
static const sc_time CLK_PERIOD    = sc_time(10, SC_NS);  // Período do clock
static const int     STARTUP_CYCLES = 2;                  // Ciclos iniciais de estabilização

// Módulo testador
SC_MODULE(RegFile_Tester) {
private:
    using addr_t = sc_uint<RegFile::ADDR_WIDTH>;
    using data_t = sc_uint<RegFile::DATA_WIDTH>;

    // Sinais
    sc_signal<addr_t> read_addr1_sig{"read_addr1"};
    sc_signal<addr_t> read_addr2_sig{"read_addr2"};
    sc_signal<addr_t> write_addr_sig{"write_addr"};
    sc_signal<data_t> write_data_sig{"write_data"};
    sc_signal<bool>   write_enable_sig{"write_enable"};
    sc_clock          clk_sig{"clk", CLK_PERIOD};
    sc_signal<data_t> read_data1_sig{"read_data1"};
    sc_signal<data_t> read_data2_sig{"read_data2"};

    RegFile* dut;

    // Contador de testes
    struct TestResult {
        int passed = 0;
        int total  = 0;
        void add(bool pass) { total++; if (pass) passed++; }
    } result;

    // Métodos auxiliares de temporização
    void wait_clock(unsigned n = 1) {
        for (unsigned i = 0; i < n; ++i) wait(clk_sig.posedge_event());
    }

    void wait_comb() { wait(COMB_DELAY); }

    // Operações básicas
    void do_write(addr_t addr, data_t data) {
        write_addr_sig.write(addr);
        write_data_sig.write(data);
        write_enable_sig.write(true);
        wait_clock();
        write_enable_sig.write(false);
        wait_comb();
    }

    void do_read(addr_t addr1, addr_t addr2) {
        read_addr1_sig.write(addr1);
        read_addr2_sig.write(addr2);
        wait_comb();
    }

    bool check_read(addr_t addr1, addr_t addr2,
                    data_t expected1, data_t expected2,
                    const std::string& description) {
        data_t actual1 = read_data1_sig.read();
        data_t actual2 = read_data2_sig.read();

        std::cout << "@" << sc_time_stamp() << " " << description << "\n";
        std::cout << "  read_addr = (" << addr1 << ", " << addr2
                  << ") => dados = (" << actual1 << ", " << actual2 << ")\n";

        bool pass = (actual1 == expected1) && (actual2 == expected2);
        if (!pass) {
            std::cout << "  ERRO: esperado (" << expected1 << ", " << expected2 << ")\n";
        } else {
            std::cout << "  OK.\n";
        }
        std::cout << std::endl;
        return pass;
    }

    // Definição dos testes individuais
    void test_initial_reads() {
        std::cout << "\n--- Teste: Leitura inicial (todos zero) ---\n";
        for (int i = 0; i < RegFile::NUM_REGS; ++i) {
            int j = (i + 1) % RegFile::NUM_REGS;
            do_read(i, j);
            bool ok = check_read(i, j, 0, 0,
                "Leitura inicial de R" + std::to_string(i) + " e R" + std::to_string(j));
            result.add(ok);
        }
    }

    void test_r0_hardwired() {
        std::cout << "\n--- Teste: R0 hardwired (escrita ignorada) ---\n";
        do_write(0, 0xAAAA);
        do_read(0, 1);
        bool ok = check_read(0, 1, 0, 0,
            "Leitura após tentar escrever 0xAAAA em R0");
        result.add(ok);
    }

    void test_write_to_regs_1_7() {
        std::cout << "\n--- Teste: Escrita em R1..R7 ---\n";
        const data_t patterns[7] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777};
        for (int i = 1; i < RegFile::NUM_REGS; ++i) {
            do_write(i, patterns[i-1]);
            do_read(i, 0);
            bool ok = check_read(i, 0, patterns[i-1], 0,
                "Verificação de escrita em R" + std::to_string(i));
            result.add(ok);
        }
    }

    void test_simultaneous_read() {
        std::cout << "\n--- Teste: Leitura simultânea ---\n";
        do_read(1, 5);
        bool ok1 = check_read(1, 5, 0x1111, 0x5555, "Leitura R1 e R5");
        result.add(ok1);
        do_read(3, 7);
        bool ok2 = check_read(3, 7, 0x3333, 0x7777, "Leitura R3 e R7");
        result.add(ok2);
    }

    void test_write_disabled() {
        std::cout << "\n--- Teste: Escrita com write_enable = 0 ---\n";
        write_addr_sig.write(1);
        write_data_sig.write(0xDEAD);
        write_enable_sig.write(false);
        wait_clock();
        wait_comb();
        do_read(1, 0);
        bool ok = check_read(1, 0, 0x1111, 0, "R1 deve permanecer 0x1111");
        result.add(ok);
    }

    void test_overwrite() {
        std::cout << "\n--- Teste: Sobrescrita de registrador ---\n";
        do_write(2, 0xBEEF);
        do_read(2, 0);
        bool ok = check_read(2, 0, 0xBEEF, 0, "R2 agora deve conter 0xBEEF");
        result.add(ok);
    }

    void test_comb_read_sensitivity() {
        std::cout << "\n--- Teste: Sensibilidade combinacional da leitura ---\n";
        read_addr1_sig.write(1);
        read_addr2_sig.write(2);
        wait_comb();
        data_t d1 = read_data1_sig.read();
        data_t d2 = read_data2_sig.read();
        std::cout << "@" << sc_time_stamp() << " Endereços alterados para (1,2):\n";
        std::cout << "  dados = (" << d1 << ", " << d2 << ") esperado (0x1111, 0xBEEF)\n";
        bool ok = (d1 == 0x1111 && d2 == 0xBEEF);
        std::cout << (ok ? "  OK." : "  ERRO.") << std::endl << std::endl;
        result.add(ok);
    }

    void test_reg_array_sensitivity() {
        std::cout << "\n--- Teste: Sensibilidade à mudança no array de registradores ---\n";
        do_write(1, 0xCAFE);
        wait_comb(); // leitura combinacional já deve refletir
        data_t d1 = read_data1_sig.read();
        std::cout << "@" << sc_time_stamp() << " Após escrita síncrona em R1 (0xCAFE):\n";
        std::cout << "  read_data1 = " << d1 << " (esperado 0xCAFE)\n";
        bool ok = (d1 == 0xCAFE);
        std::cout << (ok ? "  OK." : "  ERRO.") << std::endl << std::endl;
        result.add(ok);
    }

    // Processo principal
    void run_all_tests() {
        // Inicialização silenciosa
        read_addr1_sig.write(0);
        read_addr2_sig.write(0);
        write_addr_sig.write(0);
        write_data_sig.write(0);
        write_enable_sig.write(false);
        wait_clock(STARTUP_CYCLES);

        std::cout << "\n=== Iniciando Testes do RegFile ===\n";

        // Vetor de funções de teste (fácil de adicionar/remover)
        std::vector<std::function<void()>> tests = {
            [this]() { test_initial_reads(); },
            [this]() { test_r0_hardwired(); },
            [this]() { test_write_to_regs_1_7(); },
            [this]() { test_simultaneous_read(); },
            [this]() { test_write_disabled(); },
            [this]() { test_overwrite(); },
            [this]() { test_comb_read_sensitivity(); },
            [this]() { test_reg_array_sensitivity(); }
        };

        for (auto& test : tests) {
            test();
        }

        std::cout << "\n=== Resultado: " << result.passed << "/" << result.total
                  << " testes passaram ===\n";
        sc_stop();
    }

public:
    SC_CTOR(RegFile_Tester) {
        dut = new RegFile("RegFile_DUT");

        // Conexões
        dut->read_addr1(read_addr1_sig);
        dut->read_addr2(read_addr2_sig);
        dut->write_addr(write_addr_sig);
        dut->write_data(write_data_sig);
        dut->write_enable(write_enable_sig);
        dut->clock(clk_sig);
        dut->read_data1(read_data1_sig);
        dut->read_data2(read_data2_sig);

        SC_THREAD(run_all_tests);
        // Sem sensibilidade estática, pois usamos waits explícitos
    }

    ~RegFile_Tester() { delete dut; }

    // Métodos de acesso para tracing
    const sc_signal<addr_t>& get_read_addr1()  const { return read_addr1_sig; }
    const sc_signal<addr_t>& get_read_addr2()  const { return read_addr2_sig; }
    const sc_signal<addr_t>& get_write_addr()  const { return write_addr_sig; }
    const sc_signal<data_t>& get_write_data()  const { return write_data_sig; }
    const sc_signal<bool>&   get_write_enable() const { return write_enable_sig; }
    const sc_clock&          get_clock()       const { return clk_sig; }
    const sc_signal<data_t>& get_read_data1()  const { return read_data1_sig; }
    const sc_signal<data_t>& get_read_data2()  const { return read_data2_sig; }
};

// Ponto de entrada da simulação
int sc_main(int argc, char* argv[]) {
    RegFile_Tester tester("RegFile_Tester");

    sc_trace_file* tf = sc_create_vcd_trace_file("waves/regfile_waves");
    if (tf) {
        sc_trace(tf, tester.get_clock(),         "clock");
        sc_trace(tf, tester.get_read_addr1(),    "read_addr1");
        sc_trace(tf, tester.get_read_addr2(),    "read_addr2");
        sc_trace(tf, tester.get_write_addr(),    "write_addr");
        sc_trace(tf, tester.get_write_data(),    "write_data");
        sc_trace(tf, tester.get_write_enable(),  "write_enable");
        sc_trace(tf, tester.get_read_data1(),    "read_data1");
        sc_trace(tf, tester.get_read_data2(),    "read_data2");
    }

    sc_start();

    if (tf) sc_close_vcd_trace_file(tf);
    return 0;
}