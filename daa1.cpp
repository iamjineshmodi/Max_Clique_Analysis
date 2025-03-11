#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// Global variables
int n;
vector<set<int>> adjacencyList;
vector<int> S, T;

template<typename T>
void Print(const set<T>& s, ofstream& outfile) {
    if (!outfile.is_open()) return;
    cout << "{";
    bool first = true;
    for(const auto& element : s) {
        if (!first) {
            cout << ", ";
        }
        cout << element;
        first = false;
    }
    cout << "}" << endl;
}

set<int> intersect(const set<int>& C, const set<int>& Ni) {
    set<int> result;
    if (C.empty() || Ni.empty()) return result;
    for (int v : C) {
        if (Ni.find(v) != Ni.end()) {
            result.insert(v);
        }
    }
    return result;
}

bool isMaximal(const set<int>& C) {
    if (C.empty()) return false;
    
    for (int v = 1; v <= n; v++) {
        if (v > (int)adjacencyList.size() - 1) continue;
        if (C.find(v) == C.end()) {
            bool canExpand = true;
            for (int u : C) {
                if (u >= (int)adjacencyList.size() || 
                    adjacencyList[u].find(v) == adjacencyList[u].end()) {
                    canExpand = false;
                    break;
                }
            }
            if (canExpand) return false;
        }
    }
    return true;
}

void printClique(const set<int>& C, ofstream& outfile) {
    if (!outfile.is_open()) return;
    if (C.size() >= 2 && isMaximal(C)) {
        Print(C, outfile);
    }
}

void UPDATE(int i, set<int> C, ofstream& outfile) {
    if (i > n || i >= (int)adjacencyList.size()) return;
    if (!outfile.is_open()) return;

    // Step 1: Compute C - N(i)
    set<int> CminusNi;
    for (int v : C) {
        if (adjacencyList[i].find(v) == adjacencyList[i].end()) {
            CminusNi.insert(v);
        }
    }

    if (!CminusNi.empty()) {
        UPDATE(i + 1, C, outfile);
    }

    // Safety check for array bounds
    if (S.size() != n + 1 || T.size() != n + 1) {
        cerr << "Array size mismatch" << endl;
        return;
    }

    // Step 2: Compute T[y] = |N(y) ∩ C ∩ N(i)|
    set<int> CintersectNi = intersect(C, adjacencyList[i]);
    for (int x : CintersectNi) {
        if (x >= (int)adjacencyList.size()) continue;
        for (int y : adjacencyList[x]) {
            if (y < (int)T.size() && C.find(y) == C.end() && y != i) {
                T[y]++;
            }
        }
    }

    // Step 3: Compute S[y] = |N(y) ∩ (C - N(i))|
    for (int x : CminusNi) {
        if (x >= (int)adjacencyList.size()) continue;
        for (int y : adjacencyList[x]) {
            if (y < (int)S.size() && C.find(y) == C.end()) {
                S[y]++;
            }
        }
    }

    bool FLAG = true;

    // Step 4: Maximality test
    for (const auto& y : adjacencyList[i]) {
        if (y >= (int)T.size()) continue;
        if (y < i && C.find(y) == C.end() && T[y] == (int)CintersectNi.size()) {
            FLAG = false;
            break;
        }
    }

    // Step 5: Sort C in ascending order
    vector<int> sortedC(C.begin(), C.end());
    for (int x = 0; x < (int)sortedC.size() - 1; x++) {
        for (int y = x + 1; y < (int)sortedC.size(); y++) {
            if (sortedC[x] > sortedC[y]) {
                swap(sortedC[x], sortedC[y]);
            }
        }
    }

    // Step 6: Lexicographic test
    for (int k = 0; k < (int)sortedC.size(); k++) {
        if (sortedC[k] >= (int)adjacencyList.size()) continue;
        for (int y = 0; y < n && y < (int)S.size() && y < (int)T.size(); y++) {
            if (adjacencyList[sortedC[k]].find(y) != adjacencyList[sortedC[k]].end() && 
                y < i && T[y] == (int)C.size()) {
                if (y >= sortedC[k]) {
                    S[y]--;
                } else {
                    if (k > 0 && S[y] + k - 1 == (int)C.size() && 
                        y >= sortedC[k - 1]) {
                        FLAG = false;
                    }
                }
            }
        }
    }

    // Step 7: Case S(y) = 0
    for (int y = 0; y < n && y < (int)S.size() && y < (int)T.size(); y++) {
        if (adjacencyList[i].find(y) == adjacencyList[i].end() && 
            y < i && T[y] == (int)C.size() && S[y] == 0) {
            if (!C.empty() && (*C.rbegin() < y || *C.rbegin() < i - 1)) {
                FLAG = false;
            }
        }
    }

    // Reset S and T
    fill(S.begin(), S.end(), 0);
    fill(T.begin(), T.end(), 0);

    if (FLAG) {
        set<int> SAVE = CminusNi;
        C = CintersectNi;
        C.insert(i);
        printClique(C, outfile);
        UPDATE(i + 1, C, outfile);
    }
}

void CLIQUE(ofstream& outfile) {
    if (!outfile.is_open()) return;
    set<int> C;
    UPDATE(1, C, outfile);
}

int main() {
    ifstream infile("input.txt");
    if (!infile) {
        cerr << "Error opening input.txt" << endl;
        return 1;
    }

    ofstream outfile("output.txt");
    if (!outfile) {
        cerr << "Error opening output.txt" << endl;
        return 1;
    }

    int e;
    infile >> n;
    if (infile.fail()) {
        cerr << "Error reading number of vertices" << endl;
        return 1;
    }
    
    infile >> e;
    if (infile.fail()) {
        cerr << "Error reading number of edges" << endl;
        return 1;
    }

    if (n <= 0 || e < 0 || e > n * (n-1) / 2) {
        cerr << "Invalid input: n=" << n << ", e=" << e << endl;
        return 1;
    }

    // Initialize with proper sizes
    adjacencyList.clear();
    adjacencyList.resize(n + 1);
    S.clear();
    S.resize(n + 1, 0);
    T.clear();
    T.resize(n + 1, 0);

    // Read edges
    for (int i = 0; i < e; i++) {
        int u, v;
        infile >> u >> v;
        if (infile.fail()) {
            cerr << "Error reading edge " << i+1 << endl;
            return 1;
        }
        
        if (u < 1 || u > n || v < 1 || v > n || u == v) {
            cerr << "Invalid edge: " << u << " " << v << endl;
            return 1;
        }
        
        adjacencyList[u].insert(v);
        adjacencyList[v].insert(u);
    }

    cout << "Graph loaded successfully." << endl;
    cout << "Vertices: " << n << ", Edges: " << e << endl;

    auto start = high_resolution_clock::now();
    CLIQUE(outfile);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Execution time: " << duration.count() << "ms" << endl;

    infile.close();
    outfile.close();
    return 0;
}