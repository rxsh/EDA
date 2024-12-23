#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <sstream>
#include <string>
#include <utility> 
#include <unordered_map>

using namespace std;

typedef pair<double, bool> Valor; // el bool se usa para ver si no es un campo vacio
//porque como hay columnas vacias al guardarlas en negativo no se tomaran en cuenta para las funciones 



vector<string> atributoscompletos = { "Pregnant     ","Glucose      ","Diastolic_BP","Skin_Fold    ","Serum_Insulin","BMI          ","Diabetes_Pedigree","Age          ","Class        " };
//estos dos vectores son para las impresiones
vector<string> atributos = { "PREG","GLUC","DIAS","SKIN","SERU","BMI ","DIAB","AGE ","CLAS"};

class Punto { //la clase punto son cada individuo
    vector<Valor> dimensiones;

public:
    Punto(const vector<Valor>& v = vector<Valor>(0)) : dimensiones(v) {}

    const vector<Valor>& get_dimensiones() const {
        return dimensiones;
    }

    void set_dimensiones(const vector<Valor>& v) {
        dimensiones = v;
    }

    double distancia(const Punto& otro) const {
        const vector<Valor>& otras_dimensiones = otro.get_dimensiones();
        double suma = 0.0;
        for (size_t i = 0; i < dimensiones.size(); i++) {
            if (dimensiones[i].second && otras_dimensiones[i].second) {
                double diff = dimensiones[i].first - otras_dimensiones[i].first;
                suma += diff * diff;
            }
        }
        return sqrt(suma);
    }
};


class Nodo { //cada nodo en si es una hiperesfera del ball
    vector<Punto> puntos; //
    Punto centroide;
    double radio;
    Nodo* next[2] = { nullptr, nullptr }; //aqui se guardan los hijos

public:
    Nodo*& izquierda = next[0]; //y esto son alias para un acceso mas facil
    Nodo*& derecha = next[1];

    Nodo() : centroide(), radio(0) {}

    void add_punto(const Punto& p) {
        puntos.push_back(p);
    }

    const vector<Punto>& get_puntos() const {
        return puntos;
    }

    void set_centroide(const Punto& c) {
        centroide = c;
    }

    const Punto& get_centroide() const {
        return centroide;
    }

    void set_radio(double r) {
        radio = r;
    }

    double get_radio() const {
        return radio;
    }

};

void PCA(const vector<Punto>& puntos, vector<Punto>& grupo1, vector<Punto>& grupo2) { //el PCA es el algoritmo que se usa para tomar en cuneta
    //la dimension mas importante
    size_t n_dimensiones = puntos[0].get_dimensiones().size();
    size_t n_puntos = puntos.size();

    vector<double> medias(n_dimensiones, 0.0); //las sumas para la media
    vector<int> conteos_validos(n_dimensiones, 0); //guardar la cantidad de valores que se toman en cuenta para la media
    //trate de paralelizarlo pero el rendimiento empeora

    //esto es para una media tomando en cuenta los valores validos y no nulos
    for (const auto& punto : puntos) {
        const vector<Valor>& dims = punto.get_dimensiones();
        for (size_t j = 0; j < n_dimensiones; j++) {
            if (dims[j].second) { 
                medias[j] += dims[j].first;
                conteos_validos[j]++;
            }
        }
    }

    for (size_t j = 0; j < n_dimensiones; j++) { //divide para la media
        if (conteos_validos[j] > 0) {
            medias[j] /= conteos_validos[j];
        }
        else {
            medias[j] = -1.0; 
        }
    }

    vector<vector<double>> covarianza(n_dimensiones, vector<double>(n_dimensiones, 0.0)); 
    vector<int> conteos_pares(n_dimensiones * n_dimensiones, 0);
    //calcula la matriz de covarianza ignorando valores nulos
    for (const auto& punto : puntos) {
        const vector<Valor>& dims = punto.get_dimensiones();
        for (size_t i = 0; i < n_dimensiones; i++) {
            for (size_t j = 0; j < n_dimensiones; j++) {
                if (dims[i].second && dims[j].second) { 
                    covarianza[i][j] += (dims[i].first - medias[i]) * (dims[j].first - medias[j]);
                    conteos_pares[i * n_dimensiones + j]++;
                }
            }
        }
    }

    for (size_t i = 0; i < n_dimensiones; i++) {
        for (size_t j = 0; j < n_dimensiones; j++) {
            int conteo = conteos_pares[i * n_dimensiones + j];
            if (conteo > 1) {
                covarianza[i][j] /= (conteo - 1); 
            }
            else {
                covarianza[i][j] = 0.0; 
            }
        }
    }

    //obtener la varianza (la cual es la diagnonal de la matriz de covarianza)
    //esto nos indica la dispercion de los valores segun cada dimension
    //asi evidenciando cual es la dimension mas importante
    vector<double> varianzas(n_dimensiones, 0.0);
    for (size_t i = 0; i < n_dimensiones; i++) {
        varianzas[i] = covarianza[i][i];
    }

    auto it_max = max_element(varianzas.begin(), varianzas.end());
    size_t dimension_mas_significativa = distance(varianzas.begin(), it_max);
    //identificar la dimension mas importante



    //cout << "La dimension mas significativa es la numero: "<< dimension_mas_significativa + 1 << " con varianza " << *it_max << endl;

    //divide los puntos en base a su dimension mas importante de modo que se divide en grupos segun la media
    for (const auto& punto : puntos) {
        const auto& dims = punto.get_dimensiones();
        if (dims[dimension_mas_significativa].second && dims[dimension_mas_significativa].first < medias[dimension_mas_significativa]) {
            grupo1.push_back(punto);
        }
        else {
            grupo2.push_back(punto);
        }
    }
}



