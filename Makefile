# Variáveis
CXX = g++
CXXFLAGS = -std=c++17 -I./include -I/usr/local/systemc-3.0.2/include
LDFLAGS = -L/usr/local/systemc-3.0.2/lib-linux64 -lsystemc -Wl,-rpath=/usr/local/systemc-3.0.2/lib-linux64

# Diretórios
SRCDIR   = src
INCDIR   = include
BUILDDIR = build

# Arquivos fonte (lista manual)
SOURCES  = $(SRCDIR)/main.cpp
# Se houver mais arquivos .cpp (ex: módulos), adicione aqui

# Objetos (cria um .o para cada .cpp)
OBJECTS  = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))

# Executável final
TARGET   = $(BUILDDIR)/mips_sim

# Arquivo VCD
VCD_FILE = waves/simulacao.vcd

# Regra padrão (sem argumentos)
default:
	@echo "Use 'make help' para ver as opções."

# Compilação condicional: gera o executável apenas se fontes/objetos mudaram
$(TARGET): $(OBJECTS)
	@echo "Linkando o executável..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
	@echo "Executável criado em $(TARGET)"

# Regra genérica para compilar cada .cpp em .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(wildcard $(INCDIR)/*.h)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 'make build' agora só chama a regra do executável; o make decide se precisa compilar
.PHONY: build
build: $(TARGET)

# Execução com passagem de argumento
.PHONY: run
run: $(TARGET)
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

# Visualização das ondas no GTKWave (inalterado)
.PHONY: run_waves
run_waves:
	@if [ ! -f $(VCD_FILE) ]; then \
		echo "Erro: Arquivo $(VCD_FILE) não encontrado."; \
		echo "Execute primeiro: make run ARGS=<arquivo_de_instrucoes>"; \
		exit 1; \
	fi
	@echo "Abrindo $(VCD_FILE) no GTKWave..."
	gtkwave $(VCD_FILE)

# Limpeza
.PHONY: clean
clean:
	@echo "Limpando diretórios de build e waves..."
	rm -rf build waves
	@echo "Limpeza concluída."

# Ajuda
.PHONY: help
help:
	@echo "Makefile para compilar e simular o processador MIPS com SystemC."
	@echo ""
	@echo "Comandos disponíveis:"
	@echo "  make build            - Compila o projeto (apenas se necessário)"
	@echo "  make run ARGS=...     - Executa a simulação (compila se preciso)"
	@echo "                          Exemplo: make run ARGS=tests/teste1.txt"
	@echo "  make run_waves        - Abre o arquivo waves/simulacao.vcd no GTKWave"
	@echo "  make clean            - Remove os diretórios build/ e waves/"
	@echo "  make help             - Mostra esta mensagem"
	@echo "  make                  - Exibe sugestão para usar o help"