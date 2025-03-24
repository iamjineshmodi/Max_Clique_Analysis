#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

constexpr size_t MAX_VERTICES = 15000000;

class Graph
{
private:
    int n; // number of vertices
    vector<vector<int>> adj;
    vector<vector<bool>> adjMatrix;
    vector<int> Q;
    vector<int> vertexMap;
    unordered_map<int, int> reverseMap;
    vector<unordered_set<int>> adjSet;
    int minCliqueSize;
    bool useAdjMatrix;
    FILE* outputFile;
    int cliqueCount;

    // Optimized pivot selection with bitset for large candidate sets
    int findPivot(const vector<int>& SUBG, const vector<int>& CAND, 
                  const vector<bool>& inSUBG, const vector<bool>& inCAND) {
        int pivot = -1;
        int maxCovered = -1;
        
        // Fast path for small candidate sets
        if (CAND.size() < 100) {
            for (int p : CAND) {
                int covered = 0;
                
                for (int v : CAND) {
                    if (v != p && isAdjacent(p, v)) {
                        covered++;
                    }
                }
                
                if (covered > maxCovered) {
                    maxCovered = covered;
                    pivot = p;
                }
            }
            
            // Try vertices in SUBG that are not in CAND
            if (maxCovered < static_cast<int>(CAND.size()) - 1) {
                for (int p : SUBG) {
                    if (inCAND[p]) continue;
                    
                    int covered = 0;
                    for (int v : CAND) {
                        if (isAdjacent(p, v)) {
                            covered++;
                        }
                    }
                    
                    if (covered > maxCovered) {
                        maxCovered = covered;
                        pivot = p;
                    }
                }
            }
        } 
        else {
            // Use sampling for large candidate sets
            const int SAMPLE_SIZE = min(30, static_cast<int>(CAND.size()));
            
            vector<int> samples;
            samples.reserve(SAMPLE_SIZE);
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, CAND.size() - 1);

            for (int i = 0; i < SAMPLE_SIZE; i++) {
                int idx = dis(gen);
                samples.push_back(CAND[idx]);
            }
            
            for (int p : samples) {
                int covered = 0;
                
                for (int v : CAND) {
                    if (v != p && isAdjacent(p, v)) {
                        covered++;
                    }
                }
                
                if (covered > maxCovered) {
                    maxCovered = covered;
                    pivot = p;
                }
            }
        }
        
