#include <stdio.h>
#include "funciones.h"


int main(void){

    Zona zonas[MAX_ZONAS];
    int numZonas = 0;
    int opc = 0;

    numZonas = cargarDatos(zonas);
    if (numZonas > 0){
        printf("Se cargaron %d zonas desde '%s'.\n", numZonas, ARCHIVO_DATOS);
    } else {
        printf("[INFO] No se encontraron datos previos.\n");
        printf("Cargando zonas de prueba automaticamente...\n");
        cargarDatosPrueba(zonas, &numZonas);
        guardarDatos(zonas, numZonas);
    }

    do {
        opc = menu();

        switch (opc){
        case 1:
            establecerLimites();
            guardarDatos(zonas, numZonas);
            break;
        case 2:
            registrarZona(zonas, &numZonas);
            break;
        case 3:
            registrarNiveles(zonas, numZonas);
            break;
        case 4:
            monitoreoActual(zonas, numZonas);
            break;
        case 5:
            prediccion24h(zonas, numZonas);
            break;
        case 6:
            promediosHistoricos(zonas, numZonas);
            break;
        case 7:
            alertasYRecomendaciones(zonas, numZonas);
            break;
        case 8:
            exportarReporteTXT(zonas, numZonas);
            break;
        case 9:
            guardarDatos(zonas, numZonas);
            printf("\n  Saliendo del sistema. Hasta luego!\n\n");
            break;
        }
    } while (opc != 9);

    return 0;
}
