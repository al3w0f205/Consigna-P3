#include <stdio.h>
#include <string.h>
#include "funciones.h"

Contaminacion limitesOMS = {400.0f, 40.0f, 25.0f, 15.0f};

void limpiarBuffer(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void leerCadena(char *cadena, int n){
    fgets(cadena, n, stdin);
    int len = strlen(cadena) - 1;
    if (len >= 0 && cadena[len] == '\n'){
        cadena[len] = '\0';
    }
}

int leerEntero(const char *mensaje, int min, int max){
    int valor, res;
    while (1){
        printf("%s", mensaje);
        res = scanf("%d", &valor);
        if (res != 1){
            printf("  [!] Entrada invalida. Ingrese un numero entero.\n");
            limpiarBuffer();
            continue;
        }
        limpiarBuffer();
        if (valor < min || valor > max){
            printf("  [!] Valor fuera de rango [%d - %d].\n", min, max);
            continue;
        }
        return valor;
    }
}

float leerFlotante(const char *mensaje, float min, float max){
    float valor;
    int res;
    while (1){
        printf("%s", mensaje);
        res = scanf("%f", &valor);
        if (res != 1){
            printf("  [!] Entrada invalida. Ingrese un numero.\n");
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

void guardarDatos(Zona *zonas, int numZonas){
    FILE *archivo = fopen(ARCHIVO_DATOS, "wb");
    if (!archivo){
        printf("  [ERROR] No se pudo abrir '%s' para escritura.\n", ARCHIVO_DATOS);
        return;
    }
    fwrite(&limitesOMS, sizeof(Contaminacion), 1, archivo);

    fwrite(&numZonas, sizeof(int), 1, archivo);
    fwrite(zonas, sizeof(Zona), numZonas, archivo);
    fclose(archivo);
    printf("  Datos guardados exitosamente en '%s'.\n", ARCHIVO_DATOS);
}

int cargarDatos(Zona *zonas){
    int numZonas = 0;
    Contaminacion limBackup = limitesOMS;
    
    FILE *archivo = fopen(ARCHIVO_DATOS, "rb");
    if (!archivo) return 0;
    
    if (fread(&limitesOMS, sizeof(Contaminacion), 1, archivo) != 1 ||
        fread(&numZonas, sizeof(int), 1, archivo) != 1) {
        
        limitesOMS = limBackup;
        fclose(archivo); 
        return 0; 
    }
    
    if (numZonas > 0 && numZonas <= MAX_ZONAS){
        if (fread(zonas, sizeof(Zona), numZonas, archivo) != numZonas){
            limitesOMS = limBackup;
            fclose(archivo);
            return 0;
        }
    } else {
        numZonas = 0;
    }
    fclose(archivo);
    return numZonas;
}

int menu(void){
    printf("\n CONTROL DE CALIDAD DEL AIRE\n"
           " ----------------------------------------------------------\n"
           "   1. Configurar limites maximos de contaminacion\n"
           "   2. Registrar una nueva zona de la ciudad\n"
           "   3. Actualizar niveles de contaminacion de hoy\n"
           "   4. Ver estado actual del aire\n"
           "   5. Ver prediccion del aire para manana\n"
           "   6. Ver historial de los ultimos 30 dias\n"
           "   7. Revisar alertas y recomendaciones\n"
           "   8. Exportar reporte TXT para analisis\n"
           "   9. Salir\n"
           " ----------------------------------------------------------\n"
           ">> ");
    return leerEntero("", 1, 9);
}

int verificarZonas(int n){
    if (n == 0){
        printf("\n  [!] No hay zonas registradas. Use la opcion 2 o la 8 primero.\n");
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

void imprimirTabla(Contaminacion c){
    imprimirFila("Dioxido de Carbono (CO2)",  c.co2,  limitesOMS.co2);
    imprimirFila("Dioxido de Azufre (SO2)",   c.so2,  limitesOMS.so2);
    imprimirFila("Dioxido de Nitrogeno (NO2)", c.no2,  limitesOMS.no2);
    imprimirFila("Particulas Finas (PM2.5)",  c.pm25, limitesOMS.pm25);
}

void imprimirTablaPrediccion(Contaminacion actual, Contaminacion pred){
    imprimirFilaPred("Dioxido de Carbono (CO2)",  actual.co2,  pred.co2,  limitesOMS.co2);
    imprimirFilaPred("Dioxido de Azufre (SO2)",   actual.so2,  pred.so2,  limitesOMS.so2);
    imprimirFilaPred("Dioxido de Nitrogeno (NO2)", actual.no2,  pred.no2,  limitesOMS.no2);
    imprimirFilaPred("Particulas Finas (PM2.5)",  actual.pm25, pred.pm25, limitesOMS.pm25);
}

Contaminacion calcPromedios(Zona *z){
    Contaminacion prom = {0, 0, 0, 0};
    int d;
    for (d = 0; d < MAX_DIAS; d++){
        prom.co2  += z->historial[d].co2;
        prom.so2  += z->historial[d].so2;
        prom.no2  += z->historial[d].no2;
        prom.pm25 += z->historial[d].pm25;
    }
    prom.co2 /= MAX_DIAS;  prom.so2 /= MAX_DIAS;
    prom.no2 /= MAX_DIAS;  prom.pm25 /= MAX_DIAS;
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
    float fv = clamp(1.0f - (c->viento / 200.0f), 0.5f, 1.5f);
    float ft = clamp(1.0f + (c->temperatura - 20.0f) * 0.01f, 0.8f, 1.2f);
    float fh = clamp(1.0f + (c->humedad - 50.0f) * 0.005f, 0.85f, 1.15f);
    return fv * ft * fh;
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
    for (d = 0; d < MAX_DIAS; d++){
        f = 1.0f + (float)((d * 7 + z->id * 13) % 21 - 10) / 100.0f;
        z->historial[d].co2  = base.co2  * f;
        z->historial[d].so2  = base.so2  * f;
        z->historial[d].no2  = base.no2  * f;
        z->historial[d].pm25 = base.pm25 * f;
    }
}

void encabezadoSeccion(const char *titulo){
    printf("\n  === %s ===\n\n", titulo);
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
    c->no2  = leerFlotante("  Nivel de Dioxido Nitrogeno  (NO2) [0-500]:  ", 0, 500);
    c->pm25 = leerFlotante("  Nivel de Particulas Finas (PM2.5) [0-500]:  ", 0, 500);
}

void leerClima(Clima *c){
    c->temperatura = leerFlotante("  Temperatura en grados (0 a 50):           ", 0, 50);
    c->viento      = leerFlotante("  Velocidad del viento en km/h (0 a 200):   ", 0, 200);
    c->humedad     = leerFlotante("  Porcentaje de humedad (0 a 100):          ", 0, 100);
}

void registrarZona(Zona *zonas, int *numZonas){
    Zona *n;
    Contaminacion base;

    if (*numZonas >= MAX_ZONAS){
        printf("\n  [!] No se pueden agregar mas zonas (maximo %d).\n", MAX_ZONAS);
        return;
    }

    n = &zonas[*numZonas];
    n->id = *numZonas + 1;

    printf("\n  --- REGISTRAR NUEVA ZONA ---\n");
    printf("  Nombre de la zona: ");
    leerCadena(n->nombre, 50);

    printf("\n  --- NIVELES ACTUALES DE CONTAMINACION ---\n");
    leerContaminacion(&n->actual);

    printf("\n  --- CONDICIONES CLIMATICAS DE HOY ---\n");
    leerClima(&n->climaActual);

    printf("\n  --- GENERAR HISTORIAL PASADO ---\n");
    printf("  Ingrese niveles de ejemplo para simular los 30 dias anteriores:\n");
    leerContaminacion(&base);

    genHistorial(n, base);

    n->historial[MAX_DIAS - 1] = n->actual;

    (*numZonas)++;
    guardarDatos(zonas, *numZonas);
    printf("  Zona '%s' registrada exitosamente con ID %d.\n", n->nombre, n->id);
}

void registrarNiveles(Zona *zonas, int numZonas){
    int i, d, idx;

    if (!verificarZonas(numZonas)) return;

    printf("\n  --- ACTUALIZAR NIVELES DE CONTAMINACION ---\n\n");
    printf("  Zonas disponibles:\n");
    for (i = 0; i < numZonas; i++)
        printf("    %d. %s\n", zonas[i].id, zonas[i].nombre);
    printf("\n");

    idx = leerEntero("  Seleccione zona por ID: ", 1, numZonas) - 1;

    for (d = MAX_DIAS - 1; d > 0; d--)
        zonas[idx].historial[d] = zonas[idx].historial[d - 1];
    zonas[idx].historial[0] = zonas[idx].actual;

    printf("\n  Ingrese los nuevos niveles de contaminacion para '%s':\n", zonas[idx].nombre);
    leerContaminacion(&zonas[idx].actual);

    printf("\n  --- NUEVO ESTADO CLIMATICO ---\n");
    leerClima(&zonas[idx].climaActual);

    guardarDatos(zonas, numZonas);
    printf("  Niveles de la zona '%s' actualizados.\n", zonas[idx].nombre);
}

void monitoreoActual(Zona *zonas, int numZonas){
    int i;
    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("-----ESTADO ACTUAL DEL AIRE-----");
    printf("  Limites configurados: CO2=%.0f | SO2=%.0f | NO2=%.0f | PM2.5=%.0f\n\n",
           limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25);

    for (i = 0; i < numZonas; i++){
        encabezadoZona(zonas[i].id, zonas[i].nombre);
        printf("  %-34s %10s   %s\n"
               "  ---------------------------------- ----------   ----------\n", "Contaminante", "Nivel", "Estado");
        imprimirTabla(zonas[i].actual);
        printf("  Clima actual: %.1f grados | Viento: %.1f km/h | Humedad: %.1f%%\n\n",
               zonas[i].climaActual.temperatura,
               zonas[i].climaActual.viento,
               zonas[i].climaActual.humedad);
    }
}

void prediccion24h(Zona *zonas, int numZonas){
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
        imprimirTablaPrediccion(zonas[i].actual, pred);
        printf("\n");
    }
}

void promediosHistoricos(Zona *zonas, int numZonas){
    int i;
    Contaminacion prom;

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("               HISTORIAL DE LOS ULTIMOS 30 DIAS               ");

    for (i = 0; i < numZonas; i++){
        prom = calcPromedios(&zonas[i]);
        encabezadoZona(zonas[i].id, zonas[i].nombre);
        printf("  %-34s %10s   %s\n"
               "  ---------------------------------- ----------   ----------\n", "Contaminante", "Promedio", "Limite max");
        imprimirTabla(prom);
        printf("\n");
    }
}

void alertasYRecomendaciones(Zona *zonas, int numZonas){
    int i, alerta;
    float fc;
    Contaminacion prom, pred;

    const char *rCO2 = "Reducir uso de combustibles fosiles y fomentar movilidad no motorizada (bicicletas).";
    const char *rSO2 = "Inspeccionar industrias y proteger poblaciones vulnerables en refugios.";
    const char *rNO2 = "Restringir trafico vehicular y promover uso de transporte publico (Salud Publica).";
    const char *rPM25 = "Evitar actividades al aire libre y promover limpieza comunitaria de calles.";

    const char *rPrevCO2 = "Preparar restricciones de trafico y activar protocolo de emergencia ambiental.";
    const char *rPrevSO2 = "Notificar a industrias para reducir emisiones y preparar alerta sanitaria.";
    const char *rPrevNO2 = "Planificar desvios de trafico e incrementar frecuencia de transporte publico.";
    const char *rPrevPM25 = "Emitir aviso para proteger la salud comunitaria al aire libre.";

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("---ALERTAS Y RECOMENDACIONES---");

    for (i = 0; i < numZonas; i++){
        alerta = 0;

        encabezadoZona(zonas[i].id, zonas[i].nombre);

        alerta += verificarYMostrarAlerta(zonas[i].actual.co2, limitesOMS.co2, "Dioxido de Carbono (CO2)", rCO2, 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.so2, limitesOMS.so2, "Dioxido de Azufre (SO2)", rSO2, 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.no2, limitesOMS.no2, "Dioxido de Nitrogeno (NO2)", rNO2, 0);
        alerta += verificarYMostrarAlerta(zonas[i].actual.pm25, limitesOMS.pm25, "Particulas Finas (PM2.5)", rPM25, 0);

        prom = calcPromedios(&zonas[i]);
        fc = factorClimatico(&zonas[i].climaActual);
        pred = calcPrediccion(&zonas[i], fc, prom);

        alerta += verificarYMostrarAlerta(pred.co2, limitesOMS.co2, "Dioxido de Carbono (CO2)", rPrevCO2, 1);
        alerta += verificarYMostrarAlerta(pred.so2, limitesOMS.so2, "Dioxido de Azufre (SO2)", rPrevSO2, 1);
        alerta += verificarYMostrarAlerta(pred.no2, limitesOMS.no2, "Dioxido de Nitrogeno (NO2)", rPrevNO2, 1);
        alerta += verificarYMostrarAlerta(pred.pm25, limitesOMS.pm25, "Particulas Finas (PM2.5)", rPrevPM25, 1);

        if (alerta == 0){
            printf("  Todo en orden: La calidad del aire es buena hoy y manana.\n"
                   "  No hay alertas ni recomendaciones.\n");
        }
        printf("\n");
    }
}

void mostrarLimites(void){
    printf("    Dioxido de Carbono (CO2)  : %.2f maximo\n"
           "    Dioxido de Azufre (SO2)   : %.2f maximo\n"
           "    Dioxido de Nitrogeno (NO2): %.2f maximo\n"
           "    Particulas Finas (PM2.5)  : %.2f maximo\n", limitesOMS.co2, limitesOMS.so2, limitesOMS.no2, limitesOMS.pm25);
}

void establecerLimites(void){
    printf("\n  --- CONFIGURAR LIMITES MAXIMOS ---\n\n"
           "  Limites actuales permitidos:\n");
    mostrarLimites();

    printf("\n  Ingrese los nuevos limites maximos permitidos:\n");
    limitesOMS.co2  = leerFlotante("  Dioxido de Carbono (CO2) [1-5000]:  ", 1, 5000);
    limitesOMS.so2  = leerFlotante("  Dioxido de Azufre (SO2)  [1-500]:   ", 1, 500);
    limitesOMS.no2  = leerFlotante("  Dioxido de Nitrogeno(NO2)[1-500]:   ", 1, 500);
    limitesOMS.pm25 = leerFlotante("  Particulas Finas (PM2.5) [1-500]:   ", 1, 500);

    printf("\n  Limites actualizados en el sistema.\n");
    mostrarLimites();
}

void cargarDatosPrueba(Zona *zonas, int *numZonas){
    int i;
    char nombres[5][50] = {"Centro Historico", "Zona Norte Industrial",
                           "Zona Sur Residencial", "Zona Este Comercial",
                           "Zona Oeste Universitaria"};
    Contaminacion datos[5] = {
        {520, 22, 45, 30}, {480, 25, 38, 22}, {380, 12, 25, 18},
        {450, 18, 42, 28}, {350, 10, 20, 15}
    };
    Clima climas[5] = {
        {28, 8, 65}, {25, 12, 55}, {22, 15, 60},
        {26, 10, 58}, {20, 20, 50}
    };

    if (*numZonas > 0){
        printf("\n  [!] Ya existen %d zonas registradas.\n", *numZonas);
        printf("  Si continua, los datos actuales se sobreescribiran.\n");
        if (leerEntero("  Confirmar carga de datos de prueba (1=Si, 0=No): ", 0, 1) == 0){
            printf("  Operacion cancelada.\n");
            return;
        }
    }

    *numZonas = 5;
    for (i = 0; i < 5; i++){
        zonas[i].id = i + 1;
        strcpy(zonas[i].nombre, nombres[i]);
        zonas[i].actual = datos[i];
        zonas[i].climaActual = climas[i];
        genHistorial(&zonas[i], datos[i]);
    }

    guardarDatos(zonas, *numZonas);
    printf("\n  Se cargaron 5 zonas de prueba (con datos altos para testear alertas).\n\n");
    for (i = 0; i < 5; i++)
        printf("    %d. %s\n", zonas[i].id, zonas[i].nombre);
        
    printf("\n  Nota: Los datos se guardan automaticamente.\n");
}

void exportarReporteTXT(Zona *zonas, int numZonas){
    if (!verificarZonas(numZonas)) return;

    FILE *archivo = fopen("reporte_calidad_aire.txt", "w");
    if (!archivo){
        printf("  [ERROR] No se pudo crear el archivo TXT.\n");
        return;
    }

    fprintf(archivo, "REPORTE DE CALIDAD DEL AIRE\n");
    fprintf(archivo, "------------------------------------------------------\n\n");

    for (int i = 0; i < numZonas; i++){
        Contaminacion prom = calcPromedios(&zonas[i]);
        float fc = factorClimatico(&zonas[i].climaActual);
        Contaminacion pred = calcPrediccion(&zonas[i], fc, prom);

        fprintf(archivo, "ZONA: %s\n", zonas[i].nombre);
        fprintf(archivo, "  Niveles Actuales:\n");
        fprintf(archivo, "    CO2: %.2f | SO2: %.2f | NO2: %.2f | PM2.5: %.2f\n",
                zonas[i].actual.co2, zonas[i].actual.so2, zonas[i].actual.no2, zonas[i].actual.pm25);
        fprintf(archivo, "  Prediccion (24h):\n");
        fprintf(archivo, "    CO2: %.2f | SO2: %.2f | NO2: %.2f | PM2.5: %.2f\n\n",
                pred.co2, pred.so2, pred.no2, pred.pm25);
    }
    fclose(archivo);
    printf("\n  Reporte exportado exitosamente a 'reporte_calidad_aire.txt'.\n");
}