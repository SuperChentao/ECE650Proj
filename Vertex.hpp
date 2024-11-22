#pragma once

#include <iostream>
#include <string>
#include <climits>

#define INF UINT_MAX

using std::cerr;
using std::cout;
using std::endl;
using std::string;

class Vertex
{
private:
    int _id;
    unsigned int _distance = INF;
    string _path;
    bool _reached = false;
    int _degree = 0;

public:
    Vertex(unsigned int _id = -1);
    ~Vertex();

    int getId() const;
    unsigned int getDistance() const;
    string getPath() const;
    bool isReach() const;
    int getDegree() const;

    void update(unsigned int, string);
    void setDistance(unsigned int);
    void increaseDegree();
    void decreaseDegree();
};