        return pivot != -1 ? pivot : (CAND.empty() ? -1 : CAND[0]);
    }

    void expand(vector<int>& SUBG, vector<int>& CAND, vector<int>& DONE,
                vector<bool>& inSUBG, vector<bool>& inCAND, vector<bool>& inDONE) {
        // Size-based pruning - early termination if we can't reach minCliqueSize
        if (Q.size() + CAND.size() < minCliqueSize) {
            return; // Early termination
        }
        
        // If both SUBG and DONE are empty, we've found a maximal clique
        if (SUBG.empty() && DONE.empty()) {
            if (!Q.empty() && Q.size() >= minCliqueSize) {
                // Stream directly to file instead of storing in memory
                fprintf(outputFile, "[");
                for (size_t i = 0; i < Q.size(); i++) {
                    fprintf(outputFile, "%d", vertexMap[Q[i]]);
                    if (i != Q.size() - 1)
                        fprintf(outputFile, ", ");
                }
                fprintf(outputFile, "]\n");
                
                cliqueCount++;
                if(cliqueCount % 100000 == 0) {
                    cout << "Maximal cliques found: " << cliqueCount << endl;
                }
                
                cliqueSizeDistribution[Q.size()]++;
            }
            return;
        }
        
        if (SUBG.empty()) {
            return;
        }
        
        int pivot = findPivot(SUBG, CAND, inSUBG, inCAND);
        
        // ext = CAND - Neighbors(pivot)
        vector<int> ext;
        ext.reserve(CAND.size());
        
        for (int q : CAND) {
            if (!isAdjacent(pivot, q)) {
                ext.push_back(q);
            }
        }
        
        for (int q : ext) {
            Q.push_back(q);
            vector<int> newSUBG, newCAND, newDONE;
            vector<bool> newInSUBG(n, false);
            vector<bool> newInCAND(n, false);
            vector<bool> newInDONE(n, false);
            
            int estimatedSize = min((int)adj[q].size(), (int)SUBG.size());
            newSUBG.reserve(estimatedSize);
            newCAND.reserve(estimatedSize);
            newDONE.reserve(min((int)adj[q].size(), (int)DONE.size()));
            
            if (SUBG.size() > 1000 && adj[q].size() > 1000) {
                // Ensure adj[q] is sorted for set_intersection
                vector<int> sortedAdj = adj[q];
                sort(sortedAdj.begin(), sortedAdj.end());
                
                // SUBG ∩ N(q)
                set_intersection(SUBG.begin(), SUBG.end(), 
                                sortedAdj.begin(), sortedAdj.end(),
                                back_inserter(newSUBG));
                
                for (int v : newSUBG) newInSUBG[v] = true;
                
                // CAND ∩ N(q)
                set_intersection(CAND.begin(), CAND.end(), 
                                sortedAdj.begin(), sortedAdj.end(),
                                back_inserter(newCAND));
                
                for (int v : newCAND) newInCAND[v] = true;
                
                // DONE ∩ N(q)
                set_intersection(DONE.begin(), DONE.end(), 
                                sortedAdj.begin(), sortedAdj.end(),
                                back_inserter(newDONE));
                
                for (int v : newDONE) newInDONE[v] = true;
            } else {
                // For newSUBG: SUBG ∩ N(q)
                for (int v : adj[q]) {
                    if (inSUBG[v]) {
                        newSUBG.push_back(v);
                        newInSUBG[v] = true;
                    }
                }
                
                // For newCAND: CAND ∩ N(q)
                for (int v : adj[q]) {
                    if (inCAND[v]) {
                        newCAND.push_back(v);
                        newInCAND[v] = true;
                    }
                }
                
                // For newDONE: DONE ∩ N(q)
                for (int v : adj[q]) {
                    if (inDONE[v]) {
                        newDONE.push_back(v);
                        newInDONE[v] = true;
                    }
                }
            }
            
            expand(newSUBG, newCAND, newDONE, newInSUBG, newInCAND, newInDONE);
            
            // Remove q from CAND and add to DONE
            inCAND[q] = false;
            CAND.erase(remove(CAND.begin(), CAND.end(), q), CAND.end());
            
            DONE.push_back(q);
            inDONE[q] = true;
            
            Q.pop_back();
        }
    }
    
    // Fast adjacency check - use adjacency matrix, unordered_set, or binary search based on graph size
    inline bool isAdjacent(int u, int v) const {
        if (u == v) return false;
        
        if (useAdjMatrix) {
            return adjMatrix[u][v];
        } else if (n > 100000) {
            // For large graphs, use unordered_set for O(1) lookup
            return adjSet[u].find(v) != adjSet[u].end();
        } else {
            // For medium-sized graphs, use binary search
            return binary_search(adj[u].begin(), adj[u].end(), v);
        }
    }
    
    
    // Compute degeneracy ordering (k-core decomposition) with optimized bucket-based approach
    vector<int> computeDegeneracyOrder() {
        vector<int> order;
        order.reserve(n);
        
        // Create degree map - count only vertices with non-empty adjacency lists
        vector<int> degree(n);
        int maxDegree = 0;
        for (int i = 0; i < n; i++) {
            degree[i] = adj[i].size();
            maxDegree = max(maxDegree, degree[i]);
        }
        
        // Bucket sort approach for O(n+m) time complexity
        vector<vector<int>> buckets(maxDegree + 1);
        for (int i = 0; i < n; i++) {
            buckets[degree[i]].push_back(i);
        }
        
        // Track which vertices have been processed
        vector<bool> processed(n, false);
        int processedCount = 0;
        
        // Process vertices in ascending degree order
        for (int d = 0; d <= maxDegree && processedCount < n; d++) {
            while (!buckets[d].empty()) {
                int v = buckets[d].back();
                buckets[d].pop_back();
                
                if (processed[v] || degree[v] != d) continue;
                
                // Add to ordering
                order.push_back(v);
                processed[v] = true;
                processedCount++;
                
                for (int u : adj[v]) {
                    if (!processed[u] && degree[u] > d) {
                        buckets[degree[u]].erase(
                            remove(buckets[degree[u]].begin(), buckets[degree[u]].end(), u),
                            buckets[degree[u]].end()
                        );
                        degree[u]--;
                        buckets[degree[u]].push_back(u);
                    }
                }
            }
        }
        
        return order;
    }

    void prepareAdjacencyStructures() {
        if (n <= 100000) {
            useAdjMatrix = true;
            adjMatrix.assign(n, vector<bool>(n, false));
            
            for (int i = 0; i < n; i++) {
                for (int j : adj[i]) {
                    adjMatrix[i][j] = true;
                }
            }
            // cout << "Built adjacency matrix for fast lookups." << endl;
        } 
        // For large graphs: use unordered_set for O(1) lookup
        else {
            useAdjMatrix = false;
            adjSet.resize(n);
            
            for (int i = 0; i < n; i++) {
                adjSet[i].reserve(adj[i].size());
                for (int j : adj[i]) {
                    adjSet[i].insert(j);
                }
            }
            
            // Also maintain sorted vectors for set_intersection operations
            for (int i = 0; i < n; i++) {
                sort(adj[i].begin(), adj[i].end());
            }
            
            cout << "Using hash-based adjacency sets for large graph." << endl;
        }
    }

