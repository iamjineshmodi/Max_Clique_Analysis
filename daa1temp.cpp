#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// Global variables
int n; // Number of vertices
vector<vector<int>> adjacencyList; // Adjacency list representation of the graph
vector<int> S, T; // Arrays for S and T as used in the pseudocode

// Function to compute the intersection of C and N(i)
vector<int> intersect(const vector<int>& C, const vector<int>& Ni) {
    vector<int> result;
    for (int v : C) {
        if (find(Ni.begin(), Ni.end(), v) != Ni.end()) {
            result.push_back(v);
        }
    }
    return result;
}

// Function to check if a clique is maximal
bool isMaximal(const vector<int>& C) {
    for (int v = 1; v <= n; v++) {
        if (find(C.begin(), C.end(), v) == C.end()) { // If v is not in C
            bool canExpand = true;
            for (int u : C) {
                if (find(adjacencyList[u].begin(), adjacencyList[u].end(), v) == adjacencyList[u].end()) {
                    canExpand = false;
                    break;
                }
            }
            if (canExpand) return false; // Not maximal since we can add v
        }
    }
    return true;
}

// Function to print a clique only if it's maximal
void printClique(const vector<int>& C) {
    if (C.size() >= 2 && isMaximal(C)) { // Ensure only **maximal** cliques are printed
        cout << "Maximal Clique found: { ";
        for (size_t i = 0; i < C.size(); i++) {
            cout << C[i] << " ";
        }
        cout << "}" << endl;
    }
    else {
    cout << "Clique { ";
    for (size_t i = 0; i < C.size(); i++) cout << C[i] << " ";
    cout << "} is NOT maximal" << endl;
}
}

// The UPDATE procedure
void UPDATE(int i, vector<int> C) {
    if (i > n) return; // Prevent infinite recursion

    // Step 1: Compute C - N(i)
    vector<int> CminusNi;
    for (int v : C) {
        if (find(adjacencyList[i].begin(), adjacencyList[i].end(), v) == adjacencyList[i].end()) {
            CminusNi.push_back(v);
        }
    }

    if (!CminusNi.empty()) {
        UPDATE(i + 1, C);
    }

    // Step 2: Compute T[y] = |N(y) ∩ C ∩ N(i)|
    vector<int> CintersectNi = intersect(C, adjacencyList[i]);
    for (int x : CintersectNi) {
        for (int y : adjacencyList[x]) {
            if (find(C.begin(), C.end(), y) == C.end() && y != i) {
                T[y]++;
            }
        }
    }

    // Step 3: Compute S[y] = |N(y) ∩ (C - N(i))|
    for (int x : CminusNi) {
        for (int y : adjacencyList[x]) {
            if (find(C.begin(), C.end(), y) == C.end()) {
                S[y]++;
            }
        }
    }

    bool FLAG = true;

    // Step 4: Maximality test
    for (int y : adjacencyList[i]) {
        if (y < i && find(C.begin(), C.end(), y) == C.end() && T[y] == (int)CintersectNi.size()) {
            FLAG = false;
            break;
        }
    }

    // Step 5: Sort C - N(i) in ascending order
    for (int x = 0; x < (int)C.size() - 1; x++) {
        for (int y = x + 1; y < (int)C.size(); y++) {
            if (C[x] > C[y]) {
                swap(C[x], C[y]);
            }
        }
    }

    // Step 6: Lexicographic test case S(y) >= 1
    for (int k = 0; k < (int)C.size(); k++) {
        for (int y = 0; y < n; y++) {
            if (adjacencyList[C[k]][y] && y < i && T[y] == (int)C.size()) {
                if (y >= C[k]) {
                    S[y]--;
                } else {
                    if (S[y] + k - 1 == (int)C.size() && y >= C[k - 1]) {
                        FLAG = false;
                    }
                }
            }
        }
    }

    // Step 7: Case S(y) = 0
    for (int y = 0; y < n; y++) {
        if (!adjacencyList[i][y] && y < i && T[y] == (int)C.size() && S[y] == 0) {
            if (C[C.size() - 1] < y || C[C.size() - 1] < i - 1) {
                FLAG = false;
            }
        }
    }

    // Step 8&9: Reset S and T
    for (int y = 0; y < n; y++) {
        T[y] = 0;
        S[y] = 0;
    }

    // Step 10: If FLAG is true, update clique
    if (FLAG) {
        vector<int> SAVE = CminusNi;
        C = CintersectNi;
        C.push_back(i); // Maintain order
        printClique(C); // Only print maximal cliques
        UPDATE(i + 1, C);
    }
}

// The main CLIQUE procedure
void CLIQUE() {
    vector<int> C;
    UPDATE(1, C);
}

// Function to read the graph edges properly
void readGraphEdges() {
    adjacencyList.resize(n + 1);

    int u, v;
    while (cin >> u >> v) {
        adjacencyList[u].push_back(v);
        adjacencyList[v].push_back(u); // Undirected graph
    }
}

int main() {
    cin >> n; // Read number of vertices
    readGraphEdges();

    // Resize arrays
    S.resize(n + 1, 0);
    T.resize(n + 1, 0);

    CLIQUE();

    return 0;
}

