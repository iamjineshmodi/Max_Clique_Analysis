#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 2000000; // Adjust as needed
int n;                           // Number of vertices
vector<vector<int>> adjacencyList;
vector<int> S, T; // Global arrays for S and T
bool FLAG;
int ans = 0;
vector<int> vertexMap;              // Maps original vertex IDs to consecutive indices
unordered_map<int, int> reverseMap; // Maps consecutive indices back to original IDs

// Function to reorder vertices by degree
void reorderVerticesByDegree()
{
    // Step 1: Create a vector of pairs (vertex, degree)
    vector<pair<int, int>> vertexDegrees;
    for (int i = 1; i <= n; i++)
    {
        vertexDegrees.push_back({i, adjacencyList[i].size()});
    }

    // Step 2: Sort by degree (non-decreasing order)
    sort(vertexDegrees.begin(), vertexDegrees.end(),
         [](const pair<int, int> &a, const pair<int, int> &b)
         {
             return a.second < b.second || (a.second == b.second && a.first < b.first);
         });

    // Step 3: Create new adjacency list with reordered vertices
    vector<vector<int>> newAdjList(n + 1);
    unordered_map<int, int> oldToNew; // Maps old indices to new indices
    unordered_map<int, int> newToOld; // Maps new indices to old indices

    for (int i = 0; i < n; i++)
    {
        int oldIndex = vertexDegrees[i].first;
        int newIndex = i + 1; // New indices start from 1
        oldToNew[oldIndex] = newIndex;
        newToOld[newIndex] = oldIndex;
    }

    // Step 4: Rebuild adjacency lists with new indices
    for (int i = 1; i <= n; i++)
    {
        for (int neighbor : adjacencyList[i])
        {
            if (oldToNew.find(neighbor) != oldToNew.end())
            {
                newAdjList[oldToNew[i]].push_back(oldToNew[neighbor]);
            }
        }
    }

    // Step 5: Update the adjacency list
    adjacencyList = newAdjList;

    // Step 6: Update the reverseMap to maintain the mapping back to original IDs
    unordered_map<int, int> newReverseMap;
    for (int i = 1; i <= n; i++)
    {
        newReverseMap[i] = reverseMap[newToOld[i]];
    }
    reverseMap = newReverseMap;

    // Step 7: Sort adjacency lists
    for (int i = 1; i <= n; i++)
    {
        sort(adjacencyList[i].begin(), adjacencyList[i].end());
    }
}

set<int> setDifference(const set<int> &A, const set<int> &B)
{
    set<int> diff;
    set_difference(A.begin(), A.end(), B.begin(), B.end(),
                   inserter(diff, diff.begin()));
    return diff;
}

set<int> setUnion(const set<int> &A, const set<int> &B)
{
    set<int> result = A;
    result.insert(B.begin(), B.end());
    return result;
}

set<int> setIntersection(const set<int> &A, const set<int> &B)
{
    set<int> intersect;
    set_intersection(A.begin(), A.end(), B.begin(), B.end(),
                     inserter(intersect, intersect.begin()));
    return intersect;
}

void printClique(const set<int> &C)
{
    cout << "Clique: ";
    ans++;
    for (int v : C)
    {
        cout << reverseMap[v] << " "; // Convert back to original vertex IDs
    }
    cout << endl;
}

