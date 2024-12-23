#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <queue>
#include <SFML/Graphics.hpp>
#include <limits> 

using namespace sf;
using namespace std;



struct Point {
    vector<double> Dimensions;
    int cluster = -1;
    double rDis = std::numeric_limits<double>::max(); 
    Point(vector<double> v) {
        Dimensions = v;
    }
};


struct knode {
    Point* Punto;
    knode* children[2] = { nullptr };
    knode(Point* p) { Punto = p; }
};


struct kdtree {
    knode* root = nullptr;
    void Insert(Point* k) {
        if (root==nullptr) {
            root = new knode(k);
        }
        else {
            int eje = 0;
            knode* p = root;
            int tam = k->Dimensions.size();
            while (p->children[k->Dimensions[eje] > p->Punto->Dimensions[eje]] != nullptr) {
                p = p->children[k->Dimensions[eje] > p->Punto->Dimensions[eje]];
                eje = (eje + 1) % tam;
            }
            p->children[k->Dimensions[eje] > p->Punto->Dimensions[eje]] = new knode(k);
        }
    }

    void setPoints(vector<Point*> v) {
        for (auto& it : v) {
            Insert(it);
        }
    }

    void RegionQuery(Point* p, double R, knode* node, vector<Point*>& result, int depth = 0) {
        if (!node) {
            return;
        }

        int tam = p->Dimensions.size();
        int axis = depth % tam;

        // calcular la distancia entre el nodo actual y el punto central
        double distance = 0;
        for (int i = 0; i < tam; ++i) {
            distance += pow(node->Punto->Dimensions[i] - p->Dimensions[i], 2);
        }
        distance = sqrt(distance);

        // agregar el punto al resultado si está dentro del radio
        if (distance <= R) {
            result.push_back(node->Punto);
        }

        // verificar en qué subárbol continuar la búsqueda
        if (p->Dimensions[axis] - R <= node->Punto->Dimensions[axis]) {
            RegionQuery(p, R, node->children[0], result, depth + 1);
        }
        if (p->Dimensions[axis] + R >= node->Punto->Dimensions[axis]) {
            RegionQuery(p, R, node->children[1], result, depth + 1);
        }
    }



    vector<pair<double, Point*>> KNearestNeighbors(Point* target, int K) {
        priority_queue<pair<double, Point*>> pq;

        KNNHelper(root, target, K, pq, 0);

        vector<pair<double, Point*>> result;
        while (!pq.empty()) {
            result.push_back(pq.top());
            pq.pop();
        }

        reverse(result.begin(), result.end()); // para tener el más cercano primero

        return result;
    }

    // función auxiliar para la búsqueda de K vecinos más cercanos
    void KNNHelper(knode* node, Point* target, int K, priority_queue<pair<double, Point*>>& pq, int depth = 0) {
        if (!node) {
            return;
        }

        int tam = target->Dimensions.size();
        int axis = depth % tam;

        // calcular la distancia entre el nodo actual y el punto objetivo
        double distance = 0;
        for (int i = 0; i < tam; ++i) {
            distance += pow(node->Punto->Dimensions[i] - target->Dimensions[i], 2);
        }
        distance = sqrt(distance);

        // insertar en la cola de prioridad
        pq.push({ distance, node->Punto });

        // mantener K elementos en la cola de prioridad
        if (pq.size() > K) {
            pq.pop();
        }

        // decidir qué subárbol explorar
        if (target->Dimensions[axis] < node->Punto->Dimensions[axis]) {
            KNNHelper(node->children[0], target, K, pq, depth + 1);
        }
        else {
            KNNHelper(node->children[1], target, K, pq, depth + 1);
        }

        // verificar si es necesario explorar el otro subárbol
        if (abs(target->Dimensions[axis] - node->Punto->Dimensions[axis]) < pq.top().first || pq.size() < K) {
            if (target->Dimensions[axis] >= node->Punto->Dimensions[axis]) {
                KNNHelper(node->children[0], target, K, pq, depth + 1);
            }
            else {
                KNNHelper(node->children[1], target, K, pq, depth + 1);
            }
        }
    }
};

struct Optics {

    kdtree* kd;
    vector<Point*> Points;
    
    void setp(vector<Point*> v) {
        Points = v;
    }
    
    bool findd(vector<Point*>& ordering, Point* p) {
        return find(ordering.begin(), ordering.end(), p) != ordering.end();
    }

    vector<Point*> optics(double epsMax, int minPoints) {
        vector<Point*> ordering;
        kd = new kdtree;
        kd->setPoints(Points);
        int i = 0;

        for (auto& p : Points) {
            if (findd(ordering,p)) {
                continue;
            }
            ordering.push_back(p);
            vector<Point*> neighbors;
            kd->RegionQuery(p, epsMax, kd->root, neighbors);
            if (neighbors.size() >= minPoints) {
                queue<Point*> toProcess;
                update(p, toProcess, ordering, epsMax, minPoints);
                while (toProcess.size() != 0) {
                    Point* q = toProcess.front();
                    toProcess.pop();
                    update(q, toProcess, ordering, epsMax, minPoints);
                }
            }
        }
        return ordering;
    }


    double Distance(Point a, Point b) {
        double aux = 0;
        for (int i = 0; i < a.Dimensions.size(); i++) {
            aux = aux + pow(a.Dimensions[i] - b.Dimensions[i], 2);
        }
        return sqrt(aux);
    }

