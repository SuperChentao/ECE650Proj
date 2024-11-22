#pragma once

#include "Vertex.hpp"

class Edge
{
private:
    unsigned int _newDistA = INF;
    unsigned int _newDistB = INF;
    string _newPathA;
    string _newPathB;
    bool _updateA = false;
    bool _updateB = false;

public:
    Vertex *_a;
    Vertex *_b;
    Edge(Vertex *a, Vertex *b);
    ~Edge();
    void calcDistance();
    void update();
    bool containV(Vertex *v) const;
};