#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
    char fechaHora[20];
    Contaminacion niveles;
} RegistroHistorial;

typedef struct {
    int id;
    char nombre[50];
    RegistroHistorial historial[MAX_DIAS];
    int head;
    int count;
    Contaminacion actual;
    Clima climaActual;
} Zona;

void limpiarPantalla(void);
void pausarPantalla(void);
void mostrarZonasResumidas(Zona *zonas, int numZonas);
void limpiarBuffer(void);
void leerCadena(char *cadena, int n);
int validarIntRango(int a, int b);
float leerFlotante(const char *mensaje, float min, float max);

void guardarDatos(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
int cargarDatos(Zona **zonas, Contaminacion *limitesOMS, int *maxZonasPermitidas);
int menu(void);
void registrarZona(Zona **zonas, int *numZonas, int maxZonasPermitidas, Contaminacion limitesOMS);
void registrarNiveles(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void monitoreoActual(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void prediccion24h(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void promediosHistoricos(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void alertasYRecomendaciones(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void establecerLimites(Contaminacion *limitesOMS, int *maxZonasPermitidas);
void inicializarDatosQuito(Zona **zonas, int *numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void exportarReporteTXT(Zona *zonas, int numZonas, Contaminacion limitesOMS);
int imprimirAlertasGenerales(FILE *stream, Zona *zona, Contaminacion limitesOMS, Contaminacion pred);
int contieneSubcadenaIgnorandoMayusculas(const char *pajar, const char *aguja);
void buscarZonasPorNombre(Zona *zonas, int numZonas);
void filtrarZonasExcedidas(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void editarZona(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void menuConfiguracion(Zona *zonas, int numZonas, Contaminacion *limitesOMS, int *maxZonasPermitidas);
void menuMonitoreoConsultas(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void consultarZonaDetalle(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void registrarAlertaLog(const char *nombreZona, const char *contaminante, float valor, float limite);