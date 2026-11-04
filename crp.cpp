#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <tuple>
#include <climits>
#include <random>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstdio>

using namespace std;

typedef pair<int, double> pi;

int n = 0, m = 0;
int h = 0, H = 0;

vector<vector<int>> bahia;
vector<vector<int>> bahiaInicial;

struct Solucion {
    vector<tuple<int, int, int>> movimientos;
    int costo = 0;
};

void imprimirBahia() {
    for (size_t i  = 0; i < bahia.size(); i++) {
        for (size_t  j = 0; j < bahia[i].size(); j++) {
            cout << bahia[i][j] << " ";
        }
        cout << endl;
    }
}

void leerDatos(string nombreArchivo) {

    int W, k;

    sscanf(nombreArchivo.c_str(), "data%d-%d-%d.dat", &h, &W, &k);

    ifstream archivo(nombreArchivo);

    archivo >> m >> n;

    H = h + 2;

    bahia.assign(H, vector<int>(m, 0));

    for (int i = 0; i < m; i++) {

        int hi;
        archivo >> hi;

        for (int j = 0; j < hi; j++) {
            archivo >> bahia[H - 1 - j][i];
        }
    }

    archivo.close();
}

int obHMaxima(int p1) {

    int h1 = 0;

    while (h1 < H) {
        if (bahia[h1][p1] != 0) break;
        h1++;
    }

    if (h1 == 0) return -1;

    return h1 - 1;
}

bool esValido(int p1, int p2) {

    return p1 >= 0 && p1 < m &&
           p2 >= 0 && p2 < m &&
           obHMaxima(p2) != -1 &&
           p1 != p2 &&
           obHMaxima(p1) < H - 1;
}

bool puedoMover(int p1, int objetivo) {

    if (p1 < 0 || p1 >= m) return false;

    int fila = obHMaxima(p1) + 1;

    if (fila >= H) return false;

    return bahia[fila][p1] == objetivo;
}

bool estaEnLaPila(int p1, int objetivo) {

    for (int j = 0; j < H; j++) {
        if (bahia[j][p1] == objetivo) return true;
    }

    return false;
}

void mover(int p1, int p2) {

    int hP1 = obHMaxima(p1) + 1;
    int hP2 = obHMaxima(p2);

    bahia[hP2][p2] = bahia[hP1][p1];
    bahia[hP1][p1] = 0;
}

void extraer(int contenedorActual, int p1) {
    int hP1 = obHMaxima(p1) + 1;
    bahia[hP1][p1] = 0;
}

vector<int> construirRCL(const vector<pair<int, double>>& candidatos,double alpha) {

    vector<int> RCL;

    if (candidatos.empty()) return RCL;

    auto [minIt, maxIt] = minmax_element(candidatos.begin(),candidatos.end(),[](const auto& a, const auto& b) {return a.second < b.second;}
    );

    double limite = minIt->second +alpha * (maxIt->second - minIt->second);

    for (const auto& [pila, costo] : candidatos) {
        if (costo <= limite) {
            RCL.push_back(pila);
        }
    }

    return RCL;
}

// Bloqueos futuros
double greedy(int container, int pilaDestino) {
    double count = 0;
    for (int altura = 0; altura < H; altura++) {
        if (container > bahia[altura][pilaDestino] && bahia[altura][pilaDestino] != 0) {
            count++;
        }
    }
    return count;
}

