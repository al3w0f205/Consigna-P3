#include <stdio.h>
#include <string.h>

#define MAX_ZONAS       10
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
    Contaminacion actual;
    Clima climaActual;
} Zona;

void limpiarBuffer(void);
void leerCadena(char *cadena, int n);
int leerEntero(const char *mensaje, int min, int max);
float leerFlotante(const char *mensaje, float min, float max);
void guardarDatos(Zona *zonas, int numZonas);
int cargarDatos(Zona *zonas);
int menu(void);
void registrarZona(Zona *zonas, int *numZonas);
void registrarNiveles(Zona *zonas, int numZonas);
void monitoreoActual(Zona *zonas, int numZonas);
void prediccion24h(Zona *zonas, int numZonas);
void promediosHistoricos(Zona *zonas, int numZonas);
void alertasYRecomendaciones(Zona *zonas, int numZonas);
void establecerLimites(void);
void cargarDatosPrueba(Zona *zonas, int *numZonas);
void exportarReporteTXT(Zona *zonas, int numZonas);
