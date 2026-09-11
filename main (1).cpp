#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

using namespace std;

// Estructura para la tarea
struct Tarea {
    int id;
    int tiempo;
};

// Variables compartidas entre hilos
queue<Tarea> cola;
mutex m;
bool terminado = false;

// Hilo 1: Generador de tareas de trafico
void generador() {
    Tarea tareas[4] = {{1, 300}, {2, 150}, {3, 80}, {4, 200}};

    for (int i = 0; i < 4; i++) {
        this_thread::sleep_for(chrono::milliseconds(50));
        
        m.lock();
        cola.push(tareas[i]);
        cout << "[SIGET] Llego tarea " << tareas[i].id << " (" << tareas[i].tiempo << "ms)\n";
        m.unlock();
    }

    m.lock();
    terminado = true;
    m.unlock();
}

// Hilo 2: Planificador CPU (Round-Robin)
void planificador(int quantum) {
    while (true) {
        Tarea t;
        bool hay_tarea = false;

        m.lock();
        if (!cola.empty()) {
            t = cola.front();
            cola.pop();
            hay_tarea = true;
        }
        bool fin = terminado && cola.empty();
        m.unlock();

        if (fin && !hay_tarea) break;

        if (hay_tarea) {
            cout << "CPU procesando tarea " << t.id << " (Restante: " << t.tiempo << "ms)\n";

            if (t.tiempo > quantum) {
                this_thread::sleep_for(chrono::milliseconds(quantum));
                t.tiempo -= quantum;
                cout << "  -> Quantum de " << quantum << "ms fin. Tarea " << t.id << " regresa a la cola\n";

                m.lock();
                cola.push(t);
                m.unlock();
            } else {
                this_thread::sleep_for(chrono::milliseconds(t.tiempo));
                cout << "  -> Tarea " << t.id << " TERMINADA\n";
            }
        } else {
            this_thread::sleep_for(chrono::milliseconds(20));
        }
    }
}

int main() {
    int quantum = 100;

    cout << "=== SIMULADOR ROUND ROBIN (Quantum = " << quantum << "ms) ===\n\n";

    thread h1(generador);
    thread h2(planificador, quantum);

    h1.join();
    h2.join();

    cout << "\nFin de la simulacion.\n";
    return 0;
}
