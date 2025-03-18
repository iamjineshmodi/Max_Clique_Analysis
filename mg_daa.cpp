#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

// Global variables
vector<vector<int>> adjList;
vector<int> indexToVertex;

struct ReportStats
{
    int maximalCliquesFound = 0;
    int maxCliqueSize = 0;
    int totalNodesExplored = 0;
    int pruningEvents = 0;
};

template <typename T>
void Print(const unordered_set<T> &s, ofstream &outfile)
{
    if (!outfile.is_open())
        return;
    outfile << "{";
    bool first = true;
    for (const auto &element : s)
    {
        if (!first)
        {
            outfile << ", ";
        }
        outfile << indexToVertex[element];
        first = false;
    }
    outfile << "}" << endl;
}

unordered_set<int> intersect(const unordered_set<int> &C, const vector<int> &Ni)
{
    unordered_set<int> result;
    if (C.empty() || Ni.empty())
        return result;
    for (int v : C)
    {
        if (find(Ni.begin(), Ni.end(), v) != Ni.end())
        {
            result.insert(v);
        }
    }
    return result;
}

bool isMaximal(const unordered_set<int> &C)
{
    if (C.empty())
        return false;

    for (int v = 0; v < (int)adjList.size(); v++)
    {
        if (C.find(v) == C.end())
        {
            bool canExpand = true;
            for (int u : C)
            {
                if (find(adjList[u].begin(), adjList[u].end(), v) == adjList[u].end())
                {
                    canExpand = false;
                    break;
                }
            }
            if (canExpand)
                return false;
        }
    }
    return true;
}

void printClique(const unordered_set<int> &C, ofstream &outfile, ReportStats &stats)
{
    if (!outfile.is_open())
        return;
    if (C.size() >= 2 && isMaximal(C))
    {
        outfile << "Maximal Clique found: ";
        Print(C, outfile);
        stats.maximalCliquesFound++;
        stats.maxCliqueSize = max(stats.maxCliqueSize, (int)C.size());
    }
}

void Update(int idx, unordered_set<int> &C, const vector<int> &degreeOrder,
            unordered_map<int, int> &T, unordered_map<int, int> &S,
            ReportStats &stats, ofstream &outfile)
{

    if (idx >= (int)degreeOrder.size())
        return;
    if (!outfile.is_open())
        return;

    stats.totalNodesExplored++;
    int i = degreeOrder[idx];

    // Step 1: Compute C - N(i)
    unordered_set<int> CminusNi;
    for (int v : C)
    {
        if (find(adjList[i].begin(), adjList[i].end(), v) == adjList[i].end())
        {
            CminusNi.insert(v);
        }
    }

    if (!CminusNi.empty())
    {
        Update(idx + 1, C, degreeOrder, T, S, stats, outfile);
        stats.pruningEvents++;
    }

    // Step 2: Compute T[y] = |N(y) ∩ C ∩ N(i)|
    unordered_set<int> CintersectNi = intersect(C, adjList[i]);
    for (int x : CintersectNi)
    {
        for (int y : adjList[x])
        {
            if (C.find(y) == C.end() && y != i)
            {
                T[y]++;
            }
        }
    }

    // Step 3: Compute S[y] = |N(y) ∩ (C - N(i))|
    for (int x : CminusNi)
    {
        for (int y : adjList[x])
        {
            if (C.find(y) == C.end())
            {
                S[y]++;
            }
        }
    }

    bool FLAG = true;

    // Step 4: Maximality test
    for (const auto &y : adjList[i])
    {
        if (y < i && C.find(y) == C.end() && T[y] == (int)CintersectNi.size())
        {
            FLAG = false;
            break;
        }
    }

    // Step 5: Sort vertices in C - N(i) in ascending order
    vector<int> sortedCminusNi(CminusNi.begin(), CminusNi.end());
    sort(sortedCminusNi.begin(), sortedCminusNi.end());
    // p = |C - N(i)| is just sortedCminusNi.size()

    // Step 6: Lexicographic test
    vector<int> sortedC(C.begin(), C.end());
    sort(sortedC.begin(), sortedC.end()); // Sort C in ascending order

    for (size_t k = 0; k < sortedC.size(); k++)
    {                        // k:= 1 to p (1-indexed in pseudocode, 0-indexed here)
        int jk = sortedC[k]; // jk is the k-th vertex in sorted C

        for (int y : adjList[jk])
        { // For each vertex y in N(jk)
            if (C.find(y) == C.end() && y < i && T[y] == CintersectNi.size())
            { // y ∈ N(jk) - C such that y < i and T[y] = |C ∩ N(i)|
                if (y >= jk)
                {           // if y ≥ jk
                    S[y]--; // Alter S[y] to S(y)
                }
                else
                { // else (y < jk)
                    // Check if jk is the first vertex which satisfies y < jk
                    for (size_t j = 0; j < k; j++)
                    {
                        if (y < sortedC[j])
                        {
                            if ( S[y] + k - 1 == sortedC.size() && y >= sortedC[k - 1])
                            {                 // if (S[y] + k - 1 = p) and (y ≥ jk-1)
                                FLAG = false; // C is not lexicographically largest
                            }
                            break;
                        }
                    }

                }
            }
        }
    }

    // Step 7
    if (!CintersectNi.empty()) // if intersection of C and N(i) is not empty
    {
        for (int y = 0; y < i; y++)
        {
            if (C.find(y) == C.end() && y != i && T[y] == CintersectNi.size() && S[y] == 0)
            {
                // For unordered_set, we need to find the max element manually
                if (!CminusNi.empty() && *std::max_element(CminusNi.begin(), CminusNi.end()) < y)
                {
                    FLAG = false;
                    break;
                }
            }
        }
    }
    else // if intersection is empty
    {
        // For unordered_set, we need to find the max element manually
        if (!CminusNi.empty() && *std::max_element(CminusNi.begin(), CminusNi.end()) < (i - 1))
        {
            FLAG = false;
        }
    }

    // Reset S and T
    S.clear();
    T.clear();

    if (FLAG)
    {
        unordered_set<int> newC = CintersectNi;
        newC.insert(i);
        printClique(newC, outfile, stats);
        Update(idx + 1, newC, degreeOrder, T, S, stats, outfile);
    }
}