void UPDATE(int i, set<int> &C)
{
    if (i == n + 1)
    {
        printClique(C);
        return;
    }

    // Make N(i) as a set
    set<int> Ni(adjacencyList[i].begin(), adjacencyList[i].end());
    // C - N(i)
    set<int> CminusNi = setDifference(C, Ni);
    // If CminusNi is not empty, call UPDATE(i+1, C)
    if (!CminusNi.empty())
    {
        UPDATE(i + 1, C);
    }

    // (C ∩ N(i))
    set<int> CintersectNi = setIntersection(C, Ni);
    set<int> Cminusi = setDifference(C, {i});

    // Compute T[y]
    for (int x : CintersectNi)
    {
        set<int> Nxi(adjacencyList[x].begin(), adjacencyList[x].end());
        for (int y : setDifference(Nxi, Cminusi))
        {
            T[y]++;
        }
    }

    // Compute S[y]
    for (int x : CminusNi)
    {
        set<int> Nxi(adjacencyList[x].begin(), adjacencyList[x].end());
        for (int y : setDifference(Nxi, C))
        {
            S[y]++;
        }
    }

    FLAG = true;
    // Maximality test
    for (int y : setDifference(Ni, C))
    {
        if (y < i && T[y] == (int)CintersectNi.size())
        {
            FLAG = false;
            break;
        }
    }

    // Sort C - N(i)
    vector<int> cMinusNiVec(CminusNi.begin(), CminusNi.end());
    cMinusNiVec.push_back(0);
    sort(cMinusNiVec.begin(), cMinusNiVec.end());

    // Lexico test
    int p = (int)cMinusNiVec.size() - 1;
    for (int k = 0; k <= p; k++)
    {
        int j_k = cMinusNiVec[k];
        set<int> Nxjk(adjacencyList[j_k].begin(), adjacencyList[j_k].end());
        for (int y : setDifference(Nxjk, C))
        {
            if (y < i && T[y] == (int)CintersectNi.size())
            {
                if (y >= j_k)
                {
                    S[y]--;
                }
                else
                {
                    int first = -1;
                    for (int m = 0; m <= p; m++)
                    {
                        if (y < cMinusNiVec[m])
                        {
                            first = m;
                            break;
                        }
                    }
                    if (first != -1 && k == first)
                    {
                        if ((S[y] + k - 1) == p)
                        {
                            if (y >= (k - 1 > 0 ? cMinusNiVec[k - 1] : 0))
                            {
                                FLAG = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!FLAG)
            break;
    }

    // Case S[y] = 0
    int j_p = (p > 0 ? cMinusNiVec[p] : 0);
    if (!CintersectNi.empty())
    {
        for (int y = 1; y <= n; y++)
        {
            if ((C.find(y) == C.end()) && y != i && y < i)
            {
                if (T[y] == (int)CintersectNi.size() && S[y] == 0)
                {
                    // int j_p = cMinusNiVec.empty() ? 0 : cMinusNiVec[p];
                    if (j_p < y)
                    {
                        FLAG = false;
                        break;
                    }
                }
            }
        }
    }
    else if (j_p < i - 1)
    {
        FLAG = false;
    }

    // Reinitialize S[y] and T[y]
    for (int x : CintersectNi)
    {
        set<int> Nxi(adjacencyList[x].begin(), adjacencyList[x].end());
        for (int y : setDifference(Nxi, Cminusi))
        {
            if (y >= 1 && y <= n)
                T[y] = 0;
        }
    }
    for (int x : CminusNi)
    {
        set<int> Nxi(adjacencyList[x].begin(), adjacencyList[x].end());
        for (int y : setDifference(Nxi, C))
        {
            if (y >= 1 && y <= n)
                S[y] = 0;
        }
    }

    if (FLAG)
    {
        set<int> SAVE = CminusNi;
        set<int> newC = setIntersection(C, Ni);
        newC.insert(i);
        set<int> oldC = C;
        C = newC;
        UPDATE(i + 1, C);
        C = setUnion(setDifference(C, {i}), SAVE);
    }
}

void CLIQUE()
{
    // First reorder vertices by degree
    reorderVerticesByDegree();

    // Now initialize S and T arrays
    S.assign(n + 1, 0);
    T.assign(n + 1, 0);

    // Start with C = {1} (vertex with smallest degree)
    set<int> C;
    if (n > 0)
        C.insert(1);

    // Call UPDATE starting from vertex 2
    UPDATE(2, C);
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string dataset_name;
    cin >> dataset_name;
    ifstream infile(dataset_name + ".txt");
    infile >> n;

    // Read edges
    int m;
    infile >> m;

    // First pass: collect all unique vertex IDs
    set<int> uniqueVertices;
    vector<pair<int, int>> edges;

    for (int i = 0; i < m; i++)
    {
        int u, v;
        infile >> u >> v;
        uniqueVertices.insert(u);
        uniqueVertices.insert(v);
        edges.push_back({u, v});
    }

    // Create mapping from original IDs to consecutive indices starting from 1
    vertexMap.push_back(0); // Dummy value at index 0
    int idx = 1;
    for (int v : uniqueVertices)
    {
        vertexMap.push_back(v);
        reverseMap[idx] = v;
        idx++;
    }

    // Adjust n to the number of unique vertices
    n = uniqueVertices.size();

    // Initialize adjacency list with the correct size
    adjacencyList.resize(n + 1);

    // Second pass: build adjacency list using the mapped indices
    for (auto &edge : edges)
    {
        int u = edge.first;
        int v = edge.second;

        // Find the indices of u and v in vertexMap
        int uIdx = find(vertexMap.begin(), vertexMap.end(), u) - vertexMap.begin();
        int vIdx = find(vertexMap.begin(), vertexMap.end(), v) - vertexMap.begin();

        adjacencyList[uIdx].push_back(vIdx);
        adjacencyList[vIdx].push_back(uIdx);
    }

    for (int i = 0; i <= n; i++)
    {
        sort(adjacencyList[i].begin(), adjacencyList[i].end());
    }

    CLIQUE();
    cout << "Total cliques: " << ans << endl;
    return 0;
}