class BallTree {
    Nodo* head = nullptr;
    vector<Punto> puntos;

    Punto calcular_centroide(const vector<Punto>& puntos) { //calcula centroides
        size_t dimensiones = puntos[0].get_dimensiones().size();
        vector<double> suma(dimensiones, 0.0);
        vector<int> conteos_validos(dimensiones, 0);

        for (const auto& punto : puntos) {
            const auto& dim = punto.get_dimensiones();
            for (size_t i = 0; i < dimensiones; ++i) {
                if (dim[i].second) { 
                    suma[i] += dim[i].first;
                    conteos_validos[i]++;
                }
            }
        }

        vector<Valor> centroide_dims(dimensiones);
        for (size_t i = 0; i < dimensiones; ++i) {
            if (conteos_validos[i] > 0) {
                centroide_dims[i] = { suma[i] / conteos_validos[i], true }; 
            }
            else {
                centroide_dims[i] = { -1.0, false }; 
            }
        }

        return Punto(centroide_dims);
    }

    Nodo* construirRaiz(vector<Punto>& puntos) { //se separa en dos funciones de construccion, uno que es para la raiz para lanzar los threads
        if (puntos.empty())
            return nullptr;

        Nodo* nodo = new Nodo(); // Crear nodo raíz
        nodo->set_centroide(calcular_centroide(puntos)); // Calcular el centroide de la raíz

        vector<Punto> izquierda;
        vector<Punto> derecha;

        PCA(puntos, izquierda, derecha); // Dividir puntos en dos grupos según PCA

        if (izquierda.empty() || derecha.empty()) { // Si no hay división posible
            nodo->set_radio(0);
            nodo->add_punto(puntos[0]); // Asignar único punto
            return nodo;
        }

        // Usar threads para construir los hijos
        Nodo* hijoIzquierdo = nullptr;
        Nodo* hijoDerecho = nullptr;

        std::thread threadIzquierda([&]() {
            hijoIzquierdo = construirNodo(izquierda, 1); // Construir hijo izquierdo
            });

        std::thread threadDerecha([&]() {
            hijoDerecho = construirNodo(derecha, 1); // Construir hijo derecho
            });

        threadIzquierda.join();
        threadDerecha.join();

        nodo->izquierda = hijoIzquierdo;
        nodo->derecha = hijoDerecho;

        // Calcular el radio del nodo raíz
        double radio_max = 0.0;
        for (const auto& punto : puntos) {
            double dist = punto.distancia(nodo->get_centroide());
            if (dist > radio_max) {
                radio_max = dist;
            }
        }
        nodo->set_radio(radio_max);

        return nodo;
    }

