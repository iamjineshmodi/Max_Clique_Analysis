#include<bits/stdc++.h>


using namespace std;
using namespace std::chrono;

ofstream outfile("output.txt");

template<typename T>
void Print(const vector<T>& s)
{
    outfile << "[";
    for(const auto& element : s)
    {
        outfile << element << ", ";
    }
    outfile << "]";
}

class Graph {
private:
    int n; // number of vertices
    vector<set<int>> adj; // adjacency list using sets
    vector<int> Q; // current clique being formed
    vector<vector<int>> maximalCliques; // store all maximal cliques

    vector<int> getIntersection(const vector<int>& vertices, int v) {
        vector<int> result;
        for (int i : vertices) {
            if (adj[v-1].count(i)) {
                result.push_back(i);
            }
        }
        return result;
    }

    bool isMaximal(const vector<int>& clique) {
        for (int v = 1; v <= n; v++) {
            if (find(clique.begin(), clique.end(), v) != clique.end()) {
                continue;
            }

            bool canAdd = true;
            for (int u : clique) {
                if (!adj[v-1].count(u)) {
                    canAdd = false;
                    break;
                }
            }
            if (canAdd) {
                return false;
            }
        }
        return true;
    }

    void expand(vector<int>& SUBG, vector<int>& CAND) {
        if (SUBG.empty()) {
            if (!Q.empty() && isMaximal(Q)) {
                maximalCliques.push_back(Q);
            }
            return;
        }

        // Maximizing |CAND ∩ Γ(u)|
        int max_count = -1;
        int u = -1;
        for (int v : SUBG) {
            int count = 0;
            for (int w : CAND) {
                if (adj[v-1].count(w)) count++;
            }
            if (count > max_count) {
                max_count = count;
                u = v;
            }
        }

        // ext = CAND - Γ(u)
        vector<int> ext;
        for (int q : CAND) {
            if (!adj[u-1].count(q)) {
                ext.push_back(q);
            }
        }

        while (!ext.empty()) {
            int q = ext.back();
            ext.pop_back();

            Q.push_back(q);
            vector<int> newSUBG = getIntersection(SUBG, q);
            vector<int> newCAND = getIntersection(CAND, q);

            expand(newSUBG, newCAND);

            CAND.erase(remove(CAND.begin(), CAND.end(), q), CAND.end());
            Q.pop_back();
        }
    }

public:
    Graph(int vertices) : n(vertices) {
        adj.resize(n); 
    }

    void addEdge(int v, int w) {
        adj[v-1].insert(w);
        adj[w-1].insert(v);
    }

    void findMaximalCliques() {
        vector<int> V(n);
        for (int i = 1; i <= n; i++) V[i-1] = i;  
        Q.clear();
        maximalCliques.clear();
        expand(V, V);
        for(const auto& clique : maximalCliques)
        {
            Print(clique);
            outfile << endl;
        }

        cout << "Maximal cliques:" << endl;
        for (const auto& clique : maximalCliques) {
            cout << "{";
            for (size_t i = 0; i < clique.size(); i++) {
                cout << clique[i];  
                if (i < clique.size() - 1) cout << ", ";
            }
            cout << "}" << endl;
        }
    }
};

int main() {
    ifstream infile("input.txt");
    if (!infile) {
        cerr << "Error opening input.txt" << endl;
        return 1;
    }

    int n, e;
    infile >> n >> e;
    
    if (n < 0 || e < 0 || e > n * (n-1) / 2) {
        cout << "Invalid input" << endl;
        return 1;
    }

    Graph g(n);
    
    for (int i = 0; i < e; i++) {
        int u, v;
        infile >> u >> v;
        if (u < 1 || u > n || v < 1 || v > n || u == v) {
            cout << "Invalid edge" << endl;
            return 1;
        }
        g.addEdge(u, v);
    }

    auto start = high_resolution_clock::now();
    g.findMaximalCliques();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Execution time: " << duration.count() << "ms" << endl;

    return 0;
}