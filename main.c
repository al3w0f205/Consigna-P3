#include <stdio.h>
#include <stdlib.h>
#include "funciones.h"

int main(void){
    Contaminacion limitesOMS = {400.0f, 40.0f, 25.0f, 15.0f};
    int maxZonasPermitidas = 100;
    Zona zonas[100];
    int numZonas = 0;
    int opc = 0;

    numZonas = cargarDatos(zonas, &limitesOMS, &maxZonasPermitidas);
    if (numZonas > 0){
        printf("Se cargaron %d zonas desde '%s'.\n", numZonas, ARCHIVO_DATOS);
    } else {
        printf("[INFO] No se encontraron datos previos.\n");
        printf("Cargando zonas de prueba automaticamente...\n");
        cargarDatosPrueba(zonas, &numZonas, limitesOMS, maxZonasPermitidas);
    }

    do {
        opc = menu();

        switch (opc){
        case 1:
            establecerLimites(&limitesOMS, &maxZonasPermitidas);
            guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
            break;
        case 2:
            registrarZona(zonas, &numZonas, maxZonasPermitidas, limitesOMS);
            break;
        case 3:
            registrarNiveles(zonas, numZonas, limitesOMS, maxZonasPermitidas);
            break;
        case 4:
            monitoreoActual(zonas, numZonas, limitesOMS);
            break;
        case 5:
            prediccion24h(zonas, numZonas, limitesOMS);
            break;
        case 6:
            promediosHistoricos(zonas, numZonas, limitesOMS);
            break;
        case 7:
            alertasYRecomendaciones(zonas, numZonas, limitesOMS);
            break;
        case 8:
            exportarReporteTXT(zonas, numZonas);
            break;
        case 9:
            guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
            printf("\n  Saliendo del sistema. Hasta luego!\n\n");
            break;
        }
    } while (opc != 9);
    return 0;
}
