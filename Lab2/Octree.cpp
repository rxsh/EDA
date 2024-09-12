#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct Point {
    int x;
    int y;
    int z;

    Point(int a, int b, int c) : x(a), y(b), z(c) {}
};


class Octree {
private:
    Octree *children[8];   // Los 8 hijos del nodo
    vector<Point> points;  // Puntos almacenados en este nodo
    
    Point bottomLeft;  // Coordenada de la esquina inferior izquierda del nodo
    double h;  // Longitud del lado del cubo
    
    int nPoints;  // Número de puntos almacenados en el nodo

    // Subdivide el nodo actual en 8 hijos.
    void octant();

public:
    Octree(Point bottomLeft, double h);
    
    bool exist(const Point &p);
    void insert(const Point &p);
    Point find_closest(const Point &, int);
    void print_tree(int level = 0);
    
};

// Constructor de Octree
Octree::Octree(Point bottomLeft, double h) : bottomLeft(bottomLeft), h(h), nPoints(0) {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
}

void Octree::octant() {
    
    double half_h = h / 2;

    children[0] = new Octree(Point(bottomLeft.x, bottomLeft.y, bottomLeft.z), half_h);
    children[1] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y, bottomLeft.z), half_h);
    children[2] = new Octree(Point(bottomLeft.x, bottomLeft.y + half_h, bottomLeft.z), half_h);
    children[3] = new Octree(Point(bottomLeft.x, bottomLeft.y, bottomLeft.z + half_h), half_h);
    children[4] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y + half_h, bottomLeft.z), half_h);
    children[5] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y, bottomLeft.z + half_h), half_h);
    children[6] = new Octree(Point(bottomLeft.x, bottomLeft.y + half_h, bottomLeft.z + half_h), half_h);
    children[7] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y + half_h, bottomLeft.z + half_h), half_h);
}

bool Octree::exist(const Point &p) {
    if (p.x < bottomLeft.x || p.x >= bottomLeft.x + h ||
        p.y < bottomLeft.y || p.y >= bottomLeft.y + h ||
        p.z < bottomLeft.z || p.z >= bottomLeft.z + h) {
        return false;
    }
    
    for (const Point &point : points) {
        if (point.x == p.x && point.y == p.y && point.z == p.z) {
            return true;
        }
    }
    
    if (children[0] != nullptr) {
        for (int i = 0; i < 8; ++i) {
            if (children[i]->exist(p)) {
                return true;
            }
        }
    }
    
    return false;
}

void Octree::insert(const Point &p) {
    if (p.x < bottomLeft.x || p.x >= bottomLeft.x + h ||
        p.y < bottomLeft.y || p.y >= bottomLeft.y + h ||
        p.z < bottomLeft.z || p.z >= bottomLeft.z + h) {
        return; 
    }
    
    if (nPoints < 4) {
        points.push_back(p);
        nPoints++;
    } else {
        if (children[0] == nullptr) {
            octant();
        }
        for (int i = 0; i < 8; ++i) {
            children[i]->insert(p);
        }
    }
}


Point Octree::find_closest(const Point &p, int radius) {
    double minDist = INT_MAX;
    Point near = Point(0, 0, 0);

    for (const Point &point : points) {
        if (point.x == p.x && point.y == p.y && point.z == p.z) {
            continue;
        }

        double dist = sqrt(pow(p.x - point.x, 2) +
                           pow(p.y - point.y, 2) +
                           pow(p.z - point.z, 2));

        if (dist < minDist) {
            minDist = dist;
            near = point;
        }
    }

    if (children[0] != nullptr) {
        for (int i = 0; i < 8; ++i) {
            Point childClosest = children[i]->find_closest(p, radius);
            
            if (childClosest.x == p.x && childClosest.y == p.y && childClosest.z == p.z) {
                continue;
            }

            double dist = sqrt(pow(p.x - childClosest.x, 2) +
                               pow(p.y - childClosest.y, 2) +
                               pow(p.z - childClosest.z, 2));
            if (dist < minDist) {
                minDist = dist;
                near = childClosest;
            }
        }
    }

    if (near.x == p.x && near.y == p.y && near.z == p.z) {
        return Point(0, 0, 0); 
    }

    return near;
}

void readCSV(const string &filename, Octree &octree) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string pointX, pointY, pointZ;
        
        getline(ss, pointX, ',');
        getline(ss, pointY, ',');
        getline(ss, pointZ, ',');

        int x = stoi(pointX);
        int y = stoi(pointY);
        int z = stoi(pointZ);

        Point p(x, y, z);
        octree.insert(p);
    }
}

int main() {
    Point bottomLeft(-500, -500, -500);
    double SL = 1000;
    Octree octree(bottomLeft, SL);

    string filename = "C:/Users/cesar/Desktop/DS-EDA/Lab2/points1.csv";
    readCSV(filename, octree);

    Point queryPoint(155,208,47);
    if (octree.exist(queryPoint)) {
        cout << "El punto (" << queryPoint.x << ", " << queryPoint.y << ", " << queryPoint.z << ") existe en el Octree." <<endl;
    } else {
        cout << "El punto no existe en el Octree." << endl;
    }

    Point nearest = octree.find_closest(queryPoint, 100);
    if (nearest.x == 0 && nearest.y == 0 && nearest.z == 0) {
        cout << "No se pudo encontrar ningun punto cercano.\n";
    } else {
        cout << "Punto mas cercano a (" << queryPoint.x << ", " << queryPoint.y << ", " << queryPoint.z
                  << ") es (" << nearest.x << ", " << nearest.y << ", " << nearest.z << ").\n";
    }



    return 0;
}