public:
    map<int, int> cliqueSizeDistribution; // Distribution of clique sizes

    Graph(int minSize = 1) : n(0), minCliqueSize(minSize), useAdjMatrix(false), outputFile(nullptr), cliqueCount(0)
    {
        adj.reserve(MAX_VERTICES);
        vertexMap.reserve(MAX_VERTICES);
        Q.reserve(1000); 
        
        srand(time(nullptr));
    }

    ~Graph() {
        if (outputFile) {
            fclose(outputFile);
        }
    }

    void addVertex(int v)
    {
        if (reverseMap.find(v) == reverseMap.end())
        {
            int idx = n++;

            if (adj.size() <= idx)
            {
                adj.resize(idx + 1);
            }
            if (vertexMap.size() <= idx)
            {
                vertexMap.resize(idx + 1);
            }

            vertexMap[idx] = v;
            reverseMap[v] = idx;
        }
    }

    void addEdge(int v, int w)
    {
        addVertex(v);
        addVertex(w);

        int v_idx = reverseMap[v];
        int w_idx = reverseMap[w];

        adj[v_idx].push_back(w_idx);
        adj[w_idx].push_back(v_idx);
    }

    void setMinCliqueSize(int size) {
        minCliqueSize = max(1, size);
    }

    void findMaximalCliques()
    {
        if (n == 0)
        {
            cout << "Graph is empty." << endl;
            return;
        }

        outputFile = fopen("output.txt", "wb");
        if (!outputFile) {
            cerr << "Error: Unable to open output file." << endl;
            return;
        }
        
        char* buffer = new char[102400]; // 100KB buffer
        setvbuf(outputFile, buffer, _IOFBF, sizeof(char) * 102400);

        for (int i = 0; i < n; i++) {
            sort(adj[i].begin(), adj[i].end());
            adj[i].erase(unique(adj[i].begin(), adj[i].end()), adj[i].end());
        }

        prepareAdjacencyStructures();

        // cout << "Computing degeneracy ordering..." << endl;
        vector<int> degOrder = computeDegeneracyOrder();
        // cout << "Degeneracy ordering complete." << endl;

        // Initialize sets for maximal clique algorithm
        vector<int> SUBG, CAND, DONE;
        SUBG.reserve(n);
        CAND.reserve(n);
        DONE.reserve(n);
        
        // Initialize boolean vectors for set membership
        vector<bool> inSUBG(n, false);
        vector<bool> inCAND(n, false);
        vector<bool> inDONE(n, false);
        
        // Use degeneracy ordering to populate CAND
        for (int v : degOrder) {
            // Skip vertices with no neighbors
            if (!adj[v].empty()) {
                CAND.push_back(v);
                inCAND[v] = true;
            }
        }
        
        // Copy CAND to SUBG
        SUBG = CAND;
        for (int v : SUBG) {
            inSUBG[v] = true;
        }

        // Clear clique tracking
        Q.clear();
        Q.reserve(min(1000, n)); // More reasonable size estimate
        
        // Reset clique counter
        cliqueCount = 0;
        cliqueSizeDistribution.clear();

        try
        {
            cout << "Finding maximal cliques..." << endl;
            expand(SUBG, CAND, DONE, inSUBG, inCAND, inDONE);

            // Error checking
            if (cliqueCount == 0 && n > 0)
            {
                cout << "Warning: No maximal cliques found. This may indicate an issue." << endl;
            }
        }
        catch (const std::exception& e)
        {
            cerr << "Exception caught: " << e.what() << endl;
            cerr << "This may indicate a problem with the dataset or memory limitations." << endl;
        }

        // Close output file
        if (outputFile) {
            fclose(outputFile);
            outputFile = nullptr;
        }
        
        // Clean up buffer
        delete[] buffer;

        // Print results
        printResults();
    }

    void printResults() {
        // Print results to console
        cout << "========== Dataset Report ==========" << endl;
        cout << "Total number of vertices: " << n << endl;
        cout << "Total number of maximal cliques: " << cliqueCount << endl;

        // Calculate largest clique size
        int largestCliqueSize = 0;
        if (!cliqueSizeDistribution.empty()) {
            largestCliqueSize = cliqueSizeDistribution.rbegin()->first;
        }
        cout << "Largest clique size: " << largestCliqueSize << endl;

        cout << "\nDistribution of clique sizes:" << endl;
        for (const auto& pair : cliqueSizeDistribution) {
            cout << "Size " << pair.first << ": " << pair.second << " cliques" << endl;
        }
        
        // Write summary report to file
        FILE* reportFile = fopen("clique_report.txt", "ab");
        if (reportFile) {
            char rptBuffer[65536]; // 64KB buffer
            setvbuf(reportFile, rptBuffer, _IOFBF, sizeof(rptBuffer));
            
            fprintf(reportFile, "Total vertices: %d\n", n);
            fprintf(reportFile, "Minimum clique size: %d\n", minCliqueSize);
            fprintf(reportFile, "Largest clique size: %d\n", largestCliqueSize);
            fprintf(reportFile, "Total maximal cliques: %d\n", cliqueCount);

            fprintf(reportFile, "Distribution of clique sizes:\n");
            for (const auto& pair : cliqueSizeDistribution) {
                fprintf(reportFile, "Size %d: %d cliques\n", pair.first, pair.second);
            }
            fprintf(reportFile, "----------------------------------------\n");
            fclose(reportFile);
        }
    }
};