    Nodo* construirNodo(vector<Punto>& puntos, int profundidad) { //mimsa logica del construir raiz pero sin threads en la llamada reucrsiva
        if (puntos.empty())
            return nullptr;

        Nodo* nodo = new Nodo();
        nodo->set_centroide(calcular_centroide(puntos));

        vector<Punto> izquierda;
        vector<Punto> derecha;

        PCA(puntos, izquierda, derecha);

        if (izquierda.empty() || derecha.empty()) {
            nodo->set_radio(0);
            nodo->add_punto(puntos[0]);
            return nodo;
        }

        nodo->izquierda = construirNodo(izquierda, profundidad + 1);
        nodo->derecha = construirNodo(derecha, profundidad + 1);

        double radio_max = 0.0;
        for (const auto& punto : puntos) {
            double dist = punto.distancia(nodo->get_centroide());
            if (dist > radio_max) {
                radio_max = dist;
            }
        }
        nodo->set_radio(radio_max);

        return nodo;
    }





    void find_recursivo(Nodo* nodo, int dimension, double valor, vector<Punto*>& resultado) {
        if (!nodo) return;

        // Verificar puntos en el nodo actual (solo hojas tienen puntos)
        if (!nodo->izquierda && !nodo->derecha) {
            const auto& punto = nodo->get_puntos().front(); // Obtener el unico punto
            const auto& dimensiones = punto.get_dimensiones();
            if (dimension >= 0 && dimension < dimensiones.size() &&
                dimensiones[dimension].second &&
                fabs(dimensiones[dimension].first - valor) < 1e-6) {
                resultado.push_back(const_cast<Punto*>(&punto)); // Guardar referencia al punto original
            }
            return;
        }

        // Revisar la hiperesfera izquierda
        if (nodo->izquierda) {
            const auto& centroide_izquierda = nodo->izquierda->get_centroide().get_dimensiones();
            if (centroide_izquierda[dimension].second && fabs(centroide_izquierda[dimension].first - valor) <= nodo->izquierda->get_radio()) {
                find_recursivo(nodo->izquierda, dimension, valor, resultado);
            }
        }

        // Revisar la hiperesfera derecha
        if (nodo->derecha) {
            const auto& centroide_derecha = nodo->derecha->get_centroide().get_dimensiones();
            if (centroide_derecha[dimension].second && fabs(centroide_derecha[dimension].first - valor) <= nodo->derecha->get_radio()) {
                find_recursivo(nodo->derecha, dimension, valor, resultado);
            }
        }
    }


    void rangequery_recursivo(Nodo* nodo, int dimension, double valor, double rango, vector<Punto*>& resultado) {
        if (!nodo) return;

        // Caso hoja: Verificar el unico punto almacenado
        if (!nodo->izquierda && !nodo->derecha) {
            const auto& punto = nodo->get_puntos().front();
            const auto& dimensiones = punto.get_dimensiones();
            if (dimension >= 0 && dimension < dimensiones.size() &&
                dimensiones[dimension].second &&
                fabs(dimensiones[dimension].first - valor) <= rango) {
                resultado.push_back(const_cast<Punto*>(&punto)); // Guardar referencia al punto original
            }
            return;
        }

        // Revisar la hiperesfera izquierda
        if (nodo->izquierda) {
            const auto& centroide_izquierda = nodo->izquierda->get_centroide().get_dimensiones();
            if (centroide_izquierda[dimension].second && fabs(centroide_izquierda[dimension].first - valor) - rango <= nodo->izquierda->get_radio()) {
                rangequery_recursivo(nodo->izquierda, dimension, valor, rango, resultado);
            }
        }

        // Revisar la hiperesfera derecha
        if (nodo->derecha) {
            const auto& centroide_derecha = nodo->derecha->get_centroide().get_dimensiones();
            if (centroide_derecha[dimension].second && fabs(centroide_derecha[dimension].first - valor) - rango <= nodo->derecha->get_radio()) {
                rangequery_recursivo(nodo->derecha, dimension, valor, rango, resultado);
            }
        }
    }

