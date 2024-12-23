#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>

using namespace std;

struct Point {
    vector<double> coords;

    Point(vector<double> c) : coords(c) {}
};

// Función para calcular la distancia Euclidiana entre dos puntos
double distanceEuclidean(Point* p1, Point* p2) {
    double sum = 0.0;
    for (size_t i = 0; i < p1->coords.size(); i++) {
        sum += (p1->coords[i] - p2->coords[i]) * (p1->coords[i] - p2->coords[i]);
    }
    return sqrt(sum);
}

// Función para encontrar los vecinos de un punto dentro de un radio `epsMax` usando búsqueda a fuerza bruta
vector<int> regionQuery(int pointIdx, double epsMax, vector<Point*>& points) {
    vector<int> neighbors;
    for (size_t i = 0; i < points.size(); i++) {
        if (i != pointIdx) {
            double dist = distanceEuclidean(points[pointIdx], points[i]);
            if (dist <= epsMax) {
                neighbors.push_back(i);
            }
        }
    }
    return neighbors;
}

// Función para leer los puntos desde un archivo CSV
vector<Point*> readCSVFile(const string& filename) {
    vector<Point*> points;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << filename << endl;
        return points;
    }

    string line;
    while (getline(file, line)) {
        istringstream ss(line);
        vector<double> values;
        string value;

        while (getline(ss, value, ',')) {
            values.push_back(stod(value));
        }

        points.push_back(new Point(values));
    }

    file.close();
    return points;
}

// Función para ejecutar OPTICS con fuerza bruta
void fuerzaBrutaOptics(vector<Point*>& points, double epsMax, int minPoints, vector<int>& clusterLabels) {
    int n = points.size();
    vector<bool> visited(n, false);
    vector<int> ordering;
    vector<double> reachabilityDistance(n, -1);
    vector<int> coreDistances(n, -1);
    clusterLabels.resize(n, -1);  // -1 indica que aún no está asignado a un clúster

    int clusterID = 0;

    // Procesar cada punto
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            visited[i] = true;

            // Encuentra los vecinos de este punto
            vector<int> neighbors = regionQuery(i, epsMax, points);

            if (neighbors.size() >= minPoints) {
                coreDistances[i] = epsMax;  // Definir distancia núcleo
                ordering.push_back(i);
                clusterLabels[i] = clusterID; // Asignar clúster al primer punto encontrado

                // Expandir el clúster
                for (size_t j = 0; j < neighbors.size(); ++j) {
                    int neighborIdx = neighbors[j];
                    if (!visited[neighborIdx]) {
                        visited[neighborIdx] = true;
                        ordering.push_back(neighborIdx);
                        clusterLabels[neighborIdx] = clusterID; // Asignar clúster a los vecinos
                    }
                }

                // Incrementar el identificador del clúster
                clusterID++;
            } else {
                reachabilityDistance[i] = epsMax;  // Punto no es un núcleo, asignar distancia de alcance
            }
        }
    }

    // Imprimir el orden de los puntos y sus clústeres
    for (size_t i = 0; i < ordering.size(); i++) {
        cout << "Punto: (";
        for (size_t j = 0; j < points[ordering[i]]->coords.size(); j++) {
            cout << points[ordering[i]]->coords[j];
            if (j < points[ordering[i]]->coords.size() - 1) cout << ", ";
        }
        cout << ") pertenece al clúster: " << clusterLabels[ordering[i]] << endl;
    }
}

// Función principal
int main() {
    // Leer los datos del archivo CSV
    vector<Point*> data = readCSVFile("dataset.csv");

    // Medir el tiempo de ejecución
    auto start = std::chrono::high_resolution_clock::now();

    // Parámetros para OPTICS
    double epsMax = 0.05; // Radio máximo para los vecinos
    int minPoints = 4;    // Número mínimo de puntos para formar un clúster

    vector<int> clusterLabels; // Etiquetas de los clústeres

    // Ejecutar OPTICS con fuerza bruta
    fuerzaBrutaOptics(data, epsMax, minPoints, clusterLabels);

    // Imprimir el tiempo de ejecución
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Tiempo de ejecución: " << duration.count() << " microsegundos" << std::endl;

    // Liberar memoria de los puntos
    for (auto p : data) {
        delete p;
    }

    return 0;
}