void construirSolucion(Solucion& temporal,double alpha,int objetivoInicial = 1) {

    for (int contenedorActual = objetivoInicial;contenedorActual <= n;contenedorActual++) {
        int pilaOb = -1;
        for (int pila = 0; pila < m; pila++) {

            if (estaEnLaPila(pila, contenedorActual)) {
                pilaOb = pila;
                break;
            }
        }
        if (pilaOb == -1) {
            temporal.costo = INT_MAX;
            return;
        }
        while (!puedoMover(pilaOb, contenedorActual)) {
            int fila = obHMaxima(pilaOb) + 1;
            int container = bahia[fila][pilaOb];
            vector<pi> candidatos;
            for (int pilaCandidata = 0;
                 pilaCandidata < m;
                 pilaCandidata++) {
                if (esValido(pilaOb, pilaCandidata)) {
                    double val = greedy(container,pilaCandidata);
                    candidatos.push_back({
                        pilaCandidata, val
                    });
                }
            }

            vector<int> RCL = construirRCL(candidatos, alpha);

            if (RCL.empty()) {
                temporal.costo = INT_MAX;
                return;
            }
            int indice = rand() % RCL.size();
            int pilaDestino = RCL[indice];
            mover(pilaOb, pilaDestino);
            temporal.movimientos.push_back({container, pilaOb, pilaDestino});
            temporal.costo++;
        }
        extraer(contenedorActual, pilaOb);
        temporal.movimientos.push_back({contenedorActual, pilaOb, -1});
    }
}

pair<vector<tuple<int,int,int>>, int> cambiarMovimiento(int i,int pila,vector<tuple<int,int,int>> movimientos) {
    auto [container, origen, destino] = movimientos[i];

    while (movimientos.size() > static_cast<size_t>(i)) {
        movimientos.pop_back();
    }

    vector<tuple<int,int,int>> ans = movimientos;

    int costo = 0;

    bahia = bahiaInicial;
    for (const auto& [c, p1, p2] : movimientos) {
        if (p2 == -1) {
            extraer(c, p1);
        }
        else {
            mover(p1, p2);
            costo++;
        }
    }
   
    if (!esValido(origen, pila)) {
        return {{}, INT_MAX};
    }

    mover(origen, pila);

    ans.push_back({container, origen, pila});
    costo++;

    return make_pair(ans, costo);
}

Solucion mejorarSolucion(Solucion solucion) {

    bool mejora;
    do {
        mejora = false;
        for (size_t i = 0; i < solucion.movimientos.size(); i++) {
            auto [container, origen, destino] = solucion.movimientos[i];

            if (destino == -1) continue;

            for (int pila = 0; pila < m; pila++) {
                if (pila == destino || pila == origen) continue;
                auto temporal = cambiarMovimiento(i,pila,solucion.movimientos);
                if (temporal.second == INT_MAX) continue;
                Solucion temporalS;
                temporalS.movimientos = temporal.first;
                temporalS.costo = temporal.second;
                int objetivoInicial = 1;
                for (const auto& [c, p1, p2] :
                     temporalS.movimientos) {
                    if (p2 == -1) {
                        objetivoInicial++;
                    }
                }
                construirSolucion(temporalS,0.6,objetivoInicial);
                if (temporalS.costo < solucion.costo) {
                    solucion = temporalS;
                    mejora = true;
                    break;
                }
            }
            if (mejora) break;
        }

    } while (mejora);

    return solucion;
}

Solucion GRASP(int iteracionMaxima) {

    Solucion mejor;
    mejor.costo = INT_MAX;

    double alpha = 0.6;

    bahiaInicial = bahia;

    for (int i = 0; i < iteracionMaxima; i++) {

        bahia = bahiaInicial;

        Solucion temporal;

        construirSolucion(temporal, alpha);

        if (temporal.costo == INT_MAX) continue;

        Solucion s = mejorarSolucion(temporal);

        if (s.costo < mejor.costo) {
            mejor = s;
        }
    }

    return mejor;
}

void imprimirSolucion(const Solucion& mejor) {

    cout << "Mejor solucion encontrada:" << endl;

    for (const auto& [container, origen, destino] :
         mejor.movimientos) {

        if (destino == -1) {
            cout << "Extraer contenedor "<< container<< " de la pila "<< origen + 1<< endl;
        }
        else {
            cout << "Reubicar contenedor "<< container<< ": Pila "<< origen + 1<< " -> Pila "<< destino + 1 << endl;
        }
    }
    cout << "Total de reubicaciones: "<< mejor.costo<< endl;
}

void solve(string s) {
    leerDatos(s);
    imprimirBahia();
    Solucion mejor = GRASP(50);
    imprimirSolucion(mejor);
}

int main(int argc, char* argv[]) {
    srand(time(nullptr));
    solve(argv[1]);
    return 0;
}
