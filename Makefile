# Variáveis
CXX = g++
CXXFLAGS = -std=c++17 -I./include -I/usr/local/systemc-3.0.2/include
LDFLAGS = -L/usr/local/systemc-3.0.2/lib-linux64 -lsystemc -Wl,-rpath=/usr/local/systemc-3.0.2/lib-linux64
SRC = src/main.cpp
TARGET = build/mips_sim
VCD_FILE = waves/simulacao.vcd

# Targets principais
.PHONY: build run run_waves help clean default

# Alvo padrão (sem argumentos)
default:
	@echo "Use 'make help' para ver as opções disponíveis."

# Compilação
build:
	@echo "Compilando o projeto..."
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)
	@echo "Executável criado em $(TARGET)"

# Execução com passagem de argumento
run: build
	@if [ -z "$(ARGS)" ]; then \
		echo "Erro: É necessário passar o arquivo de instruções. Exemplo: make run ARGS=caminho/arquivo.txt"; \
		exit 1; \
	fi
	@echo "Executando simulação com o arquivo $(ARGS)..."
	mkdir -p waves
	./$(TARGET) $(ARGS)
	@if [ -f simulacao.vcd ]; then \
		mv simulacao.vcd waves/; \
		echo "Simulação concluída. Arquivo de onda salvo em $(VCD_FILE)"; \
	else \
		echo "Aviso: O arquivo simulacao.vcd não foi gerado."; \
	fi

# Visualização das ondas no GTKWave
run_waves:
	@if [ ! -f $(VCD_FILE) ]; then \
		echo "Erro: Arquivo $(VCD_FILE) não encontrado."; \
		echo "Execute primeiro: make run ARGS=<arquivo_de_instrucoes>"; \
		exit 1; \
	fi
	@echo "Abrindo $(VCD_FILE) no GTKWave..."
	gtkwave $(VCD_FILE)

# Limpeza dos artefatos gerados
clean:
	@echo "Limpando diretórios de build e waves..."
	rm -rf build waves
	@echo "Limpeza concluída."

# Mensagem de ajuda
help:
	@echo "Makefile para compilar e simular o processador MIPS com SystemC."
	@echo ""
	@echo "Comandos disponíveis:"
	@echo "  make build            - Compila o projeto e gera o executável em build/"
	@echo "  make run ARGS=...     - Executa a simulação com o arquivo de instruções (obrigatório)"
	@echo "                          Exemplo: make run ARGS=tests/teste1.txt"
	@echo "                          As ondas (VCD) são movidas para a pasta waves/"
	@echo "  make run_waves        - Abre o arquivo waves/simulacao.vcd no GTKWave"
	@echo "  make clean            - Remove os diretórios build/ e waves/"
	@echo "  make help             - Mostra esta mensagem"
	@echo "  make                  - Exibe sugestão para usar o help"