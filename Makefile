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

alu: $(BUILD_DIR)/tb_alu

regfile: $(BUILD_DIR)/tb_regfile

pc: $(BUILD_DIR)/tb_pc

imem: $(BUILD_DIR)/tb_imem

dmem: $(BUILD_DIR)/tb_dmem

control: $(BUILD_DIR)/tb_control

datapath: $(BUILD_DIR)/tb_datapath

processor: $(BUILD_DIR)/tb_processor

# Cria o diretório build se não existir
$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/tb_alu: sim/tb_alu.cpp include/alu.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_alu.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_regfile: sim/tb_regfile.cpp include/regfile.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_regfile.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_pc: sim/tb_pc.cpp include/pc.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_pc.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_imem: sim/tb_imem.cpp include/imem.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_imem.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_dmem: sim/tb_dmem.cpp include/dmem.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_dmem.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_control: sim/tb_control.cpp include/control.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_control.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_datapath: sim/tb_datapath.cpp include/alu.h include/regfile.h include/pc.h include/imem.h include/dmem.h include/control.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_datapath.cpp $(LDFLAGS) -o $@

$(BUILD_DIR)/tb_processor: sim/tb_processor.cpp include/alu.h include/regfile.h include/pc.h include/imem.h include/dmem.h include/control.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) sim/tb_processor.cpp $(LDFLAGS) -o $@
	
run_alu: $(BUILD_DIR)/tb_alu
	./$(BUILD_DIR)/tb_alu

run_regfile: $(BUILD_DIR)/tb_regfile
	./$(BUILD_DIR)/tb_regfile

run_pc: $(BUILD_DIR)/tb_pc
	./$(BUILD_DIR)/tb_pc
	
run_imem: $(BUILD_DIR)/tb_imem
	./$(BUILD_DIR)/tb_imem

run_dmem: $(BUILD_DIR)/tb_dmem
	./$(BUILD_DIR)/tb_dmem

run_control: $(BUILD_DIR)/tb_control
	./$(BUILD_DIR)/tb_control

run_datapath: $(BUILD_DIR)/tb_datapath
	./$(BUILD_DIR)/tb_datapath

run_processor: $(BUILD_DIR)/tb_processor
	./$(BUILD_DIR)/tb_processor

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