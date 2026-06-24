
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
    int head;
    int count;
    Contaminacion actual;
    Clima climaActual;
} Zona;

void limpiarBuffer(void);
void leerCadena(char *cadena, int n);
int validarIntRango(int a, int b);
float leerFlotante(const char *mensaje, float min, float max);

void guardarDatos(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
int cargarDatos(Zona zonas[], Contaminacion *limitesOMS, int *maxZonasPermitidas);
int menu(void);
void registrarZona(Zona zonas[], int *numZonas, int maxZonasPermitidas, Contaminacion limitesOMS);
void registrarNiveles(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void monitoreoActual(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void prediccion24h(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void promediosHistoricos(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void alertasYRecomendaciones(Zona *zonas, int numZonas, Contaminacion limitesOMS);
void establecerLimites(Contaminacion *limitesOMS, int *maxZonasPermitidas);
void inicializarDatosQuito(Zona zonas[], int *numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);
void exportarReporteTXT(Zona *zonas, int numZonas, Contaminacion limitesOMS);

/**
 * @brief Compara si una cadena de texto contiene a otra subcadena ignorando mayúsculas y minúsculas.
 * @param pajar La cadena principal en la que buscar.
 * @param aguja La subcadena a buscar.
 * @return int 1 si la subcadena está contenida, 0 en caso contrario.
 */
int contieneSubcadenaIgnorandoMayusculas(const char *pajar, const char *aguja);

/**
 * @brief Permite realizar búsquedas interactivas de zonas por nombre (coincidencia parcial).
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas actualmente registradas.
 */
void buscarZonasPorNombre(Zona *zonas, int numZonas);

/**
 * @brief Filtra y muestra aquellas zonas que excedan los límites de la OMS.
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas actualmente registradas.
 * @param limitesOMS Límites máximos permitidos configurados en el sistema.
 */
void filtrarZonasExcedidas(Zona *zonas, int numZonas, Contaminacion limitesOMS);

/**
 * @brief Permite seleccionar una zona por ID o búsqueda de nombre y editar su nombre de forma persistente.
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas actualmente registradas.
 * @param limitesOMS Límites máximos permitidos (para pasar al guardar datos).
 * @param maxZonasPermitidas Límite máximo de zonas (para pasar al guardar datos).
 */
void editarZona(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas);

/**
 * @brief Muestra el submenú de configuración del sistema (límites, tope de zonas, edición de nombres).
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas registradas.
 * @param limitesOMS Puntero a la estructura de límites OMS a configurar.
 * @param maxZonasPermitidas Puntero al tope máximo de zonas a configurar.
 */
void menuConfiguracion(Zona *zonas, int numZonas, Contaminacion *limitesOMS, int *maxZonasPermitidas);

/**
 * @brief Muestra el submenú de monitoreo y consultas (resumen, detalle, búsqueda, filtrado).
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas registradas.
 * @param limitesOMS Límites máximos permitidos configurados en el sistema.
 */
void menuMonitoreoConsultas(Zona *zonas, int numZonas, Contaminacion limitesOMS);

/**
 * @brief Imprime un reporte consolidado de una zona en específico (actual, clima, promedio 30 días, predicción 24h, alertas y sugerencias).
 * @param zonas Arreglo de zonas del sistema.
 * @param numZonas Cantidad de zonas registradas.
 * @param limitesOMS Límites máximos permitidos configurados en el sistema.
 */
void consultarZonaDetalle(Zona *zonas, int numZonas, Contaminacion limitesOMS);