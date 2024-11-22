#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

// defined std::unique_ptr
#include <memory>
// defines Var and Lit
#include "minisat/core/SolverTypes.h"
// defines Solver
#include "minisat/core/Solver.h"

#include "Edge.hpp"

class EdgeManager
{
private:
    int _vertexNum;
    int _sourceVID;
    int _endVID;
    bool _validGraph;

    Vertex **_vertices;
    std::vector<Edge *> _edges;

    void parseEdges(const string &input)
    {
        std::stringstream ss(input);
        char c;
        int v1, v2;
        while (ss >> c)
        {
            if (c == '<')
            {
                ss >> v1 >> c >> v2;
                if (v1 == v2)
                {
                    cerr << "Error: edge:<" << v1 << "," << v2 << "> is invalid" << endl;
                    _validGraph = false;
                    return;
                }
                if (v1 < 1 || v1 > _vertexNum)
                {
                    cerr << "Error: vertex:" << v1 << " in the edge <" << v1 << "," << v2 << "> doesn't exist" << endl;
                    _validGraph = false;
                    return;
                }
                if (v2 < 1 || v2 > _vertexNum)
                {
                    cerr << "Error: vertex:" << v2 << " in the edge <" << v1 << "," << v2 << "> doesn't exist" << endl;
                    _validGraph = false;
                    return;
                }

                for (Edge *e : _edges)
                {
                    if ((e->_a->getId() == v1 && e->_b->getId() == v2) || (e->_a->getId() == v2 && e->_b->getId() == v1))
                    {
                        cerr << "Error: edge:<" << v1 << "," << v2 << "> already added to graph" << endl;
                        _validGraph = false;
                        return;
                    }
                }

                if (_vertices[v1 - 1] == nullptr)
                    _vertices[v1 - 1] = new Vertex(v1);
                if (_vertices[v2 - 1] == nullptr)
                    _vertices[v2 - 1] = new Vertex(v2);
                Edge *e = new Edge(_vertices[v1 - 1], _vertices[v2 - 1]);
                _edges.push_back(e);
            }
        }
    }

    void print_propositions(std::vector<std::vector<Minisat::Lit>> &propositions, Minisat::Solver *solver)
    {
        int n = propositions.size();
        int k = propositions[0].size();
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < k; j++)
            {
                std::cout << "X_" << i + 1 << "_" << j + 1 << "=" << Minisat::toInt(solver->modelValue(propositions[i][j])) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    string print_vertex_cover(std::vector<std::vector<Minisat::Lit>> &propositions, Minisat::Solver *solver)
    {
        std::vector<int> result;
        int n = propositions.size();
        int k = propositions[0].size();
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (toInt(solver->modelValue(propositions[i][j])) == 0)
                {
                    result.push_back(i + 1);
                    break;
                }
            }
        }

        std::sort(result.begin(), result.end());
        std::ostringstream oss;
        oss << result[0];
        for (size_t i = 1; i < result.size(); i++)
        {
            oss << "," << result[i];
        }
        return oss.str();
    }

    std::vector<std::vector<Minisat::Lit>> initialize_propositions(int n, int k, Minisat::Solver *solver)
    {
        std::vector<std::vector<Minisat::Lit>> propositions(n, std::vector<Minisat::Lit>(k));
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < k; ++j)
            {
                propositions[i][j] = Minisat::mkLit(solver->newVar());
            }
        }
        return propositions;
    }

    int highestDegreeVertex()
    {
        int k = -1;
        int degree = -1;
        for (int i = 0; i < _vertexNum; i++)
        {
            if (_vertices[i] == nullptr)
                continue;

            if (_vertices[i]->getDegree() > degree)
            {
                degree = _vertices[i]->getDegree();
                k = i;
            }
        }
        return k;
    }

