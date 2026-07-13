#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

class KnapsackInstance {
private:
    std::string filename;
    int N;
    double capacity;
    std::vector<double> values;
    std::vector<double> weights;
    std::vector<std::string> item_names;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

public:
    KnapsackInstance() : N(0), capacity(0.0) {}

    bool load(const std::string& filename) {
        // Clear all state before loading new file
        this->filename = filename;
        N = 0;
        capacity = 0.0;
        values.clear();
        weights.clear();
        item_names.clear();

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        // Inspect the file to decide format: check if it contains the required headers "N:" and "CAPACITY:"
        bool has_header_keys = false;
        std::string test_line;
        bool found_n = false;
        bool found_capacity = false;
        int line_count = 0;
        while (std::getline(file, test_line) && line_count < 50) {
            std::string trimmed = trim(test_line);
            if (trimmed.empty() || trimmed[0] == '#') continue;
            line_count++;
            
            size_t colon = trimmed.find(':');
            if (colon != std::string::npos) {
                std::string key = trim(trimmed.substr(0, colon));
                std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                if (key == "N" || key == "ITEMS_COUNT") found_n = true;
                if (key == "CAPACITY") found_capacity = true;
            }
            if (found_n && found_capacity) {
                has_header_keys = true;
                break;
            }
        }

        // Reset file stream to beginning
        file.clear();
        file.seekg(0, std::ios::beg);

        if (has_header_keys) {
            // Key-Value header format (like our generated files)
            std::string line;
            bool found_items_marker = false;
            while (std::getline(file, line)) {
                std::string trimmed = trim(line);
                if (trimmed.empty() || trimmed[0] == '#') continue;

                size_t colon = trimmed.find(':');
                if (colon != std::string::npos) {
                    std::string key = trim(trimmed.substr(0, colon));
                    std::string val = trim(trimmed.substr(colon + 1));
                    std::transform(key.begin(), key.end(), key.begin(), ::toupper);

                    try {
                        if (key == "N" || key == "ITEMS_COUNT") {
                            N = std::stoi(val);
                        } else if (key == "CAPACITY") {
                            capacity = std::stod(val);
                        }
                    } catch (...) {
                        std::cerr << "Error: Invalid numeric value for " << key << ": " << val << std::endl;
                        return false;
                    }
                } else if (trimmed == "ITEMS" || trimmed == "DATA") {
                    found_items_marker = true;
                    break;
                }
            }

            if (!found_items_marker) {
                std::cerr << "Error: ITEMS or DATA marker not found in header-based file." << std::endl;
                return false;
            }

            if (N <= 0 || capacity <= 0.0) {
                std::cerr << "Error: Invalid N (" << N << ") or capacity (" << capacity << ")." << std::endl;
                return false;
            }

            values.resize(N);
            weights.resize(N);
            item_names.resize(N);

            int count = 0;
            while (std::getline(file, line) && count < N) {
                std::string trimmed = trim(line);
                if (trimmed.empty() || trimmed[0] == '#') continue;

                std::stringstream ss(trimmed);
                double val_v, val_w;
                std::string name_opt;
                
                std::vector<std::string> tokens;
                std::string token;
                while (ss >> token) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 3) {
                    name_opt = tokens[0];
                    for (size_t t = 1; t < tokens.size() - 2; ++t) {
                        name_opt += " " + tokens[t];
                    }
                    try {
                        val_v = std::stod(tokens[tokens.size() - 2]);
                        val_w = std::stod(tokens[tokens.size() - 1]);
                    } catch (...) {
                        std::cerr << "Error parsing line: " << line << std::endl;
                        return false;
                    }
                } else if (tokens.size() == 2) {
                    name_opt = "Item_" + std::to_string(count + 1);
                    try {
                        val_v = std::stod(tokens[0]);
                        val_w = std::stod(tokens[1]);
                    } catch (...) {
                        std::cerr << "Error parsing line: " << line << std::endl;
                        return false;
                    }
                } else {
                    std::cerr << "Error: Item data must contain either 2 or more values. Line: " << line << std::endl;
                    return false;
                }

                values[count] = val_v;
                weights[count] = val_w;
                item_names[count] = name_opt;
                count++;
            }

            if (count < N) {
                std::cerr << "Error: Expected " << N << " items, but only read " << count << "." << std::endl;
                return false;
            }
        } else {
            // Standard KPLIB format (space/newline separated numbers)
            if (!(file >> N >> capacity)) {
                std::cerr << "Error: Could not read N and capacity from file." << std::endl;
                return false;
            }

            if (N <= 0 || capacity <= 0.0) {
                std::cerr << "Error: Invalid N (" << N << ") or capacity (" << capacity << ") in standard format." << std::endl;
                return false;
            }

            values.resize(N);
            weights.resize(N);
            item_names.resize(N);

            for (int i = 0; i < N; ++i) {
                double val_v, val_w;
                if (!(file >> val_v >> val_w)) {
                    std::cerr << "Error: Could not read value and weight for item " << i + 1 << "." << std::endl;
                    return false;
                }
                values[i] = val_v;
                weights[i] = val_w;
                item_names[i] = "Item_" + std::to_string(i + 1);
            }
        }

