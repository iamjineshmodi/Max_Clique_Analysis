
#include <iostream>
#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

struct ReportStats {
    int largestCliqueSize = 0;
    int totalMaximalCliques = 0;
    unordered_map<int, int> cliqueDistribution;
};

class FastSet {
private:
    static const int BITS_PER_WORD = 64;
    static const int SHIFT = 6;
    static const int MASK = 0x3F;
    vector<uint64_t> words;
    int maxSize;
    
public:
    FastSet() : maxSize(0) {}
    
    FastSet(int n) : maxSize(n) {
        words.resize((n + BITS_PER_WORD - 1) / BITS_PER_WORD, 0);
    }
    
    void resize(int n) {
        maxSize = n;
        words.resize((n + BITS_PER_WORD - 1) / BITS_PER_WORD, 0);
        clear();
    }
    
    void clear() {
        fill(words.begin(), words.end(), 0);
    }
    void add(int x) {
        if (x < 0 || x >= maxSize) return;
        words[x >> SHIFT] |= (1U << (x & MASK));
    }
    
    void remove(int x) {
        if (x < 0 || x >= maxSize) return;
        words[x >> SHIFT] &= ~(1U << (x & MASK));
    }
    
    bool contains(int x) const {
        if (x < 0 || x >= maxSize) return false;
        return (words[x >> SHIFT] & (1U << (x & MASK))) != 0;
    }
    
    bool isEmpty() const {
        for (uint32_t word : words) {
            if (word != 0) return false;
        }
        return true;
    }
    
    int count() const {
        int cnt = 0;
        for (uint32_t word : words) {
            cnt += __builtin_popcount(word);
        }
        return cnt;
    }
    
    vector<int> toVector() const {
        vector<int> result;
        for (int i = 0; i < maxSize; i++) {
            if (contains(i)) {
                result.push_back(i);
            }
        }
        return result;
    }
};

vector<vector<int>> adj;
vector<int> indexToVertex;
vector<int> S, T;
int cnt = 1;

void printDebugInfo(const string& location, int i, const vector<int>& C, int n) {
    cerr << "Debug at " << location << ": vertex=" << i << ", n=" << n << ", C size=" << C.size() << endl;
    if (C.size() < 20) { 
        cerr << " C contents: ";
        for (int v : C) cerr << v << " ";
        cerr << endl;
    }
}

bool safeContains(const vector<int>& vec, int val) {
    if (vec.empty()) return false;
    return binary_search(vec.begin(), vec.end(), val);
}

vector<int> safeIntersection(const vector<int>& A, const vector<int>& B) {
    if (A.empty() || B.empty()) return {};
    
    vector<int> result;
    result.reserve(min(A.size(), B.size()));
    
    int i = 0, j = 0;
    while (i < A.size() && j < B.size()) {
        if (A[i] < B[j]) {
            i++;
        } else if (A[i] > B[j]) {
            j++;
        } else {
            result.push_back(A[i]);
            i++;
            j++;
        }
    }
    return result;
}

vector<int> safeDifference(const vector<int>& A, const vector<int>& B) {
    if (A.empty()) return {};
    if (B.empty()) return A;
    
    vector<int> result;
    result.reserve(A.size());
    
    int i = 0, j = 0;
    while (i < A.size()) {
        if (j == B.size() || A[i] < B[j]) {
            result.push_back(A[i]);
            i++;
        } else if (A[i] > B[j]) {
            j++;
        } else {
            i++;
            j++;
        }
    }
    return result;
}

void recordClique(const vector<int>& C, ReportStats& stats) {
    if (cnt % 10000 == 0) {
        cout << "Clique #" << cnt << ": ";
        for (int v : C) {
            if (v < 0 || v >= indexToVertex.size()) {
                cout << "INVALID(" << v << ") ";
            } else {
                cout << indexToVertex[v] << " ";
            }
        }
        cout << endl;
    }

    int cliqueSize = C.size();
    stats.totalMaximalCliques++;
    stats.cliqueDistribution[cliqueSize]++;
    stats.largestCliqueSize = max(stats.largestCliqueSize, cliqueSize);
    cnt++;
}