    vector<Punto> recalcular_propiedades(Nodo* nodo, int dimension_actualizada) {
        if (!nodo) return {};

        // Vector para almacenar puntos mas bajos del arbol para actualizar
        vector<Punto> puntos_actualizados;

        // cuando se llega a una hoja se actualiza los centroides
        if (!nodo->izquierda && !nodo->derecha) {
            if (!nodo->get_puntos().empty()) {
                const Punto& punto_hoja = nodo->get_puntos().front();
                vector<Valor> dimensiones_actualizadas = nodo->get_centroide().get_dimensiones();

                // Solo actualizar la dimension indicada
                dimensiones_actualizadas[dimension_actualizada] = punto_hoja.get_dimensiones()[dimension_actualizada];
                nodo->set_centroide(Punto(dimensiones_actualizadas));

                nodo->set_radio(0.0); // Radio es 0 en hojas
                puntos_actualizados.push_back(nodo->get_centroide()); // Agregar el punto actualizado
            }
            return puntos_actualizados;
        }

        //vector para almacenar los puntos de las hojas
        vector<Punto> puntos_izquierda;
        vector<Punto> puntos_derecha;

        if (nodo->izquierda) {
            puntos_izquierda = recalcular_propiedades(nodo->izquierda, dimension_actualizada); //espanxir el recalculo
        }
        if (nodo->derecha) {
            puntos_derecha = recalcular_propiedades(nodo->derecha, dimension_actualizada);
        }

        // Acumular puntos de los hijos
        puntos_actualizados.insert(puntos_actualizados.end(), puntos_izquierda.begin(), puntos_izquierda.end());
        puntos_actualizados.insert(puntos_actualizados.end(), puntos_derecha.begin(), puntos_derecha.end());

        // Recalcular el centroide usando los puntos acumulados
        Punto nuevo_centroide = calcular_centroide(puntos_actualizados);
        nodo->set_centroide(nuevo_centroide);

        // Recalcular el radio
        double nuevo_radio = 0.0;
        for (const auto& punto : puntos_actualizados) {
            double distancia = punto.distancia(nuevo_centroide);
            nuevo_radio = max(nuevo_radio, distancia);
        }
        nodo->set_radio(nuevo_radio);
        //la logica es esta, del de las hojas mas bajas se guarda los puntos de manera permanente a medidad que se sube
        //para que los nuevos centroides se calcule con el promedio de todos los puntos y no un promedio de promedio
        //porque esa vaina genera perdida de datos

        return puntos_actualizados; // Retornar los puntos acumulados
    }

    void deleteNodo(Nodo*& nodo, int dimension, double valor, int &eliminados) {
        if (!nodo) return;

        // Verificar si el nodo actual contiene el valor en la dimension indicada
        if (!nodo->izquierda && !nodo->derecha) { // Caso hoja
            const auto& dimensiones = nodo->get_centroide().get_dimensiones();
            if (dimension >= 0 && dimension < dimensiones.size() &&
                dimensiones[dimension].second &&
                fabs(dimensiones[dimension].first - valor) < 1e-6) { //tolerar problemas doubles
                delete nodo; // Liberar memoria del nodo
                nodo = nullptr; // Asegurarse de que el puntero se actualiza
                eliminados++;
            }
            return;
        }

        // Recorrer hacia los hijos para buscar el nodo
        if (nodo->izquierda) {
            deleteNodo(nodo->izquierda, dimension, valor,eliminados);
        }
        if (nodo->derecha) {
            deleteNodo(nodo->derecha, dimension, valor,eliminados);
        }

        // Reorganizar si uno de los hijos se elimino
        if (!nodo->izquierda && !nodo->derecha) {
            // Ambos hijos se han eliminado, entonces elimina este nodo
            delete nodo;
            nodo = nullptr;
        }
        else if (!nodo->izquierda) {
            // Solo queda el hijo derecho, este nodo puede ser reemplazado por el
            Nodo* hijo_derecho = nodo->derecha;
            delete nodo;
            nodo = hijo_derecho;
        }
        else if (!nodo->derecha) {
            // Solo queda el hijo izquierdo, este nodo puede ser reemplazado por el
            Nodo* hijo_izquierdo = nodo->izquierda;
            delete nodo;
            nodo = hijo_izquierdo;
        }
        
    }

public:
    BallTree(const vector<Punto>& puntos_iniciales) : puntos(puntos_iniciales) {
        head = construirRaiz(puntos);
    }