        // Validate weights and values
        for (int i = 0; i < N; ++i) {
            if (weights[i] < 0.0) {
                std::cerr << "Error: Item " << item_names[i] << " (index " << i + 1 << ") has negative weight: " << weights[i] << "." << std::endl;
                return false;
            }
            if (weights[i] == 0.0 && values[i] > 0.0) {
                std::cerr << "Warning: Item " << item_names[i] << " (index " << i + 1 << ") has zero weight but positive value: " << values[i] << "." << std::endl;
            }
            if (values[i] < 0.0) {
                std::cerr << "Warning: Item " << item_names[i] << " (index " << i + 1 << ") has negative value: " << values[i] << "." << std::endl;
            }
        }

        // Check for extra trailing data (non-whitespace, non-comment garbage)
        std::string remaining_text;
        bool has_extra = false;
        while (std::getline(file, remaining_text)) {
            std::string trimmed = trim(remaining_text);
            if (!trimmed.empty() && trimmed[0] != '#') {
                has_extra = true;
                break;
            }
        }
        if (has_extra) {
            std::cerr << "Warning: Extra data found at the end of the file after reading the expected " << N << " items." << std::endl;
        }

        file.close();
        return true;
    }

    std::string get_filename() const { return filename; }
    int get_N() const { return N; }
    double get_capacity() const { return capacity; }
    double get_value(int i) const { return values[i]; }
    double get_weight(int i) const { return weights[i]; }
    std::string get_name(int i) const { return item_names[i]; }
};

class KnapsackSolver {
public:
    static void solve(const KnapsackInstance& instance) {
        int N = instance.get_N();
        double W = instance.get_capacity();
        IloEnv env;

        try {
            IloModel model(env);

            // Decision variables: x[i] = 1 if item i is selected, 0 otherwise
            IloBoolVarArray x(env, N);

            // Objective: Maximize total value
            IloExpr obj_expr(env);
            for (int i = 0; i < N; ++i) {
                obj_expr += instance.get_value(i) * x[i];
            }
            model.add(IloMaximize(env, obj_expr));
            obj_expr.end();

            // Constraint: Total weight <= capacity
            IloExpr weight_expr(env);
            for (int i = 0; i < N; ++i) {
                weight_expr += instance.get_weight(i) * x[i];
            }
            model.add(weight_expr <= W);
            weight_expr.end();

            IloCplex cplex(model);
            cplex.setOut(env.getNullStream()); // Suppress CPLEX log
            cplex.setParam(IloCplex::Param::Threads, 1);

            cplex.exportModel("knapsack_model.lp");

            auto start_time = std::chrono::high_resolution_clock::now();
            
            if (!cplex.solve()) {
                std::cerr << "Model is infeasible or solver failed." << std::endl;
                env.end();
                return;
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(end_time - start_time).count();

            // Output Results
            double total_value = cplex.getObjValue();
            double total_weight = 0.0;
            std::vector<int> selected_items;

            for (int i = 0; i < N; ++i) {
                if (cplex.getValue(x[i]) > 0.5) {
                    selected_items.push_back(i);
                    total_weight += instance.get_weight(i);
                }
            }

            std::cout << "========================================" << std::endl;
            std::cout << "MOCHILA SOLVER STATISTICS" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "Arquivo de Instância:         " << instance.get_filename() << std::endl;
            std::cout << "Total de Itens:               " << N << std::endl;
            std::cout << "Itens Selecionados:           " << selected_items.size() 
                      << " (" << (double)selected_items.size() / N * 100.0 << "%)" << std::endl;
            std::cout << "Capacidade Máxima da Mochila: " << W << std::endl;
            std::cout << "Capacidade Utilizada:         " << total_weight 
                      << " (" << total_weight / W * 100.0 << "%)" << std::endl;
            std::cout << "Lucro Ótimo:                  " << total_value << std::endl;
            std::cout << "Tempo de Execução:            " << elapsed << " segundos" << std::endl;
            std::cout << "========================================" << std::endl;

            std::cout << "Detalhe dos Itens Selecionados:" << std::endl;
            std::cout << "ID/Nome\tLucro/Valor\tPeso" << std::endl;
            for (int idx : selected_items) {
                std::cout << instance.get_name(idx) << "\t" 
                          << instance.get_value(idx) << "\t\t" 
                          << instance.get_weight(idx) << std::endl;
            }
            std::cout << "========================================" << std::endl;

        } catch (IloException& e) {
            std::cerr << "CPLEX exception: " << e << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception" << std::endl;
        }
        env.end();
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <instance_file>" << std::endl;
        return 1;
    }
    KnapsackInstance instance;
    if (!instance.load(argv[1])) {
        std::cerr << "Failed to load instance: " << argv[1] << std::endl;
        return 1;
    }
    KnapsackSolver::solve(instance);
    return 0;
}