void UPDATE(int i, vector<int>& C, int n, ReportStats& stats, int depth = 0) {
    // Add depth check to prevent stack overflow
    if (depth > n + 10) {
        cerr << "Warning: Maximum recursion depth exceeded. Terminating branch." << endl;
        return;
    }
    
    if (i <= 0 || i > n + 1) {
        cerr << "Error: Invalid vertex index " << i << " (valid range: 1 to " << n << ")" << endl;
        return;
    }
    
    if (i > n) {
        recordClique(C, stats);
        return;
    }

    try {
        // C ∩ N(i)
        vector<int> CintersectNi = safeIntersection(C, adj[i]);
        
        // C - (C ∩ N(i))
        vector<int> CminusNi = safeDifference(C, CintersectNi);

        // If CminusNi is not empty, call UPDATE(i+1, C)
        if (!CminusNi.empty()) {
            UPDATE(i + 1, C, n, stats, depth + 1);
        }

        // C - {i}
        vector<int> Cminusi;
        Cminusi.reserve(C.size());
        for (int v : C) {
            if (v != i) {
                Cminusi.push_back(v);
            }
        }

        // Reset S and T arrays
        fill(S.begin(), S.end(), 0);
        fill(T.begin(), T.end(), 0);

        // Compute T[y]
        for (int x : CintersectNi) {
            for (int y : adj[x]) {
                if (!safeContains(Cminusi, y)) {
                    if (y >= 0 && y <= n) {
                        T[y]++;
                    }
                }
            }
        }

        // Compute S[y]
        for (int x : CminusNi) {
            for (int y : adj[x]) {
                if (!safeContains(C, y)) {
                    if (y >= 0 && y <= n) {
                        S[y]++;
                    }
                }
            }
        }

        bool FLAG = true;
        
        // Maximality test
        for (int y : adj[i]) {
            if (y < i && !safeContains(C, y) && y >= 0 && y <= n && T[y] == CintersectNi.size()) {
                FLAG = false;
                break;
            }
        }

        CminusNi.insert(CminusNi.begin(), 0); // Add 0 at beginning
        
        // Lexicographic test with additional bounds checking
        int p = CminusNi.size() - 1;
        for (int k = 1; k <= p && FLAG; k++) {
            int j_k = CminusNi[k];
            
            if (j_k < 0 || j_k > n || adj[j_k].size() == 0) {
                cerr << "Warning: Invalid j_k = " << j_k << " at depth " << depth << endl;
                continue;
            }
            
            for (int y : adj[j_k]) {
                if (y < i && !safeContains(C, y) && y >= 0 && y <= n && T[y] == CintersectNi.size()) {
                    if (y >= j_k) {
                        S[y]--;
                    } else {
                        auto it = lower_bound(CminusNi.begin() + 1, CminusNi.begin() + p + 1, y);
                        int first = (it != CminusNi.begin() + p + 1) ? (it - CminusNi.begin()) : -1;
                        
                        if (first != -1 && k == first) {
                            if ((S[y] + k - 1) == p) {
                                if (y >= (k - 1 > 0 ? CminusNi[k - 1] : 0)) {
                                    FLAG = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Final FLAG checks
        int j_p = (p > 0 ? CminusNi[p] : 0);
        if (!CintersectNi.empty() && FLAG) {
            for (int y = 1; y <= n; y++) {
                if (!safeContains(C, y) && y < i && T[y] == CintersectNi.size() && S[y] == 0) {
                    if (j_p < y) {
                        FLAG = false;
                        break;
                    }
                }
            }
        } else if (j_p < i - 1) {
            FLAG = false;
        }

        if (FLAG) {
            vector<int> SAVE = CminusNi;
            SAVE.erase(SAVE.begin());
            
            vector<int> newC = CintersectNi;
            newC.push_back(i);
            vector<int> oldC = C;
            
            C = newC;
            
            // Recursive call
            UPDATE(i + 1, C, n, stats, depth + 1);
            
            // Restore C
            vector<int> tempC = safeDifference(oldC, vector<int>{i});
            C = tempC;
            for (int v : SAVE) {
                C.push_back(v);
            }
        }
    }
    catch (const exception& e) {
        cerr << "Exception in UPDATE at depth " << depth << ": " << e.what() << endl;
        printDebugInfo("exception", i, C, n);
    }
}

void CLIQUE(int n, ReportStats& stats) {
    S.resize(n + 1, 0);
    T.resize(n + 1, 0);

    vector<int> C = {1};

    try {
        UPDATE(2, C, n, stats);
    }
    catch (const exception& e) {
        cerr << "Exception in CLIQUE: " << e.what() << endl;
    }
}

void generateReport(const ReportStats& stats, long long execTime) {
    cout << "=== MAXIMAL CLIQUE ANALYSIS REPORT ===" << endl;
    cout << "1. Largest size of clique: " << stats.largestCliqueSize << endl;
    cout << "2. Total number of maximal cliques: " << stats.totalMaximalCliques << endl;
    cout << "3. Execution time: " << execTime << " ms" << endl;
    cout << "4. Distribution of different size cliques:" << endl;
    
    vector<pair<int, int>> sortedDist(stats.cliqueDistribution.begin(), stats.cliqueDistribution.end());
    sort(sortedDist.begin(), sortedDist.end());
    
    for (const auto& element : sortedDist) {
        int size = element.first, count = element.second;
        cout << " Size " << size << ": " << count << " cliques" << endl;
    }
}

int main() {

    try {
        // Get filename with error handling
        string filename;
        cout << "Enter dataset name: ";
        cin >> filename;
        filename += ".txt";
        ios::sync_with_stdio(false);
        cin.tie(nullptr);
        cout.tie(nullptr);
        
        ifstream infile(filename);
        if (!infile) {
            cerr << "Error: Could not open file " << filename << endl;
            return 1;
        }

        cout << "Reading input file " << filename << endl;

        // Read graph parameters
        int n = 0, e = 0;
        if (!(infile >> n >> e)) {
            cerr << "Error: Failed to read n and e from file" << endl;
            return 1;
        }
        
        if (n <= 0 || e < 0 || n > 100000 || e > 10000000) {
            // cerr << "Warning: Suspicious graph parameters: n=" << n << ", e=" << e << endl;
            cout << "Continue with these parameters? (y/n): ";
            char response;
            cin >> response;
            if (response != 'y' && response != 'Y') {
                return 1;
            }
        }

        // Vertex mapping structures
        unordered_map<int, int> vertexToIndex;
        vertexToIndex.reserve(n);
        indexToVertex.clear();
        indexToVertex.push_back(0);
        
        vector<pair<int, int>> edges;
        edges.reserve(max(1000, min(e, 10000000)));

        cout << "Reading edges..." << endl;
        int u, v;
        int edgesRead = 0;
        
        while (edgesRead < e && infile >> u >> v) {
            if (u == v) continue;
            
            if (u < 0 || v < 0) {
            cerr << "Warning: Negative vertex ID found: " << u << " " << v << endl;
                continue;
            }
            
            edges.push_back({u, v});
            
            if (vertexToIndex.find(u) == vertexToIndex.end()) {
                int idx = vertexToIndex.size() + 1;
                vertexToIndex[u] = idx;
                indexToVertex.push_back(u);
            }
            
            if (vertexToIndex.find(v) == vertexToIndex.end()) {
                int idx = vertexToIndex.size() + 1;
                vertexToIndex[v] = idx;
                indexToVertex.push_back(v);
            }
            
            if (++edgesRead % 100000 == 0) {
                cout << "Read " << edgesRead << " edges so far" << endl;
            }
            
            if (edges.size() >= 10000000) {
                cerr << "Warning: Reached maximum edge limit (10M). Truncating input." << endl;
                break;
            }
        }
        
        cout << "Read " << edges.size() << " edges" << endl;

        int numVertices = vertexToIndex.size();
        cout << "Total unique vertices: " << numVertices << endl;
        
        if (numVertices == 0) {
            cerr << "Error: No vertices found in the graph" << endl;
            return 1;
        }

        vector<int> degree(numVertices + 1, 0);
        adj.clear();
        adj.resize(numVertices + 1);
        for (const auto& edge : edges) {
            int uIdx = vertexToIndex[edge.first];
            int vIdx = vertexToIndex[edge.second];
            
            if (uIdx <= 0 || uIdx > numVertices || vIdx <= 0 || vIdx > numVertices) {
                cerr << "Warning: Invalid vertex index: " << uIdx << " or " << vIdx << endl;
                continue;
            }
            
            degree[uIdx]++;
            degree[vIdx]++;
        }
        
        for (int i = 1; i <= numVertices; i++) {
            adj[i].reserve(degree[i]);
        }

        cout << "Building adjacency list..." << endl;
        for (const auto& edge : edges) {
            int uIdx = vertexToIndex[edge.first];
            int vIdx = vertexToIndex[edge.second];
            
            if (uIdx <= 0 || uIdx > numVertices || vIdx <= 0 || vIdx > numVertices) {
                continue; 
            }
            
            adj[uIdx].push_back(vIdx);
            adj[vIdx].push_back(uIdx);
        }

        // cout << "Sorting adjacency lists..." << endl;
        for (int i = 1; i <= numVertices; i++) {
            sort(adj[i].begin(), adj[i].end());
            auto newEnd = unique(adj[i].begin(), adj[i].end());
            adj[i].resize(distance(adj[i].begin(), newEnd));
        }

        cout << "Sorting vertices by degree..." << endl;
        vector<pair<int, int>> vertexDegrees;
        for (int i = 1; i <= numVertices; i++) {
            vertexDegrees.push_back({i, adj[i].size()});
        }
        
        sort(vertexDegrees.begin(), vertexDegrees.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 return a.second < b.second || (a.second == b.second && a.first < b.first);
             });

        vector<int> oldToNew(numVertices + 1);
        vector<int> newToOld(numVertices + 1);
        
        vector<int> newIndexToVertex;
        newIndexToVertex.push_back(0);
        
        for (int i = 0; i < numVertices; i++) {
            int oldIdx = vertexDegrees[i].first;
            int newIdx = i + 1;
            oldToNew[oldIdx] = newIdx;
            newToOld[newIdx] = oldIdx;
            newIndexToVertex.push_back(indexToVertex[oldIdx]);
        }

        cout << "Creating reordered adjacency list..." << endl;
        vector<vector<int>> newAdj(numVertices + 1);
        
        for (int i = 1; i <= numVertices; i++) {
            newAdj[i].reserve(adj[newToOld[i]].size());
        }
        
        for (int newI = 1; newI <= numVertices; newI++) {
            int oldI = newToOld[newI];
            for (int oldNeighbor : adj[oldI]) {
                newAdj[newI].push_back(oldToNew[oldNeighbor]);
            }
            sort(newAdj[newI].begin(), newAdj[newI].end());
        }

        adj = move(newAdj);
        indexToVertex = move(newIndexToVertex);

        // cout << "Validating adjacency lists..." << endl;
        bool valid = true;
        for (int i = 1; i <= numVertices; i++) {
            for (int neighbor : adj[i]) {
                if (neighbor <= 0 || neighbor > numVertices) {
                    // cerr << "Invalid neighbor " << neighbor << " for vertex " << i << endl;
                    valid = false;
                }
            }
        }
        
        if (!valid) {
            // cerr << "Warning: Invalid adjacency list detected" << endl;
            // cout << "Continue anyway? (y/n): ";
            char response;
            cin >> response;
            if (response != 'y' && response != 'Y') {
                return 1;
            }
        }

        // cout << "Reordered vertices by degree" << endl;

        // cout << "Starting clique finding algorithm..." << endl;
        ReportStats stats;
        
        auto start = high_resolution_clock::now();
        CLIQUE(numVertices, stats);
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<milliseconds>(end - start);
        generateReport(stats, duration.count());
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}