    Nodo* get_head() const {
        return head;
    }

    void imprimir_resultados(vector<Punto*>& puntos, int dimension) {
        if (puntos.size() > 0) {
            // Ordenar los resultados segun la dimension especificada
            sort(puntos.begin(), puntos.end(), [dimension](Punto* a, Punto* b) {
                auto& dimA = a->get_dimensiones()[dimension]; // Ordenar por dimension
                auto& dimB = b->get_dimensiones()[dimension];

                if (!dimA.second && !dimB.second) return false; // Ambos valores son NaN
                if (!dimA.second) return false;                 // Si A es NaN, B es menor
                if (!dimB.second) return true;                  // Si B es NaN, A es menor

                return dimA.first < dimB.first; // Ordenar por valor
                });


            // Imprimir encabezados
            for (auto& atributo : atributos) {
                cout << atributo << "\t";
            }
            cout << endl;

            // Imprimir los resultados
            for (auto* punto : puntos) {
                auto& dimensiones = punto->get_dimensiones();
                for (auto& dim : dimensiones) {
                    if (dim.second) {
                        cout << dim.first << "\t";
                    }
                    else {
                        cout << "NaN\t";
                    }
                }
                cout << endl;
            }
            cout << "\n\n\nTotal de encuentros: " << puntos.size() << endl;

        }
        else {
            cout << "\n\n>>>>>>>>>>>>>>> No se encontraron valores <<<<<<<<<<<<<<<\n\n";
        }

        cout << endl << endl << endl << "---------------------- NUEVA CONSULTA ---------------------" << endl;
    }
    vector<Punto*> find(int dimension, double valor, int consulta=1) {
        vector<Punto*> resultado; // Almacena referencias a los puntos originales para la funcion supdate
        auto start = chrono::steady_clock::now();

        find_recursivo(head, dimension, valor, resultado);

        auto end = chrono::steady_clock::now();

        if (consulta == 1) {
        cout << "Tiempo de busqueda en el arbol: "
            << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
            << " ns" << endl;

        }
        return resultado;
    }



    vector<Punto*> rangequery(int dimension, double valor, double rango) {
        vector<Punto*> resultado; // Vector para almacenar referencias a los puntos originales para el rupdate

        auto start = chrono::steady_clock::now();

        rangequery_recursivo(head, dimension, valor, rango, resultado); // Llama a la funcion recursiva actualizada

        auto end = chrono::steady_clock::now();

        cout << "Tiempo de busqueda en el arbol: "
            << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
            << " ns" << endl;

        // Imprimir los resultados
        imprimir_resultados(resultado,dimension);


        return resultado;
    }


    void updateselect(int dimension, double valor, double nuevo) {
        // Buscar los puntos que coinciden con la dimension y el valor, solo los que coinciden de manera exacta
        vector<Punto*> puntos_a_actualizar = find(dimension, valor,0);

        
        auto start = chrono::steady_clock::now();


        for (auto* punto : puntos_a_actualizar) {
            vector<Valor>& dimensiones = const_cast<vector<Valor>&>(punto->get_dimensiones());
            if (dimension >= 0 && dimension < dimensiones.size() && dimensiones[dimension].second) {
                dimensiones[dimension].first = nuevo; // Actualizar el valor
            }
        }
        auto end = chrono::steady_clock::now();

        cout << "Tiempo de busqueda en el arbol: "
            << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
            << " ns" << endl;
        recalcular_propiedades(head,dimension);
        cout << "\n\n--------------Se han actualizado " << puntos_a_actualizar.size() << " columnas ---------------------" << endl << endl << endl;
    }

