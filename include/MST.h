#pragma once

#include <vector>
#include <set>
#include <map>
#include <unordered_map>

struct Edge
{
    int u, v, weight;

    bool operator<(const Edge& other) const
    {
        return weight < other.weight;
    }
};

class MST
{
private:
    std::vector<Edge> edges;
    std::set<int> vertices;

public:
    MST();
    ~MST();

    void clear();

    void insertEdge(int u, int v, int weight);

    void generateRandomGraph(int numVertices, int numEdges);

    std::vector<Edge> kruskal();

    std::vector<Edge> prim(int startVertex);

    const std::vector<Edge>& getEdges() const;

    const std::set<int>& getVertices() const;
};