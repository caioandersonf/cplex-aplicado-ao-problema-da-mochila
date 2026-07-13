# Caminhos do CPLEX Studio 22.2 no macOS
CPLEX_STUDIO_DIR = /Applications/CPLEX_Studio222
SYSTEM           = arm64_osx
LIBFORMAT        = static_pic

# Compilador C++
CXX      = clang++

# Flags de Compilação
# O macro -DIL_STD é fundamental para usar a STL C++ padrão com o CPLEX Concert Technology
CXXFLAGS = -O3 -m64 -fPIC -fexceptions -DNDEBUG -DIL_STD -stdlib=libc++ \
           -I$(CPLEX_STUDIO_DIR)/cplex/include \
           -I$(CPLEX_STUDIO_DIR)/concert/include

# Flags de Linkagem
# Inclui os caminhos para as bibliotecas estáticas e os frameworks necessários no macOS
LDFLAGS  = -L$(CPLEX_STUDIO_DIR)/cplex/lib/$(SYSTEM)/$(LIBFORMAT) \
           -L$(CPLEX_STUDIO_DIR)/concert/lib/$(SYSTEM)/$(LIBFORMAT) \
           -lconcert -lilocplex -lcplex \
           -lm -lpthread \
           -framework CoreFoundation -framework IOKit -framework Accelerate

# Alvo padrão: compilar o executável
all: teste_cplex tsp_solver mochila_solver

# Compilar o executável
teste_cplex: teste_cplex.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

tsp_solver: tsp_solver.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

mochila_solver: mochila_solver.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Limpar o executável compilado
clean:
	rm -f teste_cplex tsp_solver mochila_solver


# Compilar e executar
run: teste_cplex
	./teste_cplex

