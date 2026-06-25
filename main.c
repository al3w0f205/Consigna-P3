#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "funciones.h"

int main(void){
    setlocale(LC_ALL, "Spanish");
    Contaminacion limitesOMS = {400.0f, 40.0f, 25.0f, 15.0f};
    int maxZonasPermitidas = 100;
    Zona *zonas = NULL;
    int numZonas = 0;
    int opc = 0;

    numZonas = cargarDatos(&zonas, &limitesOMS, &maxZonasPermitidas);
    if (numZonas > 0){
        printf("Se cargaron %d zonas desde '%s'.\n", numZonas, ARCHIVO_DATOS);
    } else {
        printf("[INFO] No se encontraron datos previos.\n");
        printf("Inicializando datos de Quito, Ecuador...\n");
        inicializarDatosQuito(&zonas, &numZonas, limitesOMS, maxZonasPermitidas);
    }

    do {
        opc = menu();

        switch (opc){
        case 1:
            menuConfiguracion(zonas, numZonas, &limitesOMS, &maxZonasPermitidas);
            break;
        case 2:
            registrarZona(&zonas, &numZonas, maxZonasPermitidas, limitesOMS);
            break;
        case 3:
            registrarNiveles(zonas, numZonas, limitesOMS, maxZonasPermitidas);
            break;
        case 4:
            menuMonitoreoConsultas(zonas, numZonas, limitesOMS);
            break;
        case 5:
            exportarReporteTXT(zonas, numZonas, limitesOMS);
            break;
        case 6:
            guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
            printf("\n  Saliendo del sistema. Hasta luego!\n\n");
            break;
        }
        if (opc != 6) {
            printf("\n");
            pausarPantalla();
        }
    } while (opc != 6);

    free(zonas);
    return 0;
}
