#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funciones.h"

void limpiarBuffer(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void leerCadena(char *cadena, int n){
    int len;
    if (fgets(cadena, n, stdin) == NULL){
        cadena[0] = '\0';
        return;
    }
    len = (int)strlen(cadena) - 1;
    if (len >= 0 && cadena[len] == '\n'){
        cadena[len] = '\0';
    }
}

int validarIntRango(int a, int b){
    int n = 0, aux = 0;
    do {
        aux = scanf("%d", &n);
        limpiarBuffer();
        if (aux == EOF){
            printf("\n  [!] Entrada cerrada inesperadamente. Saliendo...\n");
            exit(1);
        }
        if (aux != 1 || n < a || n > b){
            printf("  [!] Error: valor ingresado invalido. Ingrese un entero entre %d y %d.\n", a, b);
            printf("  Vuelvalo a ingresar >> ");
        }
    } while (aux != 1 || n < a || n > b);
    return n;
}

float leerFlotante(const char *mensaje, float min, float max){
    float valor;
    int res;
    while (1){
        printf("%s", mensaje);
        res = scanf("%f", &valor);
        if (res == EOF){
            printf("\n  [!] Entrada cerrada inesperadamente. Saliendo...\n");
            exit(1);
        }
        if (res != 1){
            printf("  [!] Entrada invalida. Ingrese un numero decimal (use punto, no coma).\n");
            limpiarBuffer();
            continue;
        }
        limpiarBuffer();
        if (valor < min || valor > max){
            printf("  [!] Valor fuera de rango [%.2f - %.2f].\n", min, max);
            continue;
        }
        return valor;
    }
}

void guardarDatos(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
    FILE *archivo = fopen(ARCHIVO_DATOS, "wb");
    if (!archivo){
        printf("  [ERROR] No se pudo abrir '%s' para escritura.\n", ARCHIVO_DATOS);
        return;
    }
    fwrite(&limitesOMS, sizeof(Contaminacion), 1, archivo);
    fwrite(&maxZonasPermitidas, sizeof(int), 1, archivo);
    fwrite(&numZonas, sizeof(int), 1, archivo);
    
    if (numZonas > 0 && zonas != NULL) {
        fwrite(zonas, sizeof(Zona), numZonas, archivo);
    }
    fclose(archivo);
    printf("  Datos guardados exitosamente en '%s'.\n", ARCHIVO_DATOS);
}

int cargarDatos(Zona zonas[], Contaminacion *limitesOMS, int *maxZonasPermitidas){
    int numZonas = 0;
    Contaminacion limBackup = *limitesOMS;
    int maxBackup = *maxZonasPermitidas;
    
    FILE *archivo = fopen(ARCHIVO_DATOS, "rb");
    if (!archivo) return 0;
    
    if (fread(limitesOMS, sizeof(Contaminacion), 1, archivo) != 1 ||
        fread(maxZonasPermitidas, sizeof(int), 1, archivo) != 1 ||
        fread(&numZonas, sizeof(int), 1, archivo) != 1) {
        
        *limitesOMS = limBackup;
        *maxZonasPermitidas = maxBackup;
        fclose(archivo); 
        return 0; 
    }
    
    if (numZonas > 0){
        if (fread(zonas, sizeof(Zona), (size_t)numZonas, archivo) != (size_t)numZonas){
            *limitesOMS = limBackup;
            *maxZonasPermitidas = maxBackup;
            fclose(archivo);
            return 0;
        }
    }
    fclose(archivo);
    return numZonas;
}

int menu(void){
    system("cls");
    printf("\n----------------------------------------------------------\n");
    printf("              CONTROL DE CALIDAD DEL AIRE\n");
    printf("----------------------------------------------------------\n");
    printf("   1. Configuracion del Sistema (Limites y Zonas)\n");
    printf("   2. Registrar Nueva Zona\n");
    printf("   3. Actualizar Niveles y Clima de Hoy\n");
    printf("   4. Monitoreo y Consultas (Buscador, Prediccion y Alertas)\n");
    printf("   5. Exportar Reporte General (Archivo TXT)\n");
    printf("   6. Salir del Sistema\n");
    printf("----------------------------------------------------------\n");
    printf(">> ");
    return validarIntRango(1, 6);
}

int verificarZonas(int n){
    if (n == 0){
        printf("\n  [!] No hay zonas registradas. Use la opcion 2 primero.\n");
        return 0;
    }
    return 1;
}

const char *estado(float val, float lim){
    if (val > lim)
        return "EXCEDIDO";
    return "Normal";
}

void imprimirFila(const char *nombre, float valor, float limite){
    printf("  %-34s %10.2f   %s\n", nombre, valor, estado(valor, limite));
}

void imprimirFilaPred(const char *nombre, float actual, float pred, float limite){
    const char *est;
    if (pred > limite)
        est = "ALERTA";
    else
        est = "Normal";
    printf("  %-34s %10.2f   %10.2f   %s\n", nombre, actual, pred, est);
}

void imprimirTabla(Contaminacion c, Contaminacion limitesOMS){
    imprimirFila("Dioxido de Carbono (CO2)",  c.co2,  limitesOMS.co2);
    imprimirFila("Dioxido de Azufre (SO2)",   c.so2,  limitesOMS.so2);
    imprimirFila("Dioxido de Nitrogeno (NO2)", c.no2,  limitesOMS.no2);
    imprimirFila("Particulas Finas (PM2.5)",  c.pm25, limitesOMS.pm25);
}

void imprimirTablaPrediccion(Contaminacion actual, Contaminacion pred, Contaminacion limitesOMS){
    imprimirFilaPred("Dioxido de Carbono (CO2)",  actual.co2,  pred.co2,  limitesOMS.co2);
    imprimirFilaPred("Dioxido de Azufre (SO2)",   actual.so2,  pred.so2,  limitesOMS.so2);
    imprimirFilaPred("Dioxido de Nitrogeno (NO2)", actual.no2,  pred.no2,  limitesOMS.no2);
    imprimirFilaPred("Particulas Finas (PM2.5)",  actual.pm25, pred.pm25, limitesOMS.pm25);
}

Contaminacion calcPromedios(Zona *z){
    Contaminacion prom = {0, 0, 0, 0};
    int i;
    if (z->count == 0) return prom;
    
    for (i = 0; i < z->count; i++){
        int pos = (z->head + i) % MAX_DIAS;
        prom.co2  += z->historial[pos].co2;
        prom.so2  += z->historial[pos].so2;
        prom.no2  += z->historial[pos].no2;
        prom.pm25 += z->historial[pos].pm25;
    }
    prom.co2 /= z->count;  prom.so2 /= z->count;
    prom.no2 /= z->count;  prom.pm25 /= z->count;
    return prom;
}

float clamp(float v, float min, float max){
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

float factorClimatico(Clima *c){
    return clamp(1.0f - (c->viento / 200.0f), 0.5f, 1.5f) *
           clamp(1.0f + (c->temperatura - 20.0f) * 0.01f, 0.8f, 1.2f) *
           clamp(1.0f + (c->humedad - 50.0f) * 0.005f, 0.85f, 1.15f);
}

Contaminacion calcPrediccion(Zona *z, float fc, Contaminacion prom){
    Contaminacion r;
    r.co2  = 0.6f * z->actual.co2  * fc + 0.4f * prom.co2;
    r.so2  = 0.6f * z->actual.so2  * fc + 0.4f * prom.so2;
    r.no2  = 0.6f * z->actual.no2  * fc + 0.4f * prom.no2;
    r.pm25 = 0.6f * z->actual.pm25 * fc + 0.4f * prom.pm25;
    return r;
}

void genHistorial(Zona *z, Contaminacion base){
    int d;
    float f;
    
    z->head = 0;
    z->count = MAX_DIAS;
    
    for (d = 0; d < MAX_DIAS; d++){
        f = 1.0f + (float)((d * 7 + z->id * 13) % 21 - 10) / 100.0f;
        z->historial[d].co2  = base.co2  * f;
        z->historial[d].so2  = base.so2  * f;
        z->historial[d].no2  = base.no2  * f;
        z->historial[d].pm25 = base.pm25 * f;
    }
}

void encabezadoSeccion(const char *titulo){
    printf("\n  --- %s ---\n\n", titulo);
}

void encabezadoZona(int id, const char *nombre){
    printf("  >> Zona %d: %s\n\n", id, nombre);
}

int verificarYMostrarAlerta(float val, float lim, const char *param, const char *rec, int prev){
    if (val > lim){
        if (prev)
            printf("  - MANANA: Nivel de %s estara alto (%.2f / Limite: %.0f)\n", param, val, lim);
        else
            printf("  - HOY: Nivel de %s esta alto (%.2f / Limite: %.0f)\n", param, val, lim);
        
        printf("    Sugerencia: %s\n\n", rec);
        return 1;
    }
    return 0;
}

void leerContaminacion(Contaminacion *c){
    c->co2  = leerFlotante("  Nivel de Dioxido de Carbono (CO2) [0-2000]: ", 0, 2000);
    c->so2  = leerFlotante("  Nivel de Dioxido de Azufre  (SO2) [0-500]:  ", 0, 500);
    c->no2  = leerFlotante("  Nivel de Dioxido de Nitrogeno  (NO2) [0-500]:  ", 0, 500);
    c->pm25 = leerFlotante("  Nivel de Particulas Finas (PM2.5) [0-500]:  ", 0, 500);
}

void leerClima(Clima *c){
    c->temperatura = leerFlotante("  Temperatura en grados (0 a 50):           ", 0, 50);
    c->viento      = leerFlotante("  Velocidad del viento en km/h (0 a 200):   ", 0, 200);
    c->humedad     = leerFlotante("  Porcentaje de humedad (0 a 100):          ", 0, 100);
}

void registrarZona(Zona zonas[], int *numZonas, int maxZonasPermitidas, Contaminacion limitesOMS){
    Zona *n;
    Contaminacion base;

    if (*numZonas >= maxZonasPermitidas) {
        printf("\n  [!] Se ha alcanzado el tope maximo de zonas configurado (%d).\n", maxZonasPermitidas);
        return;
    }

    n = &zonas[*numZonas];
    n->id = *numZonas + 1;
    n->head = 0;
    n->count = 0;

    printf("\n  --- REGISTRAR NUEVA ZONA ---\n");
    do {
        printf("  Nombre de la zona: ");
        leerCadena(n->nombre, 50);
        if (n->nombre[0] == '\0'){
            printf("  [!] El nombre no puede estar vacio. Intentelo de nuevo.\n");
        }
    } while (n->nombre[0] == '\0');

    printf("\n  --- NIVELES ACTUALES DE CONTAMINACION ---\n");
    leerContaminacion(&n->actual);

    printf("\n  --- CONDICIONES CLIMATICAS DE HOY ---\n");
    leerClima(&n->climaActual);

    printf("\n  --- GENERAR HISTORIAL PASADO ---\n");
    printf("  Ingrese niveles de ejemplo para simular los 30 dias anteriores:\n");
    leerContaminacion(&base);

    genHistorial(n, base);

    if (n->count > 0) {
        int pos = (n->head + n->count - 1) % MAX_DIAS;
        n->historial[pos] = n->actual;
    }

    (*numZonas)++;
    guardarDatos(zonas, *numZonas, limitesOMS, maxZonasPermitidas);
    printf("  Zona '%s' registrada exitosamente con ID %d.\n", n->nombre, n->id);
}

void registrarNiveles(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
    int i, idx;

    if (!verificarZonas(numZonas)) return;

    printf("\n  --- ACTUALIZAR NIVELES DE CONTAMINACION ---\n\n");
    printf("  Zonas disponibles:\n");
    for (i = 0; i < numZonas; i++)
        printf("    %d. %s\n", zonas[i].id, zonas[i].nombre);
    printf("\n");

    printf("  Seleccione zona por ID: ");
    idx = validarIntRango(1, numZonas) - 1;

    printf("\n  Ingrese los nuevos niveles de contaminacion para '%s':\n", zonas[idx].nombre);
    leerContaminacion(&zonas[idx].actual);

    printf("\n  --- NUEVO ESTADO CLIMATICO ---\n");
    leerClima(&zonas[idx].climaActual);

    int pos = (zonas[idx].head + zonas[idx].count) % MAX_DIAS;
    zonas[idx].historial[pos] = zonas[idx].actual;
    if (zonas[idx].count < MAX_DIAS) zonas[idx].count++;
    else zonas[idx].head = (zonas[idx].head + 1) % MAX_DIAS;

    guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
    printf("  Niveles de la zona '%s' actualizados.\n", zonas[idx].nombre);
}

void monitoreoActual(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i;
    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("-----ESTADO ACTUAL DEL AIRE-----");
    printf("  Limites configurados: CO2=%.0f | SO2=%.0f | NO2=%.0f | PM2.5=%.0f\n\n",
           limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25);

    for (i = 0; i < numZonas; i++){
        encabezadoZona(zonas[i].id, zonas[i].nombre);
        printf("  %-34s %10s   %s\n"
               "  ---------------------------------- ----------   ----------\n", "Contaminante", "Nivel", "Estado");
        imprimirTabla(zonas[i].actual, limitesOMS);
        printf("  Clima actual: %.1f grados | Viento: %.1f km/h | Humedad: %.1f%%\n\n",
               zonas[i].climaActual.temperatura,
               zonas[i].climaActual.viento,
               zonas[i].climaActual.humedad);
    }
}

void prediccion24h(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i;
    float fc;
    Contaminacion prom, pred;

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("               PREDICCION DEL AIRE PARA MANANA               ");
    printf("  El calculo usa los niveles actuales, el historial de 30 dias\n"
           "  y toma en cuenta como el viento o la temperatura afectan al aire.\n\n");

    for (i = 0; i < numZonas; i++){
        prom = calcPromedios(&zonas[i]);
        fc = factorClimatico(&zonas[i].climaActual);
        pred = calcPrediccion(&zonas[i], fc, prom);

        encabezadoZona(zonas[i].id, zonas[i].nombre);
        printf("  Impacto del clima en la prediccion: %.3f\n\n", fc);

        printf("  %-34s %10s   %10s   %s\n"
               "  ---------------------------------- ----------   ----------   -------------\n", "Contaminante", "Nivel Hoy", "Nivel Manana", "Estado");
        imprimirTablaPrediccion(zonas[i].actual, pred, limitesOMS);
        printf("\n");
    }
}

void promediosHistoricos(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i;
    Contaminacion prom;

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("               HISTORIAL DE LOS ULTIMOS 30 DIAS               ");

    for (i = 0; i < numZonas; i++){
        prom = calcPromedios(&zonas[i]);
        encabezadoZona(zonas[i].id, zonas[i].nombre);
        printf("  %-34s %10s   %s\n"
               "  ---------------------------------- ----------   ----------\n", "Contaminante", "Promedio", "Estado");
        imprimirTabla(prom, limitesOMS);
        printf("\n");
    }
}

void alertasYRecomendaciones(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i, alerta;
    float fc;
    Contaminacion prom, pred;

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("---ALERTAS Y RECOMENDACIONES---");

    for (i = 0; i < numZonas; i++){
        alerta = 0;

        encabezadoZona(zonas[i].id, zonas[i].nombre);

        alerta += verificarYMostrarAlerta(zonas[i].actual.co2, limitesOMS.co2, "Dioxido de Carbono (CO2)", "Reducir uso de combustibles fosiles y fomentar movilidad no motorizada (bicicletas).", 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.so2, limitesOMS.so2, "Dioxido de Azufre (SO2)", "Inspeccionar industrias y proteger poblaciones vulnerables en refugios.", 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.no2, limitesOMS.no2, "Dioxido de Nitrogeno (NO2)", "Restringir trafico vehicular y promover uso de transporte publico (Salud Publica).", 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.pm25, limitesOMS.pm25, "Particulas Finas (PM2.5)", "Evitar actividades al aire libre y promover limpieza comunitaria de calles.", 0);

        prom = calcPromedios(&zonas[i]);
        fc = factorClimatico(&zonas[i].climaActual);
        pred = calcPrediccion(&zonas[i], fc, prom);

        alerta += verificarYMostrarAlerta(pred.co2, limitesOMS.co2, "Dioxido de Carbono (CO2)", "Preparar restricciones de trafico y activar protocolo de emergencia ambiental.", 1);
        alerta += verificarYMostrarAlerta(pred.so2, limitesOMS.so2, "Dioxido de Azufre (SO2)", "Notificar a industrias para reducir emisiones y preparar alerta sanitaria.", 1);
        alerta += verificarYMostrarAlerta(pred.no2, limitesOMS.no2, "Dioxido de Nitrogeno (NO2)", "Planificar desvios de trafico e incrementar frecuencia de transporte publico.", 1);
        alerta += verificarYMostrarAlerta(pred.pm25, limitesOMS.pm25, "Particulas Finas (PM2.5)", "Emitir aviso para proteger la salud comunitaria al aire libre.", 1);

        if (alerta == 0){
            printf("  Todo en orden: La calidad del aire es buena hoy y manana.\n"
                   "  No hay alertas ni recomendaciones.\n");
        }
        printf("\n");
    }
}

void mostrarLimites(Contaminacion limitesOMS, int maxZonasPermitidas){
    printf("    Dioxido de Carbono (CO2)  : %.2f maximo\n"
           "    Dioxido de Azufre (SO2)   : %.2f maximo\n"
           "    Dioxido de Nitrogeno (NO2): %.2f maximo\n"
           "    Particulas Finas (PM2.5)  : %.2f maximo\n"
           "    Tope maximo de zonas      : %d permitidas\n", 
           limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25, maxZonasPermitidas);
}

void establecerLimites(Contaminacion *limitesOMS, int *maxZonasPermitidas){
    printf("\n  --- CONFIGURAR LIMITES MAXIMOS ---\n\n"
           "  Limites actuales permitidos:\n");
    mostrarLimites(*limitesOMS, *maxZonasPermitidas);

    printf("\n  Ingrese los nuevos limites maximos permitidos:\n");
    limitesOMS->co2  = leerFlotante("  Dioxido de Carbono (CO2) [1-5000]:  ", 1, 5000);
    limitesOMS->so2  = leerFlotante("  Dioxido de Azufre (SO2)  [1-500]:   ", 1, 500);
    limitesOMS->no2  = leerFlotante("  Dioxido de Nitrogeno(NO2)[1-500]:   ", 1, 500);
    limitesOMS->pm25 = leerFlotante("  Particulas Finas (PM2.5) [1-500]:   ", 1, 500);
    printf("  Tope maximo de zonas permitidas [1-100]: ");
    *maxZonasPermitidas = validarIntRango(1, 100);

    printf("\n  Limites actualizados en el sistema.\n");
    mostrarLimites(*limitesOMS, *maxZonasPermitidas);
}

void inicializarDatosQuito(Zona zonas[], int *numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
    int i;
    char nombres[5][50] = {
        "Centro Historico",
        "Belisario",
        "Carapungo",
        "El Camal",
        "Guamani"
    };
    Contaminacion datos[5] = {
        {450.0f, 25.0f, 30.0f, 18.0f},
        {350.0f, 15.0f, 20.0f, 12.0f},
        {480.0f, 28.0f, 35.0f, 22.0f},
        {400.0f, 18.0f, 24.0f, 14.0f},
        {520.0f, 35.0f, 42.0f, 28.0f}
    };
    Clima climas[5] = {
        {16.5f, 12.0f, 65.0f},
        {15.0f, 10.0f, 60.0f},
        {18.0f, 15.0f, 55.0f},
        {16.0f, 8.0f, 70.0f},
        {14.0f, 20.0f, 75.0f}
    };

    *numZonas = 5;
    for (i = 0; i < 5; i++){
        zonas[i].id = i + 1;
        strcpy(zonas[i].nombre, nombres[i]);
        zonas[i].actual = datos[i];
        zonas[i].climaActual = climas[i];
        genHistorial(&zonas[i], datos[i]);
    }

    guardarDatos(zonas, *numZonas, limitesOMS, maxZonasPermitidas);
    printf("\n  [INFO] Base de datos no encontrada. Se inicializo con 5 zonas de Quito, Ecuador.\n\n");
    for (i = 0; i < 5; i++) {
        printf("    ID %d: %s (Cargado)\n", zonas[i].id, zonas[i].nombre);
    }
}

void exportarReporteTXT(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i, alertas;
    float fc;
    Contaminacion prom, pred;
    FILE *archivo;

    if (!verificarZonas(numZonas)) return;

    archivo = fopen("reporte_calidad_aire.txt", "w");
    if (!archivo){
        printf("  [ERROR] No se pudo crear el archivo TXT.\n");
        return;
    }

    fprintf(archivo, "REPORTE DE CALIDAD DEL AIRE\n");
    fprintf(archivo, "======================================================\n\n");
    fprintf(archivo, "LIMITES CONFIGURADOS (OMS):\n");
    fprintf(archivo, "  CO2: %.2f | SO2: %.2f | NO2: %.2f | PM2.5: %.2f\n\n",
            limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25);

    for (i = 0; i < numZonas; i++){
        prom = calcPromedios(&zonas[i]);
        fc   = factorClimatico(&zonas[i].climaActual);
        pred = calcPrediccion(&zonas[i], fc, prom);

        fprintf(archivo, "------------------------------------------------------\n");
        fprintf(archivo, "ZONA %d: %s\n", zonas[i].id, zonas[i].nombre);
        fprintf(archivo, "------------------------------------------------------\n");

        fprintf(archivo, "\n  Condiciones Climaticas:\n");
        fprintf(archivo, "    Temperatura: %.1f C | Viento: %.1f km/h | Humedad: %.1f%%\n",
                zonas[i].climaActual.temperatura,
                zonas[i].climaActual.viento,
                zonas[i].climaActual.humedad);

        fprintf(archivo, "\n  Niveles Actuales vs Limite:\n");
        fprintf(archivo, "    CO2:   %.2f / %.2f  -> %s\n",
                zonas[i].actual.co2,  limitesOMS.co2,
                zonas[i].actual.co2  > limitesOMS.co2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    SO2:   %.2f / %.2f  -> %s\n",
                zonas[i].actual.so2,  limitesOMS.so2,
                zonas[i].actual.so2  > limitesOMS.so2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    NO2:   %.2f / %.2f  -> %s\n",
                zonas[i].actual.no2,  limitesOMS.no2,
                zonas[i].actual.no2  > limitesOMS.no2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    PM2.5: %.2f / %.2f  -> %s\n",
                zonas[i].actual.pm25, limitesOMS.pm25,
                zonas[i].actual.pm25 > limitesOMS.pm25 ? "EXCEDIDO" : "Normal");

        fprintf(archivo, "\n  Promedios Historicos (30 dias) vs Limite:\n");
        fprintf(archivo, "    CO2:   %.2f / %.2f  -> %s\n",
                prom.co2,  limitesOMS.co2,  prom.co2  > limitesOMS.co2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    SO2:   %.2f / %.2f  -> %s\n",
                prom.so2,  limitesOMS.so2,  prom.so2  > limitesOMS.so2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    NO2:   %.2f / %.2f  -> %s\n",
                prom.no2,  limitesOMS.no2,  prom.no2  > limitesOMS.no2  ? "EXCEDIDO" : "Normal");
        fprintf(archivo, "    PM2.5: %.2f / %.2f  -> %s\n",
                prom.pm25, limitesOMS.pm25, prom.pm25 > limitesOMS.pm25 ? "EXCEDIDO" : "Normal");

        fprintf(archivo, "\n  Prediccion 24h vs Limite:\n");
        fprintf(archivo, "    CO2:   %.2f / %.2f  -> %s\n",
                pred.co2,  limitesOMS.co2,  pred.co2  > limitesOMS.co2  ? "ALERTA" : "Normal");
        fprintf(archivo, "    SO2:   %.2f / %.2f  -> %s\n",
                pred.so2,  limitesOMS.so2,  pred.so2  > limitesOMS.so2  ? "ALERTA" : "Normal");
        fprintf(archivo, "    NO2:   %.2f / %.2f  -> %s\n",
                pred.no2,  limitesOMS.no2,  pred.no2  > limitesOMS.no2  ? "ALERTA" : "Normal");
        fprintf(archivo, "    PM2.5: %.2f / %.2f  -> %s\n",
                pred.pm25, limitesOMS.pm25, pred.pm25 > limitesOMS.pm25 ? "ALERTA" : "Normal");

        fprintf(archivo, "\n  Alertas y Recomendaciones:\n");
        alertas = 0;
        if (zonas[i].actual.co2 > limitesOMS.co2){
            fprintf(archivo, "    - HOY CO2 alto: Reducir combustibles fosiles y fomentar movilidad no motorizada.\n");
            alertas++;
        }
        if (zonas[i].actual.so2 > limitesOMS.so2){
            fprintf(archivo, "    - HOY SO2 alto: Inspeccionar industrias y proteger poblaciones vulnerables.\n");
            alertas++;
        }
        if (zonas[i].actual.no2 > limitesOMS.no2){
            fprintf(archivo, "    - HOY NO2 alto: Restringir trafico vehicular y promover transporte publico.\n");
            alertas++;
        }
        if (zonas[i].actual.pm25 > limitesOMS.pm25){
            fprintf(archivo, "    - HOY PM2.5 alto: Evitar actividades al aire libre y fomentar limpieza comunitaria.\n");
            alertas++;
        }
        if (pred.co2 > limitesOMS.co2){
            fprintf(archivo, "    - MANANA CO2: Preparar restricciones y activar protocolo de emergencia ambiental.\n");
            alertas++;
        }
        if (pred.so2 > limitesOMS.so2){
            fprintf(archivo, "    - MANANA SO2: Notificar a industrias para reducir emisiones y preparar alerta sanitaria.\n");
            alertas++;
        }
        if (pred.no2 > limitesOMS.no2){
            fprintf(archivo, "    - MANANA NO2: Planificar desvios de trafico e incrementar transporte publico.\n");
            alertas++;
        }
        if (pred.pm25 > limitesOMS.pm25){
            fprintf(archivo, "    - MANANA PM2.5: Emitir aviso de salud comunitaria para actividades al aire libre.\n");
            alertas++;
        }
        if (alertas == 0){
            fprintf(archivo, "    Calidad del aire buena. Sin alertas activas.\n");
        }
        fprintf(archivo, "\n");
    }

    fclose(archivo);
    printf("\n  Reporte exportado exitosamente a 'reporte_calidad_aire.txt'.\n");
}

int contieneSubcadenaIgnorandoMayusculas(const char *pajar, const char *aguja) {
    if (!*aguja) return 1;
    for (; *pajar; pajar++) {
        if (tolower((unsigned char)*pajar) == tolower((unsigned char)*aguja)) {
            const char *h = pajar;
            const char *n = aguja;
            while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
                h++;
                n++;
            }
            if (!*n) return 1;
        }
    }
    return 0;
}

void buscarZonasPorNombre(Zona *zonas, int numZonas) {
    char busqueda[50] = "";
    int encontradas = 0;
    int i;

    if (!verificarZonas(numZonas)) return;

    system("cls");
    printf("\n----------------------------------------------------------\n");
    printf("              BUSCAR ZONA POR NOMBRE\n");
    printf("----------------------------------------------------------\n");
    printf("  Ingrese el nombre de la zona a buscar: ");
    leerCadena(busqueda, 50);

    if (busqueda[0] == '\0') {
        printf("  [!] Busqueda cancelada o vacia.\n");
        return;
    }

    printf("\n  Resultados de la busqueda:\n");
    printf("  ----------------------------------------------------------\n");
    for (i = 0; i < numZonas; i++) {
        if (contieneSubcadenaIgnorandoMayusculas(zonas[i].nombre, busqueda)) {
            printf("    ID %d: %s\n", zonas[i].id, zonas[i].nombre);
            printf("    Niveles: CO2: %.1f | SO2: %.1f | NO2: %.1f | PM2.5: %.1f\n",
                   zonas[i].actual.co2, zonas[i].actual.so2, zonas[i].actual.no2, zonas[i].actual.pm25);
            printf("    Clima: %.1f C | Viento: %.1f km/h | Humedad: %.1f%%\n",
                   zonas[i].climaActual.temperatura, zonas[i].climaActual.viento, zonas[i].climaActual.humedad);
            printf("  ----------------------------------------------------------\n");
            encontradas++;
        }
    }
    if (encontradas == 0) {
        printf("  No se encontraron zonas que coincidan con '%s'.\n", busqueda);
    } else {
        printf("  Se encontraron %d zona(s).\n", encontradas);
    }
}

void filtrarZonasExcedidas(Zona *zonas, int numZonas, Contaminacion limitesOMS) {
    int encontradas = 0;
    int i;

    if (!verificarZonas(numZonas)) return;

    system("cls");
    printf("\n----------------------------------------------------------\n");
    printf("        ZONAS QUE EXCEDEN LIMITES OMS (EXCEDIDO)\n");
    printf("----------------------------------------------------------\n");
    printf("  Limites configurados: CO2: %.1f | SO2: %.1f | NO2: %.1f | PM2.5: %.1f\n\n",
           limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25);

    for (i = 0; i < numZonas; i++) {
        int excede = (zonas[i].actual.co2 > limitesOMS.co2 ||
                      zonas[i].actual.so2 > limitesOMS.so2 ||
                      zonas[i].actual.no2 > limitesOMS.no2 ||
                      zonas[i].actual.pm25 > limitesOMS.pm25);
        if (excede) {
            printf("    ID %d: %s\n", zonas[i].id, zonas[i].nombre);
            printf("    Excedentes detectados:\n");
            if (zonas[i].actual.co2 > limitesOMS.co2) {
                printf("      - CO2: %.2f (Excede %.2f)\n", zonas[i].actual.co2, limitesOMS.co2);
            }
            if (zonas[i].actual.so2 > limitesOMS.so2) {
                printf("      - SO2: %.2f (Excede %.2f)\n", zonas[i].actual.so2, limitesOMS.so2);
            }
            if (zonas[i].actual.no2 > limitesOMS.no2) {
                printf("      - NO2: %.2f (Excede %.2f)\n", zonas[i].actual.no2, limitesOMS.no2);
            }
            if (zonas[i].actual.pm25 > limitesOMS.pm25) {
                printf("      - PM2.5: %.2f (Excede %.2f)\n", zonas[i].actual.pm25, limitesOMS.pm25);
            }
            printf("  ----------------------------------------------------------\n");
            encontradas++;
        }
    }
    if (encontradas == 0) {
        printf("  Excelente: Ninguna zona excede los limites establecidos hoy.\n");
    } else {
        printf("  Se detectaron %d zona(s) con niveles excedidos.\n", encontradas);
    }
}

void editarZona(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas) {
    int opc = 0;
    int idx = -1;
    int i;
    char nuevoNombre[50] = "";

    if (numZonas == 0) {
        system("cls");
        printf("\n----------------------------------------------------------\n");
        printf("  [!] No hay zonas registradas para modificar.\n");
        printf("----------------------------------------------------------\n");
        return;
    }

    do {
        system("cls");
        printf("\n----------------------------------------------------------\n");
        printf("                 EDITAR NOMBRE DE ZONA\n");
        printf("----------------------------------------------------------\n");
        printf("  Zonas disponibles:\n");
        for (i = 0; i < numZonas; i++) {
            printf("    %d. %s\n", zonas[i].id, zonas[i].nombre);
        }
        printf("\n  Seleccione zona por ID (1 a %d) o ingrese 0 para volver: ", numZonas);
        opc = validarIntRango(0, numZonas);

        if (opc == 0) {
            return; // Volver
        }

        idx = opc - 1;
        printf("\n  Zona seleccionada: %s\n", zonas[idx].nombre);
        printf("  Ingrese nuevo nombre de la zona: ");
        leerCadena(nuevoNombre, 50);

        if (nuevoNombre[0] == '\0') {
            printf("  [!] El nombre no puede estar vacio. Edicion cancelada.\n");
            system("pause");
            continue;
        }

        // Validar si ya existe ese nombre en otra zona
        int duplicado = 0;
        for (i = 0; i < numZonas; i++) {
            if (i != idx && strcmp(zonas[i].nombre, nuevoNombre) == 0) {
                duplicado = 1;
                break;
            }
        }

        if (duplicado) {
            printf("  [!] Error: Ya existe otra zona con el nombre '%s'.\n", nuevoNombre);
            system("pause");
            continue;
        }

        // Guardar nombre antiguo para el mensaje
        char nombreAnterior[50];
        strcpy(nombreAnterior, zonas[idx].nombre);
        strcpy(zonas[idx].nombre, nuevoNombre);

        // Guardar en archivo
        guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
        printf("  [EXITO] Se actualizo el nombre de la zona ID %d:\n", zonas[idx].id);
        printf("          '%s' -> '%s'\n", nombreAnterior, zonas[idx].nombre);
        system("pause");
        break; // Salir de la función tras editar con éxito
    } while (1);
}

void menuConfiguracion(Zona *zonas, int numZonas, Contaminacion *limitesOMS, int *maxZonasPermitidas) {
    int opc = 0;
    do {
        system("cls");
        printf("\n----------------------------------------------------------\n");
        printf("                 CONFIGURACION DEL SISTEMA\n");
        printf("----------------------------------------------------------\n");
        printf("  1. Configurar limites de contaminacion y tope de zonas\n");
        printf("  2. Editar/renombrar una zona existente\n");
        printf("  3. Regresar al menu principal\n");
        printf("----------------------------------------------------------\n");
        printf(">> ");
        opc = validarIntRango(1, 3);

        switch (opc) {
        case 1:
            establecerLimites(limitesOMS, maxZonasPermitidas);
            guardarDatos(zonas, numZonas, *limitesOMS, *maxZonasPermitidas);
            system("pause");
            break;
        case 2:
            editarZona(zonas, numZonas, *limitesOMS, *maxZonasPermitidas);
            break;
        case 3:
            break;
        }
    } while (opc != 3);
}

void menuMonitoreoConsultas(Zona *zonas, int numZonas, Contaminacion limitesOMS) {
    int opc = 0;
    do {
        system("cls");
        printf("\n----------------------------------------------------------\n");
        printf("                   MONITOREO Y CONSULTAS\n");
        printf("----------------------------------------------------------\n");
        printf("  1. Ver resumen de todas las zonas (Monitoreo Actual)\n");
        printf("  2. Consultar una zona especifica en detalle\n");
        printf("  3. Buscar zona por nombre (coincidencia parcial)\n");
        printf("  4. Filtrar zonas que superan limites de la OMS\n");
        printf("  5. Regresar al menu principal\n");
        printf("----------------------------------------------------------\n");
        printf(">> ");
        opc = validarIntRango(1, 5);

        switch (opc) {
        case 1:
            system("cls");
            monitoreoActual(zonas, numZonas, limitesOMS);
            system("pause");
            break;
        case 2:
            consultarZonaDetalle(zonas, numZonas, limitesOMS);
            system("pause");
            break;
        case 3:
            buscarZonasPorNombre(zonas, numZonas);
            system("pause");
            break;
        case 4:
            filtrarZonasExcedidas(zonas, numZonas, limitesOMS);
            system("pause");
            break;
        case 5:
            break;
        }
    } while (opc != 5);
}

void consultarZonaDetalle(Zona *zonas, int numZonas, Contaminacion limitesOMS) {
    int i, idx;
    float fc;
    Contaminacion prom, pred;

    if (!verificarZonas(numZonas)) return;

    system("cls");
    printf("\n----------------------------------------------------------\n");
    printf("             CONSULTAR DETALLE DE ZONA\n");
    printf("----------------------------------------------------------\n");
    printf("  Zonas disponibles:\n");
    for (i = 0; i < numZonas; i++) {
        printf("    %d. %s\n", zonas[i].id, zonas[i].nombre);
    }
    printf("\n  Seleccione zona por ID (1 a %d): ", numZonas);
    idx = validarIntRango(1, numZonas) - 1;

    system("cls");
    Zona *z = &zonas[idx];
    printf("\n----------------------------------------------------------\n");
    printf("  REPORTE DETALLADO - ZONA %d: %s\n", z->id, z->nombre);
    printf("----------------------------------------------------------\n");
    printf("  Condiciones Climaticas:\n");
    printf("    Temperatura: %.1f C | Viento: %.1f km/h | Humedad: %.1f%%\n\n",
           z->climaActual.temperatura, z->climaActual.viento, z->climaActual.humedad);

    printf("  %-25s %10s %10s %10s\n", "Contaminante", "Hoy", "Historico", "Prediccion");
    printf("  ------------------------- ---------- ---------- ----------\n");
    
    prom = calcPromedios(z);
    fc = factorClimatico(&z->climaActual);
    pred = calcPrediccion(z, fc, prom);

    printf("  %-25s %10.1f %10.1f %10.1f\n", "CO2", z->actual.co2, prom.co2, pred.co2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "SO2", z->actual.so2, prom.so2, pred.so2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "NO2", z->actual.no2, prom.no2, pred.no2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "PM2.5", z->actual.pm25, prom.pm25, pred.pm25);
    printf("  ----------------------------------------------------------\n");

    printf("\n  Alertas y Recomendaciones Activas:\n");
    int alerta = 0;
    alerta += verificarYMostrarAlerta(z->actual.co2, limitesOMS.co2, "CO2 (Hoy)", "Reducir uso de combustibles fosiles y fomentar movilidad no motorizada (bicicletas).", 0);
    alerta += verificarYMostrarAlerta(z->actual.so2, limitesOMS.so2, "SO2 (Hoy)", "Inspeccionar industrias y proteger poblaciones vulnerables en refugios.", 0);
    alerta += verificarYMostrarAlerta(z->actual.no2, limitesOMS.no2, "NO2 (Hoy)", "Restringir trafico vehicular y promover uso de transporte publico (Salud Publica).", 0);
    alerta += verificarYMostrarAlerta(z->actual.pm25, limitesOMS.pm25, "PM2.5 (Hoy)", "Evitar actividades al aire libre y promover limpieza comunitaria de calles.", 0);

    alerta += verificarYMostrarAlerta(pred.co2, limitesOMS.co2, "CO2 (Manana)", "Preparar restricciones de trafico y activar protocolo de emergencia ambiental.", 1);
    alerta += verificarYMostrarAlerta(pred.so2, limitesOMS.so2, "SO2 (Manana)", "Notificar a industrias para reducir emisiones y preparar alerta sanitaria.", 1);
    alerta += verificarYMostrarAlerta(pred.no2, limitesOMS.no2, "NO2 (Manana)", "Planificar desvios de trafico e incrementar frecuencia de transporte publico.", 1);
    alerta += verificarYMostrarAlerta(pred.pm25, limitesOMS.pm25, "PM2.5 (Manana)", "Emitir aviso para proteger la salud comunitaria al aire libre.", 1);

    if (alerta == 0) {
        printf("    Calidad del aire buena. Sin alertas activas.\n");
    }
    printf("----------------------------------------------------------\n");
}