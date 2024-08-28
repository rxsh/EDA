#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <fstream>

using namespace std;

vector<vector<double>> createPoints(const int numPoints, int dimension){

    random_device rd;  // Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    uniform_real_distribution<> dis(0.0, 1.0);

    vector<vector<double>> points(numPoints, vector<double> (dimension));

    for(int i=0; i<numPoints; i++){
        for(int j=0; j<dimension;j++){
            points[i][j] = dis(gen);
        }
    }
    return points;
}

double euclidianDistance(vector<double>& p1, vector<double>& p2){
    double sum = 0;
    for(int i=0; i<p1.size(); i++){
        sum += pow(p1[i] - p2[i], 2);
    }

    return sqrt(sum);
}

int main(){

    //vector<int> dimensiones = {10,50,100,500,1000,2000,5000};
    int d = 5000; // Aqui cambiamos la dimension 

    vector<vector<double>> points = createPoints(100,d);
    vector<double> distances;

    for(int i=0; i<points.size();i++){
        for(int j=i+1; j<points.size(); j++){
            //cout << euclidianDistance(points[i], points[j]) << endl;
            distances.push_back(euclidianDistance(points[i],points[j]));
        }
    }

    ofstream file("distancias.txt");
    for (const auto& distancia : distances) {
        file << distancia << endl;
    }
    file.close();

    return 0;
}
