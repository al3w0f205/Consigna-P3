#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DIAS        30
#define ARCHIVO_DATOS   "datos.dat"

typedef struct {
    float temperatura;
    float viento;
    float humedad;
} Clima;

typedef struct {
    float co2;
    float so2;
    float no2;
    float pm25;
} Contaminacion;

typedef struct {
    int id;
    char nombre[50];
    Contaminacion historial[MAX_DIAS];
    int head;  // Índice del registro más antiguo (Buffer circular)
    int count; // Cantidad de registros almacenados
    Contaminacion actual;
    Clima climaActual;
} Zona;

void limpiarBuffer(void);
void leerCadena(char *cadena, int n);
int leerEntero(const char *mensaje, int min, int max);
float leerFlotante(const char *mensaje, float min, float max);

void guardarDatos(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
int cargarDatos(Zona **zonas, int *capacidadZonas, Contaminacion *limitesOMS, int *maxZonasPermitidas);
int menu(void);
void registrarZona(Zona **zonas, int *numZonas, int *capacidadZonas, int maxZonasPermitidas, Contaminacion limitesOMS);
void registrarNiveles(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void monitoreoActual(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void prediccion24h(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void promediosHistoricos(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void alertasYRecomendaciones(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void establecerLimites(Contaminacion *limitesOMS, int *maxZonasPermitidas);
void cargarDatosPrueba(Zona **zonas, int *numZonas, int *capacidadZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void exportarReporteTXT(Zona *zonas, int numZonas);

#endif