    void updaterange(int dimension, double valor, double rango, double nuevo) {
        // Buscar los puntos dentro del rango especificado
        vector<Punto*> puntos_a_actualizar;
        rangequery_recursivo(head, dimension, valor, rango, puntos_a_actualizar);

        
        auto start = chrono::steady_clock::now();


        for (auto* punto : puntos_a_actualizar) {
            vector<Valor>& dimensiones = const_cast<vector<Valor>&>(punto->get_dimensiones());
            if (dimension >= 0 && dimension < dimensiones.size() && dimensiones[dimension].second) {
                dimensiones[dimension].first = nuevo; // Actualizar el valor
                //cout << "Actualizado punto en dimension " << dimension << " a " << nuevo << endl;
            }
        }
        auto end = chrono::steady_clock::now();

        cout << "Tiempo de busqueda en el arbol: "
            << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
            << " ns" << endl;
        recalcular_propiedades(head,dimension);
        cout << "\n\n--------------Se han actualizado" << puntos_a_actualizar.size() << " columnas---------------------" << endl << endl << endl;
    }

    void deleteValue(int dimension, double valor) {
        if (dimension < 0 || dimension >= static_cast<int>(atributos.size())) {
            cout << "Dimension invalida para eliminar." << endl;
            return;
        }

        auto start = chrono::steady_clock::now();
        int eliminados = 0;
        deleteNodo(head, dimension, valor, eliminados);

        auto end = chrono::steady_clock::now();

        cout << "Tiempo de eliminacion: "
            << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
            << " ns" << endl;

        cout << "\n\n--------------Se han eliminado " << eliminados << "columnas ---------------------" << endl << endl << endl;

    }

};


vector<Punto> leerCSV(const string& nombre_archivo) {
    vector<Punto> puntos; //lee el csv y crea los puntos a guardar
    ifstream archivo(nombre_archivo);
    string linea;

    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo: " << nombre_archivo << endl;
        return puntos;
    }

    getline(archivo, linea);

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string valor;
        vector<Valor> dimensiones;

        while (getline(ss, valor, ',')) {
            if (valor.empty()) {
                dimensiones.emplace_back(-1.0, false); 
            }
            else {
                dimensiones.emplace_back(stod(valor), true);
            }
        }

        
        puntos.emplace_back(dimensiones);
    }

    archivo.close();
    return puntos;
}

void printids() { //imprima la informacion de los id tabulada
    for (int i = 0; i < 9; i++) {
        if (i != 6) {
            cout << "(" << atributoscompletos[i] << "\t\t id: " << i << ")" << endl;
        }
        else {
            cout << "(" << atributoscompletos[i] << "\t id: " << i << ")" << endl;

        }
    }
    cout << endl;
}