    void update(Point* p, queue<Point*>& priority, vector<Point*>& ordering, double eps, int minP) {
        vector<Point*> neighbors;
        kd->RegionQuery(p, eps, kd->root, neighbors);

        if (neighbors.size() < minP) {
            return;
        }

        for (auto*& q : neighbors) {
            if (findd(ordering, q)) {
                continue;
            } else {
                auto aux = kd->KNearestNeighbors(p, minP);
                double nRD = max(aux[aux.size() - 1].first, Distance(*p, *q));

                if (q->rDis == std::numeric_limits<double>::max()) { // Si no se ha asignado rDis
                    q->rDis = nRD;
                    priority.push(q);
                } else if (nRD < q->rDis) {
                    q->rDis = nRD;
                }
            }
        }
    }

void opticsCluster(vector<Point*> ordering, double eps) {
    int current = 0;
    bool increment = false;

    // Mapa para almacenar los puntos por cluster
    std::map<int, vector<Point*>> clusters;

    for (int i = 0; i < ordering.size(); i++) {
        // Si el punto no tiene rDis válida o está fuera del radio eps
        if (ordering[i]->rDis == std::numeric_limits<double>::max() || ordering[i]->rDis > eps) {
            increment = true;
        } else {
            if (increment) {
                current++; // Nuevo cluster
                ordering[i - 1]->cluster = current;
                clusters[current].push_back(ordering[i - 1]); // Agregar al nuevo cluster
                increment = false;
            }
            ordering[i]->cluster = current; // Asignar al cluster actual
            clusters[current].push_back(ordering[i]); // Agregar al cluster
        }
    }

    // Imprimir los clusters
    for (const auto& cluster : clusters) {
        cout << "Cluster " << cluster.first << ":\n";
        for (const auto& point : cluster.second) {
            cout << "  ("; // Imprimir coordenadas del punto
            for (size_t j = 0; j < point->Dimensions.size(); j++) {
                cout << point->Dimensions[j];
                if (j < point->Dimensions.size() - 1) {
                    cout << ", ";
                }
            }
            cout << ")\n";
        }
    }
}

};

CircleShape createCircle(Point* p, Color c, const Vector2u& windowSize, double minX, double maxX, double minY, double maxY, bool cen, int tam = 10) {
    CircleShape cir(tam);
    double scaledX = ((p->Dimensions[0] - minX) / (maxX - minX));
    double scaledY = 1.0 - ((p->Dimensions[1] - minY) / (maxY - minY));
    float x = 50 + (scaledX * (windowSize.x - 100));
    float y = 50 + (scaledY * (windowSize.y - 100));
    cir.setPosition(x, y);
    cir.setOrigin(tam / 2, tam / 2);
    cir.setFillColor(c);
    if (cen == false) {
        cir.setOutlineColor(c);
    }
    else {
        cir.setOutlineColor(Color::Black);
    }
    cir.setOutlineThickness(1);
    return cir;
}

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
            try {
                values.push_back(stod(value));
            } catch (const invalid_argument& e) {
                cerr << "Error al convertir el valor a número: " << value << endl;
                file.close();
                return points;
            }
        }

        if (values.size() == 6) {
            points.push_back(new Point(values));
        } else {
            cerr << "Error: Se esperaban 6 columnas, pero se encontraron " << values.size() << endl;
            file.close();
            return points;
        }
    }

    file.close();
    return points;
}


int main() {
    RenderWindow window(VideoMode(900, 600), "EDA");
    window.setFramerateLimit(30);
    Event event;

    vector<Point*> data = readCSVFile("dataset.csv");

    Optics opticsAlgorithm;
    opticsAlgorithm.setp(data);

    auto start = std::chrono::high_resolution_clock::now();


    double epsMax = 0.09;
    int minPoints = 3;

    //// realizar clustering con OPTICS
    vector<Point*> ordering = opticsAlgorithm.optics(epsMax, minPoints);
    cout << "termino" << endl;
    //// asignar clusters
    opticsAlgorithm.opticsCluster(ordering, epsMax);
    cout << "fin";

    auto stop = std::chrono::high_resolution_clock::now();

    // calcular la diferencia de tiempo
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // imprimir en microsegundos
    std::cout << "Tiempo: " << duration.count() << " microsegundos" << std::endl;

    double minX = 0;
    double maxX = 1;
    double minY = 0;
    double maxY = 0.7;

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        window.clear(Color::Black);
        Color c;
        for (int i = 0; i < ordering.size(); i++) {
        if (ordering[i]->cluster == 0) {
            c = Color::Red;
        }
        else if (ordering[i]->cluster == 1) {
            c = Color::Blue;
        }
        else if ( ordering[i]->cluster == 2) {
            c = Color::Green;
        }
        else if ( ordering[i]->cluster == 3) {
            c = Color::Yellow;
        }
        else if ( ordering[i]->cluster == 4) {
            c = Color::Magenta;
        }
        else if ( ordering[i]->cluster == 5) {
            c = Color::Cyan;
        }
        else if ( ordering[i]->cluster == 6) {
            c = Color::White;
        }
        else if (ordering[i]->cluster == 7) {
            c = Color(155,155,155);
        }
        else if (ordering[i]->cluster == 7) {
            c = Color(221, 191, 218);
        }
        if (ordering[i]->cluster != -1 and ordering[i]->cluster<8) {
            window.draw(createCircle(ordering[i], c, window.getSize(), minX, maxX, minY, maxY, false, 3));
        }
    }

        window.display();
    }

    return 0;
}