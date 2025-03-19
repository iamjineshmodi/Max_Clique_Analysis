#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;

struct ReportStats
{
    int largestCliqueSize = 0;
    int totalMaximalCliques = 0;
    unordered_map<int, int> cliqueDistribution;
};

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
        for(int u : P)
        {
            size_t connections = 0;
            int uIdx = vertexToIndex.at(u);
            for(int v : P)
            {
                if(adjList[uIdx].count(v))
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
        for(int u : X)
        {
            size_t connections = 0;
            int uIdx = vertexToIndex.at(u);
            for(int v : P)
            {
                if(adjList[uIdx].count(v))
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
            for(int v : P)
            {
                if(!adjList[pivotIdx].count(v))
                {
                    vertices_to_process.push_back(v);
                }
            }
        }
        else
        {
            vertices_to_process.insert(vertices_to_process.end(), P.begin(), P.end());
        }
        unordered_set<int> newP, newX;
        for(int v : vertices_to_process)
        {
            int vIdx = vertexToIndex.at(v);
            R.insert(v);
            newP.clear();
            newX.clear();
            for(int w : P)
            {
                if(adjList[vIdx].count(w))
                {
                    newP.insert(w);
                }
            }
            for(int w : X)
            {
                if(adjList[vIdx].count(w))
                {
                    newX.insert(w);
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
        for(int w : adjList[vIdx])
        {
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
        // cout << "BronkerBosch " << i+1 << endl;
        BronKerbosch(R, Pv, Xv);
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
    ifstream infile("as-skitter.txt");
    int n, e;
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
    int index = 0;
    for(int v : vertices)
    {
        vertexToIndex[v] = index;
        indexToVertex.push_back(v);
        index++;
    }
    int n_vertices = vertices.size();
    cout << "Total vertices: " << n_vertices << endl;
    vector<unordered_set<int>> adjList(n_vertices);
    vector<int> degree(n_vertices, 0);
    for(int i=0; i < edges.size(); i++)
    {
        int u = edges[i].first, v = edges[i].second;
        int uIdx = vertexToIndex[u];
        int vIdx = vertexToIndex[v];
        adjList[uIdx].insert(v);
        adjList[vIdx].insert(u);
        degree[uIdx]++;
        degree[vIdx]++;
        // cout << "Adding edge " << i+1 << endl;
    }
    vector<int> sortedVertices(n_vertices);
    for(int i=0; i < n_vertices; i++)
    {
        sortedVertices[i] = i;
    }
    sort(sortedVertices.begin(), sortedVertices.end(), [&](int a, int b){
        return degree[a] < degree[b];
    });
    vector<int> order;
    order.reserve(n_vertices);
    for(int idx : sortedVertices)
    {
        order.push_back(indexToVertex[idx]);
    }
    
    ReportStats stats;
    auto start = high_resolution_clock::now();
    BronKerboschDegeneracy(vertices, adjList, vertexToIndex, order, stats);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    generateReport(stats, duration.count());
    return 0;
}