void generateReport(const ReportStats &stats, ofstream &outfile, long long duration)
{
    outfile << "\n--- Execution Report ---" << endl;
    outfile << "Total maximal cliques found: " << stats.maximalCliquesFound << endl;
    outfile << "Size of largest clique: " << stats.maxCliqueSize << endl;
    outfile << "Total nodes explored: " << stats.totalNodesExplored << endl;
    outfile << "Pruning events: " << stats.pruningEvents << endl;
    outfile << "Execution time: " << duration << "ms" << endl;
}

int main()
{
    ifstream infile("input1.txt");
    ofstream outfile("output2.txt");
    int n, e;
    infile >> n >> e;
    unordered_map<int, int> vertexToIndex;
    unordered_map<int, int> degree;
    unordered_set<int> uniqueVertices;
    vector<pair<int, int>> edges;
    for (int i = 0; i < e; i++)
    {
        int u, v;
        infile >> u >> v;
        if (u == v)
        {
            cerr << "Invalid edge: Self-loop detected" << endl;
            return 1;
        }
        edges.push_back({u, v});
        uniqueVertices.insert(u);
        uniqueVertices.insert(v);
        degree[u]++;
        degree[v]++;
    }
    vector<int> sortedVertices(uniqueVertices.begin(), uniqueVertices.end());
    sort(sortedVertices.begin(), sortedVertices.end(), [&](int a, int b)
         { return degree[a] < degree[b]; });
    int index = 0;
    vector<int> degreeOrder;
    for (int v : sortedVertices)
    {
        vertexToIndex[v] = index;
        indexToVertex.push_back(v);
        degreeOrder.push_back(index);
        index++;
    }
    adjList.resize(uniqueVertices.size());
    for (auto &edge : edges)
    {
        int u = vertexToIndex[edge.first];
        int v = vertexToIndex[edge.second];
        adjList[u].push_back(v);
        adjList[v].push_back(u);
    }
    ReportStats stats;
    auto start = high_resolution_clock::now();
    unordered_set<int> C;
    unordered_map<int, int> T, S;
    Update(0, C, degreeOrder, T, S, stats, outfile);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    generateReport(stats, outfile, duration.count());
    outfile.close();
    return 0;
}
