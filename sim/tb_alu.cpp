/**
 * @file   tb_alu.cpp
 * @brief  Testbench para verificação funcional da ALU de 16 bits.
 */

#include <systemc.h>
#include "alu.h"
#include <vector>
#include <iomanip>

using namespace std;

/**
 * @class ALU_Tester
 * @brief Módulo de teste para a ALU.
 *
 * Aplica um conjunto pré-definido de vetores de teste à ALU e verifica
 * se as saídas (resultado e flags) correspondem aos valores esperados.
 * Gera relatório no console e opcionalmente formas de onda VCD.
 */
SC_MODULE(ALU_Tester) {
private:
    // Sinais que conectam o testador à ALU
    sc_signal<sc_uint<16>> operand_a_sig;
    sc_signal<sc_uint<16>> operand_b_sig;
    sc_signal<sc_uint<4>>  opcode_sig;
    sc_signal<sc_uint<16>> result_sig;
    sc_signal<bool>        negative_flag_sig;
    sc_signal<bool>        zero_flag_sig;

    ALU* alu_instance;

    /**
     * @brief Atraso para propagação dos sinais combinacionais.
     */
    static const sc_time PROPAGATION_DELAY;

    /**
     * @brief Estrutura que representa um único caso de teste.
     */
    struct TestVector {
        sc_uint<16> operand_a;
        sc_uint<16> operand_b;
        sc_uint<4>  opcode;
        sc_uint<16> expected_result;
        bool        expected_negative;
        bool        expected_zero;
    };

    /**
     * @brief Aplica um estímulo, aguarda propagação e coleta os resultados.
     * @param tv Vetor de teste a ser aplicado.
     * @param[out] actual_result Resultado lido da ALU.
     * @param[out] actual_negative Flag N lida.
     * @param[out] actual_zero Flag Z lida.
     */
    void apply_stimulus(const TestVector& tv,
                        sc_uint<16>& actual_result,
                        bool& actual_negative,
                        bool& actual_zero) {
        operand_a_sig.write(tv.operand_a);
        operand_b_sig.write(tv.operand_b);
        opcode_sig.write(tv.opcode);
        wait(PROPAGATION_DELAY);

        actual_result   = result_sig.read();
        actual_negative = negative_flag_sig.read();
        actual_zero     = zero_flag_sig.read();
    }

    /**
     * @brief Verifica se os resultados correspondem ao esperado.
     * @return true se o teste passou.
     */
    bool check_result(const TestVector& tv,
                      sc_uint<16> actual_result,
                      bool actual_negative,
                      bool actual_zero) const {
        return (actual_result   == tv.expected_result) &&
               (actual_negative == tv.expected_negative) &&
               (actual_zero     == tv.expected_zero);
    }

    /**
     * @brief Imprime cabeçalho da tabela de resultados.
     */
    void print_header() const {
        cout << "\n=== Iniciando Testes da ALU ===\n";
        cout << left
            << setw(24) << "Operando A"
            << setw(24) << "Operando B"
            << setw(4)  << "Op"
            << setw(24) << "Resultado"
            << "N Z Status\n";
        cout << setw(24) << "(dec/bin)"
            << setw(24) << "(dec/bin)"
            << setw(4)  << ""
            << setw(24) << "(dec/bin)"
            << "\n";
        cout << string(24*3 + 4 + 10, '-') << "\n";
    }

    /**
     * @brief Imprime uma linha da tabela de resultados, adaptando formato ao opcode.
     */
    void print_result_line(const TestVector& tv,
                        sc_uint<16> actual_result,
                        bool actual_negative,
                        bool actual_zero,
                        bool passed) const {
        bool is_arithmetic = (tv.opcode >= 4);  // CMP(4), ADD(5), SUB(6)

        auto format_value = [is_arithmetic](sc_uint<16> val) -> string {
            ostringstream oss;
            if (is_arithmetic) {
                oss << dec << val.to_uint(); // decimal sem formatação extra
            } else {
                // Binário com 16 bits, agrupados em nibbles
                oss << "0b";
                for (int i = 15; i >= 0; --i) {
                    oss << ((val >> i) & 1);
                    if (i % 4 == 0 && i != 0) oss << '_';
                }
            }
            return oss.str();
        };

        cout << left
            << setw(24) << format_value(tv.operand_a)
            << setw(24) << format_value(tv.operand_b)
            << setw(4)  << tv.opcode
            << setw(24) << format_value(actual_result)
            << actual_negative << " " << actual_zero << " "
            << (passed ? "OK" : "FALHA") << endl;

        if (!passed) {
            cout << "  Esperado: res=" << format_value(tv.expected_result)
                << " N=" << tv.expected_negative
                << " Z=" << tv.expected_zero << "\n";
        }
    }

    /**
     * @brief Processo principal: executa todos os casos de teste.
     */
    void run_all_tests() {
        wait(SC_ZERO_TIME);  // Garante que todos os processos estejam registrados

        const vector<TestVector> test_vectors = {
            // AND
            {0xFFFF, 0xAAAA, 0, 0xAAAA, true,  false},
            {0x1234, 0x00FF, 0, 0x0034, false, false},
            {0x0000, 0xFFFF, 0, 0x0000, false, true },
            // OR
            {0xF0F0, 0x0F0F, 1, 0xFFFF, true,  false},
            {0x0000, 0x0000, 1, 0x0000, false, true },
            // XOR
            {0xAAAA, 0x5555, 2, 0xFFFF, true,  false},
            {0x1234, 0x1234, 2, 0x0000, false, true },
            // NOT (operando B ignorado)
            {0xAAAA, 0x0000, 3, 0x5555, false, false},
            {0x0000, 0x0000, 3, 0xFFFF, true,  false},
            {0xFFFF, 0x0000, 3, 0x0000, false, true },
            // CMP (subtração usada para comparação)
            {0x0005, 0x0003, 4, 0x0002, false, false},
            {0x0003, 0x0005, 4, 0xFFFE, true,  false},
            {0x0005, 0x0005, 4, 0x0000, false, true },
            // ADD
            {0x0002, 0x0003, 5, 0x0005, false, false},
            {0x7FFF, 0x0001, 5, 0x8000, true,  false},
            {0xFFFF, 0x0001, 5, 0x0000, false, true },
            // SUB
            {0x0005, 0x0002, 6, 0x0003, false, false},
            {0x0001, 0x0002, 6, 0xFFFF, true,  false},
            {0x0000, 0x0000, 6, 0x0000, false, true }
        };

        print_header();

        int passed_count = 0;
        const int total_tests = test_vectors.size();

        for (const auto& tv : test_vectors) {
            sc_uint<16> actual_result;
            bool actual_negative, actual_zero;

            apply_stimulus(tv, actual_result, actual_negative, actual_zero);
            bool test_passed = check_result(tv, actual_result, actual_negative, actual_zero);
            if (test_passed) ++passed_count;

            print_result_line(tv, actual_result, actual_negative, actual_zero, test_passed);
        }

        cout << "\n=== Resultado: " << passed_count << "/" << total_tests << " testes passaram ===\n";
        sc_stop();
    }

public:
    /**
     * @brief Construtor do testador.
     * @param name Nome da instância.
     */
    SC_CTOR(ALU_Tester) {
        alu_instance = new ALU("ALU_Instance");
        alu_instance->operand_a(operand_a_sig);
        alu_instance->operand_b(operand_b_sig);
        alu_instance->operation_code(opcode_sig);
        alu_instance->result(result_sig);
        alu_instance->negative_flag(negative_flag_sig);
        alu_instance->zero_flag(zero_flag_sig);

        SC_THREAD(run_all_tests);
    }

    /**
     * @brief Destrutor. Libera a instância da ALU.
     */
    ~ALU_Tester() {
        delete alu_instance;
    }

    // Métodos de acesso aos sinais para trace
    const sc_signal<sc_uint<16>>& get_operand_a_signal() const { return operand_a_sig; }
    const sc_signal<sc_uint<16>>& get_operand_b_signal() const { return operand_b_sig; }
    const sc_signal<sc_uint<4>>&  get_opcode_signal()    const { return opcode_sig; }
    const sc_signal<sc_uint<16>>& get_result_signal()    const { return result_sig; }
    const sc_signal<bool>&        get_negative_signal()  const { return negative_flag_sig; }
    const sc_signal<bool>&        get_zero_signal()      const { return zero_flag_sig; }
};

// Definição da constante de atraso
const sc_time ALU_Tester::PROPAGATION_DELAY(1, SC_NS);

/**
 * @brief Ponto de entrada da simulação SystemC.
 */
int sc_main(int argc, char* argv[]) {
    ALU_Tester tester("ALU_Tester");

    // Rastreamento VCD
    sc_trace_file* trace_file = sc_create_vcd_trace_file("waves/alu_waves");
    if (trace_file) {
        sc_trace(trace_file, tester.get_operand_a_signal(), "operand_a");
        sc_trace(trace_file, tester.get_operand_b_signal(), "operand_b");
        sc_trace(trace_file, tester.get_opcode_signal(),    "opcode");
        sc_trace(trace_file, tester.get_result_signal(),    "result");
        sc_trace(trace_file, tester.get_negative_signal(),  "negative_flag");
        sc_trace(trace_file, tester.get_zero_signal(),      "zero_flag");
    }

    sc_start();  // Inicia a simulação (executa até sc_stop())

    if (trace_file) {
        sc_close_vcd_trace_file(trace_file);
    }
    return 0;
}