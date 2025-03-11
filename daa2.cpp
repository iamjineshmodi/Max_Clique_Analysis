#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

ofstream outfile("output.txt");


template<typename T>
void Print(const set<T>& s) {
    cout << "{";
    bool first = true;
    for(const auto& element : s) {
        if (!first) {
            cout << ", ";
        }
        cout << element;  // No need to adjust indexing
        first = false;
    }
    cout << "}" << endl;
}

void BronKerbosch(set<int>& R, set<int>& P, set<int>& X, const vector<set<int>>& adj) {
    if(P.empty() && X.empty()) {
        Print(R);
        
        return;
    }

    int pivot = -1;
    size_t max_connections = 0;
    for(int u : P) {
        size_t connections = 0;
        for(int v : P) {
            if(adj[u].count(v)) {
                connections++;
            }
        }
        if(connections > max_connections) {
            max_connections = connections;
            pivot = u;
        }
    }

    set<int> Pcopy = P;
    if(pivot != -1) {
        for(int v : adj[pivot]) {
            Pcopy.erase(v);
        }
    }

    for(int v : Pcopy) {
        set<int> newR = R;
        newR.insert(v);
        set<int> newP, newX;
        
        for(int w : P) {
            if(adj[v].count(w)) {
                newP.insert(w);
            }
        }
        for(int w : X) {
            if(adj[v].count(w)) {
                newX.insert(w);
            }
        }
        
        BronKerbosch(newR, newP, newX, adj);
        P.erase(v);
        X.insert(v);
    }
}

void BronKerboschDegeneracy(int n, const vector<set<int>>& adj) {
    if(n == 0) {
        return;
    }

        cout << "Maximal cliques:" << endl;
        
    vector<int> order(n + 1);  // 1-based indexing
    vector<int> degree(n + 1);
    set<int> P, X;
    
    for(int i = 1; i <= n; i++) {  // 1-based loop
        P.insert(i);
        degree[i] = adj[i].size();
    }

    vector<bool> visited(n + 1, false);  // 1-based indexing
    for(int i = 1; i <= n; i++) {  // 1-based loop
        int min_degree_node = -1;
        for(int j = 1; j <= n; j++) {  // 1-based loop
            if(!visited[j] && (min_degree_node == -1 || degree[j] < degree[min_degree_node])) {
                min_degree_node = j;
            }
        }
        order[i] = min_degree_node;
        visited[min_degree_node] = true;
        for(int neighbor : adj[min_degree_node]) {
            if(!visited[neighbor]) {
                degree[neighbor]--;
            }
        }
    }

    for(int i = 1; i <= n; i++) {  // 1-based loop
        int v = order[i];
        set<int> Pv, Xv;
        for(int w : adj[v]) {
            if(P.count(w)) {
                Pv.insert(w);
            }
            if(X.count(w)) {
                Xv.insert(w);
            }
        }
        set<int> R = {v};
        BronKerbosch(R, Pv, Xv, adj);
        P.erase(v);
        X.insert(v);
    }
}

int main() {
    ifstream infile("input.txt");
    if(!infile) {
        cerr << "Error opening input.txt" << endl;
        return 1;
    }

    int n, e;
    infile >> n >> e;
    
    if(n < 0 || e < 0 || e > n * (n-1) / 2) {
        cout << "Invalid input" << endl;
        return 1;
    }

    vector<set<int>> adj(n + 1);  // 1-based indexing
    for(int i = 0; i < e; i++) {
        int u, v;
        infile >> u >> v;
        if(u < 1 || u > n || v < 1 || v > n || u == v) {
            cout << "Invalid edge" << endl;
            return 1;
        }
        adj[u].insert(v);
        adj[v].insert(u);
    }

    
    auto start = high_resolution_clock::now();
    BronKerboschDegeneracy(n, adj);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Execution time: " << duration.count() << "ms" << endl;

    return 0;
}