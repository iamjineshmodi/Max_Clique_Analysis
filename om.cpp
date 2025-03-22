#include "bits/stdc++.h"
using namespace std;
ofstream outfile("output_daa1_2.txt");

static const int MAXN = 10000000;
int n;
vector<vector<int>> adjacencyList;
vector<int> S, T;
bool FLAG;
int ans = 0;
vector<int> vertexMap;
unordered_map<int, int> reverseMap;

struct IntHash {
    size_t operator()(const int &x) const { return x; }
};

// Optimized set operations using reserve to pre-allocate memory
unordered_set<int, IntHash> setDifference(const unordered_set<int, IntHash> &A, const unordered_set<int, IntHash> &B) {
    unordered_set<int, IntHash> diff;
    diff.reserve(A.size());
    for (auto &elem : A) if (B.find(elem) == B.end()) diff.insert(elem);
    return diff;
}

unordered_set<int, IntHash> setUnion(const unordered_set<int, IntHash> &A, const unordered_set<int, IntHash> &B) {
    unordered_set<int, IntHash> result = A;
    result.reserve(A.size() + B.size());
    for (auto &elem : B) result.insert(elem);
    return result;
}

unordered_set<int, IntHash> setIntersection(const unordered_set<int, IntHash> &A, const unordered_set<int, IntHash> &B) {
    const auto &smaller = (A.size() < B.size()) ? A : B;
    const auto &larger = (A.size() < B.size()) ? B : A;
    unordered_set<int, IntHash> intersect;
    intersect.reserve(smaller.size());
    for (auto &elem : smaller) if (larger.find(elem) != larger.end()) intersect.insert(elem);
    return intersect;
}

vector<unordered_set<int, IntHash>> precomputedNeighbors;

void printClique(const unordered_set<int, IntHash> &C) {
    // Only print if we're not handling a very large graph
    if (n < 1000) {
        cout << "Clique: ";
        vector<int> sortedC(C.begin(), C.end());
        sort(sortedC.begin(), sortedC.end());
        for (int v : sortedC) cout << reverseMap[v] << " ";
        cout << "\n";
    }
    ans++;
    
    // For large graphs, periodically output progress
    if (ans % 10000 == 0) {
        cout << "Found " << ans << " cliques so far...\n";
    }
}

void reorderVerticesByDegree() {
    vector<pair<int, int>> vertexDegrees;
    vertexDegrees.reserve(n);
    for (int i = 1; i <= n; i++) {
        vertexDegrees.push_back({ i, (int)adjacencyList[i].size() });
    }
    sort(vertexDegrees.begin(), vertexDegrees.end(),
        [](auto &a, auto &b) {
            return a.second < b.second || (a.second == b.second && a.first < b.first);
        });
    vector<vector<int>> newAdjList(n + 1);
    unordered_map<int, int> oldToNew, newToOld;
    for (int i = 0; i < n; i++){
        int oldIndex = vertexDegrees[i].first;
        int newIndex = i + 1;
        oldToNew[oldIndex] = newIndex;
        newToOld[newIndex] = oldIndex;
    }
    for (int i = 1; i <= n; i++){
        newAdjList[oldToNew[i]].reserve(adjacencyList[i].size());
        for (auto neighbor : adjacencyList[i]) {
            if (oldToNew.find(neighbor) != oldToNew.end()) {
                newAdjList[oldToNew[i]].push_back(oldToNew[neighbor]);
            }
        }
    }
    adjacencyList = move(newAdjList);
    unordered_map<int, int> newReverseMap;
    for (int i = 1; i <= n; i++) {
        newReverseMap[i] = reverseMap[newToOld[i]];
    }
    reverseMap = move(newReverseMap);
    for (int i = 1; i <= n; i++) {
        sort(adjacencyList[i].begin(), adjacencyList[i].end());
    }
}

const unordered_set<int, IntHash> &getNeighbors(int i) {
    return precomputedNeighbors[i];
}

