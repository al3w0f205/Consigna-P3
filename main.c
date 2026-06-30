#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "funciones.h"

int main(void){
    setlocale(LC_CTYPE, "Spanish");
    // Limites de calidad del aire para Ecuador (NECA - TULAS Libro VI Anexo 4)
    Contaminacion limitesOMS = {500.0f, 125.0f, 100.0f, 50.0f};
    int maxZonasPermitidas = 100;
    Zona *zonas = NULL;
    int numZonas = 0;
    int opc = 0;

    int carga = cargarDatos(&zonas, &limitesOMS, &maxZonasPermitidas);
    if (carga >= 0){
        numZonas = carga;
        printf("Se cargaron %d zonas desde '%s'.\n", numZonas, ARCHIVO_DATOS);
    } else if (carga == -1) {
        printf("\n[ERROR CRITICO] El archivo '%s' existe pero esta danado o corrupto.\n", ARCHIVO_DATOS);
        printf("¿Desea sobrescribirlo con los datos por defecto? (1: Si, 2: Salir del programa)\n");
        printf(">> ");
        int opcCorr = validarIntRango(1, 2);
        if (opcCorr == 2) {
            printf("Saliendo del programa para evitar perder el archivo danado.\n");
            free(zonas);
            return 1;
        }
        printf("Inicializando datos de Quito, Ecuador...\n");
        inicializarDatosQuito(&zonas, &numZonas, limitesOMS, maxZonasPermitidas);
    } else {
        // carga == -2 (el archivo no existe)
        printf("[INFO] No se encontraron datos previos.\n");
        printf("Inicializando datos de Quito, Ecuador...\n");
        inicializarDatosQuito(&zonas, &numZonas, limitesOMS, maxZonasPermitidas);
    }

    do {
        opc = menu();

        switch (opc){
        case 1:
            menuConfiguracion(&zonas, &numZonas, &limitesOMS, &maxZonasPermitidas);
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
        if (opc != 6 && opc != 1 && opc != 4) {
            printf("\n");
            pausarPantalla();
        }
    } while (opc != 6);

    free(zonas);
    return 0;
}
