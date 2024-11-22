#include "Vertex.hpp"

using std::to_string;

Vertex::Vertex(unsigned int id) : _id(id)
{
    _path = to_string(_id);
    // cout << "Vertex " << _id << "created" << endl;
}

Vertex::~Vertex()
{
    // cout << "Node:" << _id << "deleted" << endl;
}

int Vertex::getId() const
{
    return _id;
}

unsigned int Vertex::getDistance() const
{
    return _distance;
}

string Vertex::getPath() const
{
    return _path;
};

bool Vertex::isReach() const
{
    return _reached;
}

int Vertex::getDegree() const
{
    return _degree;
}

void Vertex::increaseDegree()
{
    _degree++;
}

void Vertex::decreaseDegree()
{
    _degree--;
}

void Vertex::update(unsigned int dist, string prePath)
{
    if (dist < _distance)
    {
        _distance = dist;
        _path = prePath + "-" + _path;
        _reached = true;
        // cout << "Vertex:" << _id << " updated distance:" << _distance << " path: " << _path << endl;
    }
};

void Vertex::setDistance(unsigned int dist)
{
    _distance = dist;
}