// Define a structure to hold the state for iterative approach
struct State {
    int i;
    unordered_set<int, IntHash> C;
    unordered_set<int, IntHash> SAVE;
    bool processingBranch1;
    bool processingBranch2;
    bool needToRestoreC;
    
    State(int _i, const unordered_set<int, IntHash>& _C) 
        : i(_i), C(_C), processingBranch1(false), processingBranch2(false), needToRestoreC(false) {}
};

// Iterative UPDATE function
void UPDATE_Iterative(int startI, unordered_set<int, IntHash> &startC) {
    stack<State> stateStack;
    stateStack.emplace(startI, startC);
    
    while (!stateStack.empty()) {
        State &current = stateStack.top();
        
        // Check if we've reached the end condition
        if (current.i == n + 1) {
            printClique(current.C);
            stateStack.pop();
            continue;
        }
        
        // If we need to restore C during backtracking
        if (current.needToRestoreC) {
            unordered_set<int, IntHash> tempC = current.C;
            tempC.erase(current.i);
            current.C = setUnion(tempC, current.SAVE);
            stateStack.pop();
            continue;
        }
        
        // First branch execution: C - N(i) is not empty
        if (!current.processingBranch1) {
            current.processingBranch1 = true;
            
            const auto &Ni = getNeighbors(current.i);
            current.SAVE = setDifference(current.C, Ni);
            
            if (!current.SAVE.empty()) {
                stateStack.emplace(current.i + 1, current.C);
                continue;
            }
        }
        
        // Second branch execution: Apply conditions and possibly add i to clique
        if (!current.processingBranch2) {
            current.processingBranch2 = true;
            
            const auto &Ni = getNeighbors(current.i);
            auto CminusNi = current.SAVE;  // We already computed this
            auto CintersectNi = setIntersection(current.C, Ni);
            auto Cminusi = current.C;
            Cminusi.erase(current.i);
            
            // Reset S and T vectors
            fill(T.begin(), T.end(), 0);
            fill(S.begin(), S.end(), 0);
            
            for (auto x : CintersectNi) {
                const auto &Nxi = getNeighbors(x);
                for (auto y : setDifference(Nxi, Cminusi)) if (y >= 1 && y <= n) T[y]++;
            }
            for (auto x : CminusNi) {
                const auto &Nxi = getNeighbors(x);
                for (auto y : setDifference(Nxi, current.C)) if (y >= 1 && y <= n) S[y]++;
            }
            
            FLAG = true;
            for (auto y : setDifference(Ni, current.C)) {
                if (y < current.i && T[y] == (int)CintersectNi.size()) { FLAG = false; break; }
            }
            
            vector<int> cMinusNiVec(CminusNi.begin(), CminusNi.end());
            cMinusNiVec.push_back(0);
            sort(cMinusNiVec.begin(), cMinusNiVec.end());
            int p = (int)cMinusNiVec.size() - 1;
            
            for (int k = 0; k <= p && FLAG; k++) {
                int j_k = cMinusNiVec[k];
                const auto &Nxjk = getNeighbors(j_k);
                for (auto y : setDifference(Nxjk, current.C)) {
                    if (y < current.i && T[y] == (int)CintersectNi.size()) {
                        if (y >= j_k) { S[y]--; }
                        else {
                            int first = -1;
                            for (int m = 0; m <= p; m++) {
                                if (y < cMinusNiVec[m]) { first = m; break; }
                            }
                            if (first != -1 && k == first) {
                                if ((S[y] + k - 1) == p) {
                                    if (y >= (k - 1 > 0 ? cMinusNiVec[k - 1] : 0)) {
                                        FLAG = false; break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            int j_p = (p > 0 ? cMinusNiVec[p] : 0);
            if (!CintersectNi.empty() && FLAG) {
                for (int y = 1; y <= n; y++) {
                    if (current.C.find(y) == current.C.end() && y != current.i && y < current.i) {
                        if (T[y] == (int)CintersectNi.size() && S[y] == 0) {
                            if (j_p < y) { FLAG = false; break; }
                        }
                    }
                }
            } else if (j_p < current.i - 1) {
                FLAG = false;
            }
            
            if (FLAG) {
                auto newC = setIntersection(current.C, Ni);
                newC.insert(current.i);
                current.C = move(newC);
                current.needToRestoreC = true;
                stateStack.emplace(current.i + 1, current.C);
                continue;
            }
        }
        
        // If we've processed both branches or conditions weren't met
        stateStack.pop();
    }
}

void CLIQUE() {
    // Reorder once
    reorderVerticesByDegree();
    
    // Build neighbor sets once
    precomputedNeighbors.resize(n + 1);
    for (int i = 1; i <= n; i++) {
        precomputedNeighbors[i].reserve(adjacencyList[i].size());
        for (auto neighbor : adjacencyList[i]) {
            precomputedNeighbors[i].insert(neighbor);
        }
    }
    
    // Initialize S and T with correct size
    S.assign(n + 1, 0);
    T.assign(n + 1, 0);
    
    unordered_set<int, IntHash> C;
    if (n > 0) C.insert(1);
    
    // Progress tracking
    cout << "Starting clique detection for " << n << " vertices...\n";
    auto startTime = chrono::high_resolution_clock::now();
    
    UPDATE_Iterative(2, C);
    
    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = endTime - startTime;
    cout << "Clique detection completed in " << elapsed.count() << " seconds\n";
}

int main() {
    // ios::sync_with_stdio(false);
    // cin.tie(nullptr);
    
    string dataset_name;
    cin >> dataset_name;
    ifstream infile(dataset_name + ".txt");
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << dataset_name << ".txt\n";
        return 1;
    }
    
    infile >> n;
    int m;
    infile >> m;
    cout << "Processing graph with " << n << " vertices and " << m << " edges\n";
    
    // For very large graphs, cap the maximum memory usage
    if (n > 5000) {
        cout << "Warning: Large graph detected. Memory usage might be high.\n";
    }
    
    unordered_set<int, IntHash> uniqueVertices;
    vector<pair<int, int>> edges;
    edges.reserve(m);
    for (int i = 0; i < m; i++){
        int u, v; 
        infile >> u >> v;
        uniqueVertices.insert(u);
        uniqueVertices.insert(v);
        edges.push_back({u, v});
    }
    
    vertexMap.push_back(0);
    vector<int> sortedVertices(uniqueVertices.begin(), uniqueVertices.end());
    sort(sortedVertices.begin(), sortedVertices.end());
    int idx = 1;
    for (auto v : sortedVertices){
        vertexMap.push_back(v);
        reverseMap[idx] = v;
        idx++;
    }
    
    n = uniqueVertices.size();
    cout << "Adjusted to " << n << " unique vertices\n";
    
    adjacencyList.resize(n + 1);
    unordered_map<int,int> vertexToIdx;
    for (int i = 1; i <= n; i++) {
        vertexToIdx[vertexMap[i]] = i;
    }
    
    for (auto &edge : edges){
        int u = edge.first, v = edge.second;
        int uIdx = vertexToIdx[u], vIdx = vertexToIdx[v];
        adjacencyList[uIdx].push_back(vIdx);
        adjacencyList[vIdx].push_back(uIdx);
    }
    
    for (int i = 0; i <= n; i++){
        sort(adjacencyList[i].begin(), adjacencyList[i].end());
        adjacencyList[i].erase(unique(adjacencyList[i].begin(), adjacencyList[i].end()), adjacencyList[i].end());
    }
    
    cout << "Starting clique detection algorithm...\n";
    CLIQUE();
    cout << "Total cliques: " << ans << "\n";
    outfile<<"Dataset: "<<dataset_name<<endl;
    outfile << "Total cliques: " << ans << "\n";
    outfile <<"Current time = "<<chrono::system_clock::to_time_t(chrono::system_clock::now())<<endl;
    outfile<<"Execution time: "<<chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()<<"ms"<<endl;
    outfile.close();
    return 0;
}