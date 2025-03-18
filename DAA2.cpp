#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;
using namespace std::chrono;

struct ReportStats
{
    int largestCliqueSize = 0;
    int totalMaximalCliques = 0;
    unordered_map<int, int> cliqueDistribution;
};

void PrintSet(ofstream& outfile, const unordered_set<int>& s)
{
    outfile << "[";
    bool first = true;
    for(const auto& element : s)
    {
        if(!first)
        {
            outfile << ", ";
        }
        outfile << element;
        first = false;
    }
    outfile << "]";
}

void BronKerbosch(unordered_set<int>& R, unordered_set<int>& P, unordered_set<int>& X, const vector<vector<int>>& adjList, const unordered_map<int, int>& vertexToIndex, ReportStats& stats, ofstream& outfile)
{
    if(P.empty() && X.empty())
    {
        stats.totalMaximalCliques++;
        int cliqueSize = R.size();
        stats.largestCliqueSize = max(stats.largestCliqueSize, cliqueSize);
        stats.cliqueDistribution[cliqueSize]++;
        PrintSet(outfile, R);
        outfile << endl;
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
            int vIdx = vertexToIndex.at(v);
            if(binary_search(adjList[uIdx].begin(), adjList[uIdx].end(), v))
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
    unordered_set<int> Pcopy;
    if(pivot != -1)
    {
        int pivotIdx = vertexToIndex.at(pivot);
        for(int v : P)
        {
            if(!binary_search(adjList[pivotIdx].begin(), adjList[pivotIdx].end(), v))
            {
                Pcopy.insert(v);
            }
        }
    }
    else
    {
        Pcopy = P;
    }
    for(int v : Pcopy)
    {
        int vIdx = vertexToIndex.at(v);
        unordered_set<int> newR = R;
        newR.insert(v);
        unordered_set<int> newP, newX;
        for(int w : P)
        {
            if(binary_search(adjList[vIdx].begin(), adjList[vIdx].end(), w))
            {
                newP.insert(w);
            }
        }
        for(int w : X)
        {
            if(binary_search(adjList[vIdx].begin(), adjList[vIdx].end(), w))
            {
                newX.insert(w);
            }
        }
        BronKerbosch(newR, newP, newX, adjList, vertexToIndex, stats, outfile);
        P.erase(v);
        X.insert(v);
    }
}

void BronKerboschDegeneracy(const unordered_set<int>& vertices, const vector<vector<int>>& adjList, const unordered_map<int, int>& vertexToIndex, const vector<int>& indexToVertex, ReportStats& stats, ofstream& outfile)
{
    if(vertices.empty())
    {
        return;
    }
    int n = vertices.size();
    vector<int> degree(n);
    vector<bool> visited(n, false);
    for(int i=0; i<n; i++)
    {
        int v = indexToVertex[i];
        degree[i] = adjList[vertexToIndex.at(v)].size();
    }
    vector<int> order;
    order.reserve(n);
    for(int i=0; i<n; i++)
    {
        int minDegreeNode = -1;
        for(int j=0; j<n; j++)
        {
            if(!visited[j] && (minDegreeNode == -1 || degree[j] < degree[minDegreeNode]))
            {
                minDegreeNode = j;
            }
        }
        order.push_back(indexToVertex[minDegreeNode]);
        // cout << "Order " << i << endl;
        visited[minDegreeNode] = true;
        for(int neighbor : adjList[minDegreeNode])
        {
            int neighborIdx = vertexToIndex.find(neighbor) != vertexToIndex.end() ? vertexToIndex.at(neighbor) : -1;
            if(neighborIdx != -1 && !visited[neighborIdx])
            {
                degree[neighborIdx]--;
            }
        }
    }
    unordered_set<int> P(vertices.begin(), vertices.end());
    unordered_set<int> X;

    int i3 = 1;
    for(int v : order)
    {
        int vIdx = vertexToIndex.at(v);
        unordered_set<int> Pv, Xv;
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
        // cout << "Bron-kerBosch for node " << i3++ << endl;
        unordered_set<int> R = {v};
        BronKerbosch(R, Pv, Xv, adjList, vertexToIndex, stats, outfile);
        P.erase(v);
        X.insert(v);
    }
}

void generateReport(const ReportStats& stats, ofstream& outfile, long long execTime)
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
    outfile << "\n=== MAXIMAL CLIQUE ANALYSIS REPORT ===" << endl;
    outfile << "1. Largest size of clique: " << stats.largestCliqueSize << endl;
    outfile << "2. Total number of maximal cliques: " << stats.totalMaximalCliques << endl;
    outfile << "3. Execution time: " << execTime << " ms" << endl;
    outfile << "4. Distribution of different size cliques:" << endl;
    for(const auto& element : sortedDist)
    {
        int size = element.first, count = element.second;
        outfile << "   Size " << size << ": " << count << " cliques" << endl;
    }
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    ifstream infile("as-skitter.txt");
    ofstream outfile("output_as-skitter.txt");
    int n, e;
    infile >> n >> e;
    unordered_set<int> vertices;
    unordered_map<int, int> vertexToIndex;
    vector<int> indexToVertex;
    vector<vector<int>> adjList;
    vector<pair<int, int>> edges;
    edges.reserve(e);
    for(int i=0; i<e; i++)
    {
        int u, v;
        infile >> u >> v;
        // cout << "Reading edge " << i << endl;
        if(u == v)
        {
            cerr << "Invalid edge: Self-loop detected" << endl;
            return 1;
        }
        edges.push_back({u, v});
        vertices.insert(u);
        vertices.insert(v);
    }
    int index = 0;
    for(int v : vertices)
    {
        vertexToIndex[v] = index;
        // cout << "Pushing vertex " << index+1 << endl;
        indexToVertex.push_back(v);
        index++;
    }
    int i1=1;
    adjList.resize(vertices.size());
    for(const auto& edge : edges)
    {
        int u = edge.first, v = edge.second;
        int uIdx = vertexToIndex[u];
        int vIdx = vertexToIndex[v];
        // cout << "Adding edge to adjlist " << i1++ << endl;
        adjList[uIdx].push_back(v);
        adjList[vIdx].push_back(u);
    }
    int i2=1;
    for(auto& neighbors : adjList)
    {
        // cout << "Sorting adjlist " << i2++ << endl;
        sort(neighbors.begin(), neighbors.end());
    }

    ReportStats stats;
    auto start = high_resolution_clock::now();
    BronKerboschDegeneracy(vertices, adjList, vertexToIndex, indexToVertex, stats, outfile);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    generateReport(stats, outfile, duration.count());
    outfile.close();
    return 0;
}