int main()
{
    try
    {
        string datasetName;
        int minCliqueSize = 1;
        
        cout << "Enter dataset name: ";
        cin >> datasetName;
        
        minCliqueSize = 2; // Default minimum clique size

        string inputFile = datasetName + ".txt";
        
        // Use C-style file I/O for maximum performance
        FILE* infile = fopen(inputFile.c_str(), "rb");
        if (!infile) {
            cerr << "Error opening " << inputFile << endl;
            return 1;
        }

        // Use larger buffer for file I/O
        char buffer[1048576]; // 1MB buffer
        setvbuf(infile, buffer, _IOFBF, sizeof(buffer));

        int n = 0, e = 0;
        fscanf(infile, "%d %d", &n, &e);

        if (n <= 0 || e < 0 || e > static_cast<long long>(n) * (n - 1) / 2) {
            cout << "Invalid input parameters: n=" << n << ", e=" << e << endl;
            fclose(infile);
            return 1;
        }

        Graph g(minCliqueSize);

        // Read edges in larger batches for better I/O performance
        const int BATCH_SIZE = 5000000;
        int u, v, edgeCount = 0;
        int invalidEdges = 0;
        
        cout << "Reading edges..." << endl;
        auto startRead = high_resolution_clock::now();
        
        // Preallocate edges vector to avoid reallocations
        vector<pair<int, int>> edgeBatch;
        edgeBatch.reserve(BATCH_SIZE);
        
        while (edgeCount < e) {
            edgeBatch.clear();
            
            // Read a batch of edges
            int batchSize = 0;
            while (batchSize < BATCH_SIZE && edgeCount + batchSize < e) {
                if (fscanf(infile, "%d %d", &u, &v) != 2) break;
                
                if (u == v) {
                    invalidEdges++;
                    continue;
                }
                edgeBatch.push_back({u, v});
                batchSize++;
            }
            
            // Process the batch
            for (const auto& edge : edgeBatch) {
                g.addEdge(edge.first, edge.second);
            }
            
            edgeCount += batchSize;
            if (edgeCount > 0 && edgeCount % 10000000 == 0) {
                cout << "Processed " << edgeCount << "/" << e << " edges..." << endl;
            }
            
            // Break if we couldn't read more edges
            if (batchSize == 0) break;
        }
        
        fclose(infile);
        
        auto endRead = high_resolution_clock::now();
        auto readDuration = duration_cast<milliseconds>(endRead - startRead);
        cout << "Reading completed in " << readDuration.count() << "ms" << endl;

        cout << "\nAnalyzing dataset: " << datasetName << endl;
        cout << "Expected vertices: " << n << ", Expected edges: " << e << endl;
        cout << "Actual edges read: " << edgeCount << endl;
        if (invalidEdges > 0) {
            cout << "Invalid edges ignored: " << invalidEdges << endl;
        }

        auto start = high_resolution_clock::now();
        g.findMaximalCliques();
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        cout << "\nExecution time: " << duration.count() << "ms" << endl;

        // Write report with C-style I/O for better performance
        FILE* reportFile = fopen("clique_report.txt", "ab");
        if (reportFile) {
            char rptBuffer[65536]; // 64KB buffer
            setvbuf(reportFile, rptBuffer, _IOFBF, sizeof(rptBuffer));
            
            fprintf(reportFile, "Dataset: %s\n", datasetName.c_str());
            fprintf(reportFile, "Vertices: %d, Edges: %d\n", n, edgeCount);
            fprintf(reportFile, "Minimum clique size: %d\n", minCliqueSize);
            fprintf(reportFile, "Execution time: %ldms\n", duration.count());

            int largestCliqueSize = g.cliqueSizeDistribution.empty() ? 0 : g.cliqueSizeDistribution.rbegin()->first;
            fprintf(reportFile, "Largest clique size: %d\n", largestCliqueSize);

            int totalCliques = 0;
            for (const auto& pair : g.cliqueSizeDistribution) {
                totalCliques += pair.second;
            }
            fprintf(reportFile, "Total maximal cliques: %d\n", totalCliques);

            fprintf(reportFile, "Distribution of clique sizes:\n");
            for (const auto& pair : g.cliqueSizeDistribution) {
                fprintf(reportFile, "Size %d: %d cliques\n", pair.first, pair.second);
            }
            fprintf(reportFile, "----------------------------------------\n");
            fclose(reportFile);
        }
    }
    catch (const std::exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
