#include "Edge.hpp"

Edge::Edge(Vertex *a, Vertex *b) : _a(a), _b(b)
{
    _a->increaseDegree();
    _b->increaseDegree();
};

Edge::~Edge()
{
    _a->decreaseDegree();
    _b->decreaseDegree();
}

void Edge::calcDistance()
{
    _updateA = false;
    _updateB = false;
    unsigned distanceA = _a->getDistance();
    unsigned distanceB = _b->getDistance();

    // If both 2 nodes distances are not calculated, return
    if (distanceA == distanceB)
        return;

    if (distanceA > distanceB)
    {
        if (distanceA > distanceB + 1)
        {
            // cout << "Node" << _a->getId() << " distance: " << distanceA << "  "
            //      << "Node" << _b->getId() << " distanece: " << distanceB << endl;
            // cout << "Updating Node" << _a->getId() << " distance to " << distanceB + 1
            //      << ", Assign path: " << _b->getPath() << endl;
            _newDistA = distanceB + 1;
            _newPathA = _b->getPath();
            _updateA = true;
        }
    }
    else if (distanceB > distanceA)
    {

        if (distanceB > distanceA + 1)
        {
            // cout << "Node" << _a->getId() << " distance: " << distanceA << "  "
            //      << "Node" << _b->getId() << " distanece: " << distanceB << endl;
            // cout << "Updating Node" << _b->getId() << " distance to " << distanceA + 1
            //      << ", Assign path: " << _a->getPath() << endl;

            _newDistB = distanceA + 1;
            _newPathB = _a->getPath();
            _updateB = true;
        }
    }
}
void Edge::update()
{
    if (_updateA)
    {
        _a->update(_newDistA, _newPathA);
        _updateA = false;
    }
    if (_updateB)
    {
        _b->update(_newDistB, _newPathB);
        _updateB = false;
    }
}

bool Edge::containV(Vertex *v) const
{
    return v == _a || v == _b;
}
