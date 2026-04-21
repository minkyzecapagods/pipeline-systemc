# Caminho do SystemC (altere se necessário)
SYSTEMC_HOME = /usr/local/systemc-3.0.2

# Compilador e flags
CXX = g++
CXXFLAGS = -I. -Iinclude -I$(SYSTEMC_HOME)/include
LDFLAGS = -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc

# Arquivos
ALU_SRC = src/alu.cpp
ALU_TB  = sim/tb_alu.cpp
ALU_OBJ = alu.o tb_alu.o
ALU_EXE = tb_alu

# Alvo padrão: compila e executa a ALU
all: run_alu

# Compila os objetos
alu.o: $(ALU_SRC) include/alu.h
	$(CXX) $(CXXFLAGS) -c $(ALU_SRC) -o alu.o

tb_alu.o: $(ALU_TB) include/alu.h
	$(CXX) $(CXXFLAGS) -c $(ALU_TB) -o tb_alu.o

# Linka o executável
$(ALU_EXE): alu.o tb_alu.o
	$(CXX) $(ALU_OBJ) $(LDFLAGS) -o $(ALU_EXE)

# Executa o testbench (depende do executável)
run_alu: $(ALU_EXE)
	./$(ALU_EXE)

# Limpeza
clean:
	rm -f alu.o tb_alu.o $(ALU_EXE) *.vcd

.PHONY: all run_alu clean