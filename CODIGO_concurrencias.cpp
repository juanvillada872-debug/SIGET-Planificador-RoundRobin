#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

const int CAPACIDAD = 3;
int bufer[CAPACIDAD];
int in = 0;
int out = 0;
int cuenta = 0;

mutex mtx;
condition_variable no_lleno;
condition_variable no_vacio;

int sensores_activos = 2; // Controla el fin de los hilos productores

// HILOS 1 y 2: Sensores Norte y Sur (Productores)
void sensorTrafico(string nombre_sensor, int datos[], int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        unique_lock<mutex> lock(mtx);

        // Si el buffer esta lleno, cambia a LUZ ROJA y se detiene
        while (cuenta == CAPACIDAD) {
            cout << "[SEMÁFORO ROJO] Buffer lleno. " << nombre_sensor << " en espera...\n";
            no_lleno.wait(lock);
        }

        // LUZ VERDE: Hay espacio para enviar datos
        bufer[in] = datos[i];
        cout << "[SEMÁFORO VERDE - " << nombre_sensor << "] Alerta enviada ID: " << datos[i] 
             << " en pos " << in << "\n";
        in = (in + 1) % CAPACIDAD;
        cuenta++;

        no_vacio.notify_one();
        lock.unlock();

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    unique_lock<mutex> lock(mtx);
    sensores_activos--;
    no_vacio.notify_one();
}

// HILO 3: Modulo de Analisis (Consumidor)
void moduloAnalisis() {
    while (true) {
        unique_lock<mutex> lock(mtx);

        while (cuenta == 0 && sensores_activos > 0) {
            no_vacio.wait(lock);
        }

        if (cuenta == 0 && sensores_activos == 0) break;

        int dato = bufer[out];
        cout << "  -> [MÓDULO ANÁLISIS] Procesando alerta ID: " << dato 
             << " desde pos " << out << "\n";
        out = (out + 1) % CAPACIDAD;
        cuenta--;

        no_lleno.notify_one(); // Cambia el semaforo a VERDE para los sensores
        lock.unlock();

        this_thread::sleep_for(chrono::milliseconds(150));
    }
}

int main() {
    cout << "=== SIGET: CONTROL DE TRÁFICO Y SEMÁFOROS (3 HILOS) ===\n\n";

    int datos_norte[3] = {101, 102, 103};
    int datos_sur[3] = {201, 202, 203};

    // Creacion de los 3 hilos exigidos por la guia
    thread h1(sensorTrafico, "SENSOR NORTE", datos_norte, 3);
    thread h2(sensorTrafico, "SENSOR SUR", datos_sur, 3);
    thread h3(moduloAnalisis);

    h1.join();
    h2.join();
    h3.join();

    cout << "\nSimulacion finalizada. Trafico procesado sin interbloqueos.\n";
    return 0;
}