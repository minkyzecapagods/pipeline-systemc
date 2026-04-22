# Caminho do SystemC (ajuste conforme necessário)
SYSTEMC_HOME = /usr/local/systemc-3.0.2

# Diretórios
BUILD_DIR = build

# Compilador e flags
CXX = g++
CXXFLAGS = -I. -Iinclude -I$(SYSTEMC_HOME)/include
LDFLAGS = -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc

# Regras
.PHONY: all alu run clean

all: alu

# Compila o testbench da ALU
alu: $(BUILD_DIR)/tb_alu

# Cria o diretório build se não existir
$(BUILD_DIR):
	mkdir -p $@

# Compila e linka o testbench
$(BUILD_DIR)/tb_alu: sim/tb_alu.cpp include/alu.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_alu.cpp $(LDFLAGS) -o $@

# Executa o testbench
run_alu: $(BUILD_DIR)/tb_alu
	./$(BUILD_DIR)/tb_alu

# Limpeza: remove todo o diretório build e arquivos .vcd da raiz
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.vcd

# Ajuda
help:
	@echo "Comandos disponíveis:"
	@echo "  make alu   - Compila o testbench da ALU (executável em build/tb_alu)"
	@echo "  make run   - Compila e executa o testbench"
	@echo "  make clean - Remove o diretório build e arquivos .vcd"
	@echo "  make help  - Exibe esta mensagem"