public:
    EdgeManager(int V, string edgeInput) : _vertexNum(V)
    {
        _validGraph = true;
        _vertices = new Vertex *[_vertexNum];
        for (int i = 0; i < _vertexNum; i++)
        {
            _vertices[i] = nullptr;
        }

        this->parseEdges(edgeInput);
    }

    ~EdgeManager()
    {
        for (Edge *e : _edges)
        {
            delete e;
        }
        _edges.clear();

        if (_vertices != nullptr)
        {
            for (int i = 0; i < _vertexNum; i++)
            {
                if (_vertices[i] != nullptr)
                {
                    delete _vertices[i];
                    _vertices[i] = nullptr;
                }
            }
            delete[] _vertices;
        }
    }

    string findVertexCover()
    {
        if (!_validGraph)
        {
            string error = "Error: invalid graph";
            return error;
        }

        std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
        std::vector<std::vector<Minisat::Lit>> propositions;

        bool res = false;
        int n = _vertexNum, k = 1, i = 0, j = 0;

        while (true)
        {
            // cout << "k = " << k << endl;
            int count = 0;
            // Condition 1
            propositions = initialize_propositions(n, k, solver.get());
            std::vector<Minisat::vec<Minisat::Lit>> condition1(k);
            for (j = 0; j < k; j++)
            {
                Minisat::vec<Minisat::Lit> &Lits = condition1[j];
                for (i = 0; i < n; i++)
                {
                    Lits.push(propositions[i][j]);
                }
                solver->addClause(Lits);
                count++;
            }
            // std::cout << "Add " << count << "Clauses\n";

            // Condition 2
            count = 0;
            for (i = 0; i < n; i++)
            {
                for (j = 0; j < k; j++)
                {
                    for (int s = j + 1; s < k; s++)
                    {
                        solver->addClause(~propositions[i][j], ~propositions[i][s]);
                        count++;
                    }
                }
            }
            // std::cout << "Add " << count << "Clauses\n";

            // Condition 3
            count = 0;
            for (j = 0; j < k; j++)
            {
                for (i = 0; i < n; i++)
                {
                    for (int s = i + 1; s < n; s++)
                    {
                        solver->addClause(~propositions[i][j], ~propositions[s][j]);
                        count++;
                    }
                }
            }
            // std::cout << "Add " << count << "Clauses\n";

            // Condition 4
            count = 0;
            int v1 = 0, v2 = 0;
            std::vector<Minisat::vec<Minisat::Lit>> condition4(_edges.size());
            i = 0;
            for (Edge *e : _edges)
            {
                v1 = e->_a->getId();
                v2 = e->_b->getId();
                Minisat::vec<Minisat::Lit> &Lits1 = condition4[i++];
                // Minisat::vec<Minisat::Lit> &Lits2 = condition4[i++];
                for (j = 0; j < k; j++)
                {
                    Lits1.push(propositions[v1 - 1][j]);
                    Lits1.push(propositions[v2 - 1][j]);
                }
                solver->addClause(Lits1);
                // solver->addClause(Lits2);
                count++;
            }
            // std::cout << "Add " << count << "Clauses\n\n";

            res = solver->solve();
            if (res)
            {
                // std::cout << "The result for vector is: " << res << "\n";
                // print_propositions(propositions, solver.get());
                string vc = print_vertex_cover(propositions, solver.get());
                solver.reset(new Minisat::Solver());
                return vc;
            }

            solver.reset(new Minisat::Solver());
            ++k;
            // if (k > n)
            // {
            //     cout << "k = n" << endl;
            //     print_propositions(propositions, solver.get());
            //     return;
            // }
        }
    }

    void calcPath(int source, int end)
    {
        if (!_validGraph)
        {
            cerr << "Error: can't generate shortest path in invalid graph" << endl;
            return;
        }

        if (source == end)
        {
            cerr << "Error: no edge exists from vertex " << source << " to vertex " << end << endl;
            return;
        }

        if (source > _vertexNum || source < 1)
        {
            cerr << "Error: source vertex:" << source << " doesn't exist" << endl;
            return;
        }

        if (end > _vertexNum || end < 1)
        {
            cerr << "Error: destination vertex:" << end << " doesn't exist" << endl;
            return;
        }

        _sourceVID = source;
        _endVID = end;
        calcPath2();
    }

    void calcPath2()
    {
        if (_vertices[_sourceVID - 1] != nullptr)
        {
            _vertices[_sourceVID - 1]->setDistance(0);
        }
        else
        {
            cerr << "Error: path to from vertex " << _sourceVID << " to vertex:" << _endVID << " doesn't exist" << endl;
            return;
        }

        int loopTimes = _vertexNum - 1;
        for (int i = 0; i < loopTimes; i++)
        {
            for (Edge *e : _edges)
            {
                e->calcDistance();
            }
            for (Edge *e : _edges)
            {
                e->update();
            }
        }

        if (_vertices[_endVID - 1] != nullptr && _vertices[_endVID - 1]->isReach())
        {
            cout << _vertices[_endVID - 1]->getPath() << endl;
        }
        else
        {
            cerr << "Error: path to from vertex " << _sourceVID << " to vertex:" << _endVID << " doesn't exist" << endl;
        }
    }

    bool isValid()
    {
        return _validGraph;
    }

    string approxVC1()
    {
        int i;
        Vertex *v;
        std::ostringstream oss;

        while (_edges.size() > 0)
        {
            i = highestDegreeVertex();
            v = _vertices[i];

            // cout << "vertex " << i + 1 << " has highest degree: " << v->getDegree() << endl;
            for (auto it = _edges.begin(); it != _edges.end();)
            {
                if ((*it)->containV(v))
                {
                    delete *it;
                    it = _edges.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            if (v != nullptr)
            {
                delete v;
                _vertices[i] = nullptr;
            }

            oss << i + 1 << ",";
        }
        string result = oss.str();
        result.pop_back();
        return result;
    }

    string approxVC2()
    {
        Edge *e;
        Vertex *v1, *v2;
        int idx1, idx2;
        std::ostringstream oss;

        while (_edges.size() > 0)
        {
            e = _edges.front();
            v1 = e->_a;
            v2 = e->_b;
            idx1 = v1->getId();
            idx2 = v2->getId();

            oss << idx1 << "," << idx2 << ",";

            for (std::vector<Edge *>::iterator it = _edges.begin(); it != _edges.end();)
            {
                if ((*it)->containV(v1) || (*it)->containV(v2))
                {
                    delete *it;
                    it = _edges.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            delete v1;
            delete v2;
            _vertices[idx1 - 1] = nullptr;
            _vertices[idx2 - 1] = nullptr;
        }

        string result = oss.str();
        result.pop_back();
        return result;
    }
};