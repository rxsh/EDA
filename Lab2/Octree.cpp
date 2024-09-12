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

// Definir cuántos puntos puede almacenar un nodo antes de dividirse.
//const int MAX_POINTS = 4;

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

// Subdivide el nodo en 8 hijos.
void Octree::octant() {
    
    double half_h = h / 2;

    // Generar los 8 hijos del nodo actual.
    children[0] = new Octree(Point(bottomLeft.x, bottomLeft.y, bottomLeft.z), half_h);
    children[1] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y, bottomLeft.z), half_h);
    children[2] = new Octree(Point(bottomLeft.x, bottomLeft.y + half_h, bottomLeft.z), half_h);
    children[3] = new Octree(Point(bottomLeft.x, bottomLeft.y, bottomLeft.z + half_h), half_h);
    children[4] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y + half_h, bottomLeft.z), half_h);
    children[5] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y, bottomLeft.z + half_h), half_h);
    children[6] = new Octree(Point(bottomLeft.x, bottomLeft.y + half_h, bottomLeft.z + half_h), half_h);
    children[7] = new Octree(Point(bottomLeft.x + half_h, bottomLeft.y + half_h, bottomLeft.z + half_h), half_h);
}

// Función para verificar si un punto existe en el Octree.
bool Octree::exist(const Point &p) {
    // Verificar si el punto está dentro del espacio definido por este nodo.
    if (p.x < bottomLeft.x || p.x >= bottomLeft.x + h ||
        p.y < bottomLeft.y || p.y >= bottomLeft.y + h ||
        p.z < bottomLeft.z || p.z >= bottomLeft.z + h) {
        return false;
    }
    
    // Verificar los puntos en el nodo actual.
    for (const Point &point : points) {
        if (point.x == p.x && point.y == p.y && point.z == p.z) {
            return true;
        }
    }
    
    // Verificar en los hijos.
    if (children[0] != nullptr) {
        for (int i = 0; i < 8; ++i) {
            if (children[i]->exist(p)) {
                return true;
            }
        }
    }
    
    return false;
}

// Función para insertar un punto en el Octree.
void Octree::insert(const Point &p) {
    // Verificar si el punto está dentro del espacio definido por este nodo.
    if (p.x < bottomLeft.x || p.x >= bottomLeft.x + h ||
        p.y < bottomLeft.y || p.y >= bottomLeft.y + h ||
        p.z < bottomLeft.z || p.z >= bottomLeft.z + h) {
        return; // Punto fuera del rango del nodo.
    }
    
    // Si el nodo no tiene hijos y tiene espacio, agregar el punto aquí.
    if (nPoints < 4) {
        points.push_back(p);
        nPoints++;
    } else {
        // Si no hay hijos aún, dividir el nodo.
        if (children[0] == nullptr) {
            octant();
        }
        
        // Insertar el punto en el subnodo adecuado.
        for (int i = 0; i < 8; ++i) {
            children[i]->insert(p);
        }
    }
}


// Función para encontrar el punto más cercano dentro de un radio.
Point Octree::find_closest(const Point &p, int radius) {
    // Definir una distancia inicial muy grande.
    double minDist = INT_MAX;
    Point near = Point(0, 0, 0); // Valor por defecto para el punto más cercano.

    // Recorrer puntos en el nodo actual.
    for (const Point &point : points) {
        // Ignorar el punto si es el mismo que el de consulta.
        if (point.x == p.x && point.y == p.y && point.z == p.z) {
            continue;
        }

        // Calcular la distancia entre el punto de consulta y el punto actual.
        double dist = sqrt(pow(p.x - point.x, 2) +
                           pow(p.y - point.y, 2) +
                           pow(p.z - point.z, 2));

        // Actualizar el punto más cercano si encontramos una distancia menor.
        if (dist < minDist) {
            minDist = dist;
            near = point;
        }
    }

    // Comprobar en los nodos hijos.
    if (children[0] != nullptr) {
        for (int i = 0; i < 8; ++i) {
            // Llamada recursiva para encontrar el punto más cercano en los hijos.
            Point childClosest = children[i]->find_closest(p, radius);
            
            // Ignorar el punto de consulta en los hijos.
            if (childClosest.x == p.x && childClosest.y == p.y && childClosest.z == p.z) {
                continue;
            }

            // Calcular la distancia al punto más cercano encontrado en los hijos.
            double dist = sqrt(pow(p.x - childClosest.x, 2) +
                               pow(p.y - childClosest.y, 2) +
                               pow(p.z - childClosest.z, 2));
            // Actualizar el punto más cercano si encontramos una distancia menor.
            if (dist < minDist) {
                minDist = dist;
                near = childClosest;
            }
        }
    }

    // Si el punto más cercano es el mismo que el de consulta, devolver un punto especial.
    if (near.x == p.x && near.y == p.y && near.z == p.z) {
        return Point(0, 0, 0); // Punto especial indicando que no se encontró un punto diferente.
    }

    return near;
}

// Función para leer puntos desde un archivo CSV e insertarlos en el Octree.
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
    // Crear un Octree con bottomLeft en (-500, -500, -500) y un lado de 1000 unidades.
    Point bottomLeft(-500, -500, -500);
    double SL = 1000;
    Octree octree(bottomLeft, SL);

    // Leer puntos desde un archivo CSV y agregarlos al Octree.
    string filename = "C:/Users/cesar/Desktop/DS-EDA/Lab2/points1.csv";
    readCSV(filename, octree);

    // Verificar si un punto existe en el Octree.
    Point queryPoint(155,208,47);
    if (octree.exist(queryPoint)) {
        cout << "El punto (" << queryPoint.x << ", " << queryPoint.y << ", " << queryPoint.z << ") existe en el Octree." <<endl;
    } else {
        cout << "El punto no existe en el Octree." << endl;
    }

    // Encontrar el punto más cercano a otro punto.
    Point nearest = octree.find_closest(queryPoint, 100);
    if (nearest.x == 0 && nearest.y == 0 && nearest.z == 0) {
        cout << "No se pudo encontrar ningun punto cercano.\n";
    } else {
        cout << "Punto mas cercano a (" << queryPoint.x << ", " << queryPoint.y << ", " << queryPoint.z
                  << ") es (" << nearest.x << ", " << nearest.y << ", " << nearest.z << ").\n";
    }



    return 0;
}
