#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

struct ReportStats
{
    int largestCliqueSize = 0;
    int totalMaximalCliques = 0;
    unordered_map<int, int> cliqueDistribution;
};

int n;
vector<unordered_set<int>> adj;

vector<int> computeDegeneracyOrder()
{
    vector<int> order;
    order.reserve(n);
    vector<int> degree(n);
    int maxDegree = 0;
    for(int i=0; i<n; i++)
    {
        degree[i] = adj[i].size();
        maxDegree = max(maxDegree, degree[i]);
    }
    vector<vector<int>> buckets(maxDegree + 1);
    for(int i=0; i<n; i++)
    {
        buckets[degree[i]].push_back(i);
    }
    vector<bool> processed(n, false);
    int processedCount = 0;
    for(int d=0; d <= maxDegree && processedCount < n; d++)
    {
        while(!buckets[d].empty())
        {
            int v = buckets[d].back();
            buckets[d].pop_back();
            if(processed[v] || degree[v] != d)
            {
                continue;
            }
            order.push_back(v);
            processed[v] = true;
            processedCount++;
            for(int u : adj[v])
            {
                if(!processed[u] && degree[u] > d)
                {
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

void BronKerboschDegeneracy(const unordered_set<int>& vertices, const vector<unordered_set<int>>& adjList, const unordered_map<int, int>& vertexToIndex, const vector<int>& order, ReportStats& stats)
{
    if(vertices.empty())
    {
        return;
    }
    unordered_set<int> P(vertices.begin(), vertices.end());
    unordered_set<int> X;
    unordered_set<int> R;
    unordered_set<int> Pv, Xv;

    function<void(unordered_set<int>&, unordered_set<int>&, unordered_set<int>&)> BronKerbosch = [&](unordered_set<int>& R, unordered_set<int>& P, unordered_set<int>& X)
    {
        if(P.empty() && X.empty())
        {
            stats.totalMaximalCliques++;
            int cliqueSize = R.size();
            stats.largestCliqueSize = max(stats.largestCliqueSize, cliqueSize);
            stats.cliqueDistribution[cliqueSize]++;
            return;
        }
        int pivot = -1;
        size_t maxConnections = 0;
        for(auto it = P.begin(); it != P.end(); ++it)
        {
            int u = *it;
            size_t connections = 0;
            int uIdx = vertexToIndex.at(u);
            for(auto innerIt = P.begin(); innerIt != P.end(); ++innerIt)
            {
                if(adjList[uIdx].count(*innerIt))
                {
                    connections++;
                }
            }
            if(connections > maxConnections)
            {
                maxConnections = connections;
                pivot = u;
            }
        }
        for(auto it = X.begin(); it != X.end(); ++it)
        {
            int u = *it;
            size_t connections = 0;
            int uIdx = vertexToIndex.at(u);
            for(auto innerIt = P.begin(); innerIt != P.end(); ++innerIt)
            {
                if(adjList[uIdx].count(*innerIt))
                {
                    connections++;
                }
            }  
            if(connections > maxConnections)
            {
                maxConnections = connections;
                pivot = u;
            }
        }
        vector<int> vertices_to_process;
        vertices_to_process.reserve(P.size());
        if(pivot != -1)
        {
            int pivotIdx = vertexToIndex.at(pivot);
            for(auto it = P.begin(); it != P.end(); ++it)
            {
                if(!adjList[pivotIdx].count(*it))
                {
                    vertices_to_process.push_back(*it);
                }
            }
        }
        else
        {
            for(auto it = P.begin(); it != P.end(); ++it)
            {
                vertices_to_process.push_back(*it);
            }
        }
        unordered_set<int> newP, newX;
        for(int v : vertices_to_process)
        {
            int vIdx = vertexToIndex.at(v);
            R.insert(v);
            newP.clear();
            newX.clear();
            for(auto it = P.begin(); it != P.end(); ++it)
            {
                if(adjList[vIdx].count(*it))
                {
                    newP.insert(*it);
                }
            }
            for(auto it = X.begin(); it != X.end(); ++it)
            {
                if(adjList[vIdx].count(*it))
                {
                    newX.insert(*it);
                }
            }
            BronKerbosch(R, newP, newX);
            R.erase(v);
            P.erase(v);
            X.insert(v);
        }
    };

    int i=0;
    for(int v : order)
    {
        int vIdx = vertexToIndex.at(v);
        Pv.clear();
        Xv.clear();
        for(auto it = adjList[vIdx].begin(); it != adjList[vIdx].end(); ++it)
        {
            int w = *it;
            if(P.count(w))
            {
                Pv.insert(w);
            }
            if(X.count(w))
            {
                Xv.insert(w);
            }
        }
        R = {v};
        BronKerbosch(R, Pv, Xv);
        // cout << "BronkerBosch " << i+1 << endl;
        P.erase(v);
        X.insert(v);
        i++;
    }
}

void generateReport(const ReportStats& stats, long long execTime)
{
    cout << "=== MAXIMAL CLIQUE ANALYSIS REPORT ===" << endl;
    cout << "1. Largest size of clique: " << stats.largestCliqueSize << endl;
    cout << "2. Total number of maximal cliques: " << stats.totalMaximalCliques << endl;
    cout << "3. Execution time: " << execTime << " ms" << endl;
    cout << "4. Distribution of different size cliques:" << endl;
    vector<pair<int, int>> sortedDist(stats.cliqueDistribution.begin(), stats.cliqueDistribution.end());
    sort(sortedDist.begin(), sortedDist.end());
    for(const auto& element : sortedDist)
    {
        int size = element.first, count = element.second;
        cout << "   Size " << size << ": " << count << " cliques" << endl;
    }
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    string filename;
    cin >> filename;
    filename += ".txt";
    ifstream infile(filename);
    cout << "Reading input file " << filename << endl;
    int e;
    infile >> n >> e;
    unordered_set<int> vertices;
    unordered_map<int, int> vertexToIndex;
    vector<int> indexToVertex;
    vector<pair<int, int>> edges;
    edges.reserve(e);
    for(int i=0; i<e; i++)
    {
        int u, v;
        infile >> u >> v;
        if(u == v)
        {
            cerr << "Invalid edge: Self-loop detected" << endl;
            return 1;
        }
        // cout << "Reading edge " << i+1 << endl;
        edges.push_back({u, v});
        vertices.insert(u);
        vertices.insert(v);
    }
    cout << "Read edges" << endl;
    int index = 0;
    for(int v : vertices)
    {
        vertexToIndex[v] = index;
        indexToVertex.push_back(v);
        // cout << "Mapping vertex " << index+1 << endl;
        index++;
    }
    cout << "Created vertex index mapping" << endl;
    // Initialize adj list for computeDegeneracyOrder
    adj.resize(n);
    vector<int> degree(n, 0);
    vector<unordered_set<int>> adjList(n);
    for(int i=0; i<e; i++)
    {
        int u = edges[i].first, v = edges[i].second;
        int uIdx = vertexToIndex[u];
        int vIdx = vertexToIndex[v];
        adjList[uIdx].insert(v);
        adjList[vIdx].insert(u);
        
        // Also fill the global adj list for computeDegeneracyOrder
        adj[uIdx].insert(vIdx);
        adj[vIdx].insert(uIdx);
        
        degree[uIdx]++;
        degree[vIdx]++;
        // cout << "Adding edge " << i+1 << endl;
    }
    cout << "Created adjacency list" << endl;
    vector<int> order = computeDegeneracyOrder();
    vector<int> orderWithOriginalLabels;
    for(int idx : order)
    {
        orderWithOriginalLabels.push_back(indexToVertex[idx]);
    }
    cout << "Created degeneracy ordering" << endl;
    
    ReportStats stats;
    auto start = high_resolution_clock::now();
    BronKerboschDegeneracy(vertices, adjList, vertexToIndex, orderWithOriginalLabels, stats);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    generateReport(stats, duration.count());
    return 0;
}