void procesarConsultas(BallTree& ball_tree) { // bucle de consultas
    while (true) {
        cout << "\nEscribe 'select id valor' para realizar una consulta exacta,"
            <<endl<< " 'range dimension valor rango' para una consulta por rango,"
            << endl << " 'supdate dimension valor nuevo_valor' para actualizar un valor especifico,"
            << endl << " 'rupdate dimension valor rango nuevo_valor' para actualizar un rango, " << endl << " 'sdelete dimension valor' elimina todos los valores que coincidan con la dimensión,o 'exit' para salir." << endl;
        printids();
        cout << "----------------------------------------------------------" << endl << endl << endl;
 
        string comando;
        getline(cin, comando);
        if (comando == "exit") {
            cout << "Saliendo del programa." << endl;
            break;
        }

        stringstream ss(comando);
        string operacion;
        ss >> operacion;

        if (operacion == "select") {
            vector<vector<Punto*>> resultados;
            int id_guardado = -1; // Variable para almacenar el último ID procesado

            while (true) {
                int id;
                double valor;

                ss >> id >> valor;

                // Verificar si la entrada es válida
                if (ss.fail() || id < 0 || id >= static_cast<int>(atributos.size())) {
                    cout << "Formato invalido para 'select'. Usa 'select id valor'." << endl;
                    break;
                }

                cout << "\nProcesando consulta 'select' para id: " << id << ", valor: " << valor << endl;

                // Ejecutar la consulta y almacenar el resultado
                resultados.push_back(ball_tree.find(id, valor));
                id_guardado = id; // Guardar el último ID procesado

                // Verificar si hay más pares o se ha terminado
                string separador;
                ss >> separador;
                if (separador != "and") {
                    if (!separador.empty()) {
                        cout << "Advertencia: comando ignorado tras 'select id valor'. Se esperaba 'and' o el final." << endl;
                    }
                    break;
                }
            }

            if (resultados.size() == 1) {
                // Solo hay un vector, imprimir normalmente
                ball_tree.imprimir_resultados(resultados[0], id_guardado);
            }
            else {
                // Encontrar los puntos comunes entre todos los vectores
                unordered_map<Punto*, int> frecuencia;
                int total_vectores = resultados.size();

                // Contar la frecuencia de cada punto
                for (const auto& resultado : resultados) {
                    for (const auto& punto : resultado) {
                        frecuencia[punto]++;
                    }
                }

                // Crear una lista de puntos comunes
                vector<Punto*> puntos_comunes;
                for (const auto& it : frecuencia) {
                    Punto* punto = it.first;
                    int count = it.second;

                    if (count == total_vectores) { // El punto está en todos los vectores
                        puntos_comunes.push_back(punto);
                    }
                }

                // Imprimir los puntos comunes
                if (!puntos_comunes.empty()) {
                    cout << "\n>>>>>>>>>>>>>>> Puntos Comunes <<<<<<<<<<<<<<<\n\n";
                    ball_tree.imprimir_resultados(puntos_comunes, id_guardado);
                }
                else {
                    cout << "\n\n>>>>>>>>>>>>>>> No hay valores comunes <<<<<<<<<<<<<<<\n\n";
                }
            }

            
        }


        if (operacion == "sdelete") {
            int id;
            double valor;

            ss >> id >> valor;

            if (ss.fail() || id < 0 || id >= static_cast<int>(atributos.size())) {
                cout << "Formato invalido para 'select'. Usa 'select id valor'." << endl;
                continue;
            }

            cout << "\nProcesando consulta 'select delete' para id: " << id << ", valor: " << valor << endl;

            ball_tree.deleteValue(id, valor);

        }
        else if (operacion == "range") {
            int dimension;
            double valor, rango;

            ss >> dimension >> valor >> rango;

            if (ss.fail() || dimension < 0 || dimension >= static_cast<int>(atributos.size())) {
                cout << "Formato invalido para 'range'. Usa 'range dimension valor rango'." << endl;
                continue;
            }

            cout << "\nProcesando consulta 'range' para dimension: " << dimension
                << ", valor: " << valor << ", rango: " << rango << endl;

            ball_tree.rangequery(dimension, valor, rango);

        }
        else if (operacion == "supdate") {
            int dimension;
            double valor, nuevo_valor;

            ss >> dimension >> valor >> nuevo_valor;

            if (ss.fail() || dimension < 0 || dimension >= static_cast<int>(atributos.size())) {
                cout << "Formato invalido para 'supdate'. Usa 'supdate dimension valor nuevo_valor'." << endl;
                continue;
            }

            cout << "\nProcesando actualizacion 'supdate' para dimension: " << dimension
                << ", valor: " << valor << ", nuevo valor: " << nuevo_valor << endl;

            ball_tree.updateselect(dimension, valor, nuevo_valor);

        }
        else if (operacion == "rupdate") {
            int dimension;
            double valor, rango, nuevo_valor;

            ss >> dimension >> valor >> rango >> nuevo_valor;

            if (ss.fail() || dimension < 0 || dimension >= static_cast<int>(atributos.size())) {
                cout << "Formato invalido para 'rupdate'. Usa 'rupdate dimension valor rango nuevo_valor'." << endl;
                continue;
            }

            cout << "\nProcesando actualizacion 'rupdate' para dimension: " << dimension
                << ", valor: " << valor << ", rango: " << rango
                << ", nuevo valor: " << nuevo_valor << endl;

            ball_tree.updaterange(dimension, valor, rango, nuevo_valor);

        }
        else {
            cout << "Operacion no valida. Usa 'select', 'range', 'supdate', o 'rupdate'." << endl;
        }
    }
}

int main() {
    string nombre_archivo = "datos.csv";

    vector<Punto> puntos = leerCSV(nombre_archivo); //creacion del arbol

    auto start = chrono::steady_clock::now();
    BallTree ball_tree(puntos);
    auto end = chrono::steady_clock::now();

    cout << "Tiempo construccion del Arbol: "
        << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
        << " ns" << endl;

    procesarConsultas(ball_tree);
    
    return 0;
}
