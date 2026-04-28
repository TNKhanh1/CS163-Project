#include "MST.h"
#include <algorithm>
#include <queue>
#include <random>
#include <set>
#include <algorithm>
#include <utility>

struct DSU
{
    std::unordered_map<int, int> parent, rank;

    void make_set(int v)
    {
        parent[v] = v;
        rank[v] = 0;
    }

    int find_set(int v)
    {
        if (!parent.count(v))
        {
            make_set(v);
        }
        return (v == parent[v]) ? v : parent[v] = find_set(parent[v]);
    }

    bool union_sets(int a, int b)
    {
        a = find_set(a);
        b = find_set(b);

        if (a == b)
        {
            return false;
        }

        if (rank[a] < rank[b])
        {
            std::swap(a, b);
        }

        parent[b] = a;

        if (rank[a] == rank[b])
        {
            rank[a]++;
        }

        return true;
    }
};

MST::MST()
{
}

MST::~MST()
{
    clear();
}

void MST::clear()
{
    edges.clear();
    vertices.clear();
}

const std::vector<Edge>& MST::getEdges() const
{
    return edges;
}

const std::set<int>& MST::getVertices() const
{
    return vertices;
}

void MST::insertEdge(int u, int v, int weight)
{
    edges.push_back({u, v, weight});
    vertices.insert(u);
    vertices.insert(v);
}

void MST::generateRandomGraph(int numVertices, int numEdges)
{
    clear();

    if (numVertices <= 0)
    {
        return;
    }
    std::set<std::pair<int, int>> S;
    for (int i = 1; i < numVertices; ++i)
    {
        int u = i;
        int v = rand() % i;
        int w = rand() % 50 + 1;
        S.insert({std::min(u, v), std::max(u, v)});
        insertEdge(u, v, w);
    }

    int extraEdges = numEdges - (numVertices - 1);

    for (int i = 0; i < extraEdges; ++i)
    {
        int u = rand() % numVertices;
        int v = rand() % numVertices;
        if(S.find({std::min(u, v), std::max(u, v)}) != S.end()) continue;
        S.insert({std::min(u, v), std::max(u, v)});
        if (u != v)
        {
            int w = rand() % 50 + 1;
            insertEdge(u, v, w);
        }
    }
}

std::vector<Edge> MST::kruskal()
{
    std::vector<Edge> result;

    if (vertices.empty())
    {
        return result;
    }

    std::vector<Edge> sortedEdges = edges;
    std::sort(sortedEdges.begin(), sortedEdges.end());

    DSU dsu;

    for (int v : vertices)
    {
        dsu.make_set(v);
    }

    for (Edge e : sortedEdges)
    {
        if (dsu.union_sets(e.u, e.v))
        {
            result.push_back(e);
        }
    }

    return result;
}

std::vector<Edge> MST::prim(int startVertex)
{
    std::vector<Edge> result;

    if (vertices.find(startVertex) == vertices.end())
    {
        return result;
    }

    std::map<int, std::vector<std::pair<int, int>>> adj;

    for (const auto& e : edges)
    {
        adj[e.u].push_back({e.v, e.weight});
        adj[e.v].push_back({e.u, e.weight});
    }

    std::set<int> visited;

    std::priority_queue<
        std::pair<int, std::pair<int, int>>,
        std::vector<std::pair<int, std::pair<int, int>>>,
        std::greater<std::pair<int, std::pair<int, int>>>>
        pq;

    visited.insert(startVertex);

    for (auto neighbor : adj[startVertex])
    {
        pq.push({neighbor.second, {startVertex, neighbor.first}});
    }

    while (!pq.empty() && visited.size() < vertices.size())
    {
        auto top = pq.top();
        pq.pop();

        int w = top.first;
        int u = top.second.first;
        int v = top.second.second;

        if (visited.find(v) == visited.end())
        {
            visited.insert(v);
            result.push_back({u, v, w});

            for (auto neighbor : adj[v])
            {
                if (visited.find(neighbor.first) == visited.end())
                {
                    pq.push({neighbor.second, {v, neighbor.first}});
                }
            }
        }
    }
    return result;
}