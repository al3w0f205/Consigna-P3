#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funciones.h"

void limpiarPantalla(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}

void pausarPantalla(void) {
#ifdef _WIN32
    system("pause");
#else
    printf("  Presione Enter para continuar... ");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
#endif
}

void mostrarZonasResumidas(Zona *zonas, int numZonas) {
    int i;
    printf("\n  --------------------------------------------------------------------------------------\n");
    printf("  %-4s %-25s %8s %8s %8s %8s %8s\n", "ID", "Nombre de Zona", "CO2", "SO2", "NO2", "PM2.5", "Temp");
    printf("  --------------------------------------------------------------------------------------\n");
    for (i = 0; i < numZonas; i++) {
        printf("   %-2d  %-25s %8.1f %8.1f %8.1f %8.1f %6.1f C\n",
               zonas[i].id,
               zonas[i].nombre,
               zonas[i].actual.co2,
               zonas[i].actual.so2,
               zonas[i].actual.no2,
               zonas[i].actual.pm25,
               zonas[i].climaActual.temperatura);
    }
    printf("  --------------------------------------------------------------------------------------\n\n");
}

void limpiarBuffer(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int esCadenaVaciaOEspacios(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
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
    } else {
        limpiarBuffer();
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

int compararSalir(const char *str) {
    char s[10] = "";
    int i;
    for (i = 0; i < 9 && str[i]; i++) {
        s[i] = tolower((unsigned char)str[i]);
    }
    s[i] = '\0';
    return (strcmp(s, "salir") == 0);
}

float leerFlotante(const char *mensaje, float min, float max, int *cancelado){
    char buffer[100];
    float valor;
    int res;
    while (1){
        printf("%s", mensaje);
        leerCadena(buffer, 100);
        
        if (compararSalir(buffer)) {
            *cancelado = 1;
            return 0.0f;
        }
        
        res = sscanf(buffer, "%f", &valor);
        if (res != 1){
            printf("  [!] Entrada invalida. Ingrese un numero decimal o escriba 'salir' para cancelar.\n");
            continue;
        }
        if (valor < min || valor > max){
            printf("  [!] Valor fuera de rango [%.2f - %.2f].\n", min, max);
            continue;
        }
        *cancelado = 0;
        return valor;
    }
}

void guardarDatos(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
    FILE *archivo = fopen(ARCHIVO_DATOS, "wb");
    if (!archivo){
        printf("  [ERROR] No se pudo abrir '%s' para escritura.\n", ARCHIVO_DATOS);
        return;
    }
    
    fwrite(&limitesOMS.co2, sizeof(float), 1, archivo);
    fwrite(&limitesOMS.so2, sizeof(float), 1, archivo);
    fwrite(&limitesOMS.no2, sizeof(float), 1, archivo);
    fwrite(&limitesOMS.pm25, sizeof(float), 1, archivo);
    
    fwrite(&maxZonasPermitidas, sizeof(int), 1, archivo);
    fwrite(&numZonas, sizeof(int), 1, archivo);
    
    if (numZonas > 0 && zonas != NULL) {
        int i;
        for (i = 0; i < numZonas; i++) {
            fwrite(&zonas[i].id, sizeof(int), 1, archivo);
            fwrite(zonas[i].nombre, sizeof(char), 50, archivo);
            fwrite(zonas[i].historial, sizeof(RegistroHistorial), MAX_DIAS, archivo);
            fwrite(&zonas[i].head, sizeof(int), 1, archivo);
            fwrite(&zonas[i].count, sizeof(int), 1, archivo);
            fwrite(&zonas[i].actual, sizeof(Contaminacion), 1, archivo);
            fwrite(&zonas[i].climaActual, sizeof(Clima), 1, archivo);
        }
    }
    fclose(archivo);
    printf("  Datos guardados exitosamente en '%s'.\n", ARCHIVO_DATOS);
}

int cargarDatos(Zona **zonas, Contaminacion *limitesOMS, int *maxZonasPermitidas){
    if (*zonas != NULL) {
        free(*zonas);
        *zonas = NULL;
    }
    int numZonas = 0;
    Contaminacion limBackup = *limitesOMS;
    int maxBackup = *maxZonasPermitidas;
    
    FILE *archivo = fopen(ARCHIVO_DATOS, "rb");
    if (!archivo) return -2; // Archivo no existe
    
    if (fread(&limitesOMS->co2, sizeof(float), 1, archivo) != 1 ||
        fread(&limitesOMS->so2, sizeof(float), 1, archivo) != 1 ||
        fread(&limitesOMS->no2, sizeof(float), 1, archivo) != 1 ||
        fread(&limitesOMS->pm25, sizeof(float), 1, archivo) != 1 ||
        fread(maxZonasPermitidas, sizeof(int), 1, archivo) != 1 ||
        fread(&numZonas, sizeof(int), 1, archivo) != 1) {
        
        *limitesOMS = limBackup;
        *maxZonasPermitidas = maxBackup;
        fclose(archivo); 
        return -1; // Archivo corrupto
    }
    
    if (numZonas < 0) {
        *limitesOMS = limBackup;
        *maxZonasPermitidas = maxBackup;
        fclose(archivo);
        return -1; // numZonas negativo indica corrupcion
    }
    
    if (numZonas > 0){
        *zonas = malloc(numZonas * sizeof(Zona));
        if (*zonas == NULL) {
            *limitesOMS = limBackup;
            *maxZonasPermitidas = maxBackup;
            fclose(archivo);
            return -1; // Fallo de asignacion de memoria (tratar como error)
        }
        int i;
        for (i = 0; i < numZonas; i++) {
            if (fread(&(*zonas)[i].id, sizeof(int), 1, archivo) != 1 ||
                fread((*zonas)[i].nombre, sizeof(char), 50, archivo) != 50 ||
                fread((*zonas)[i].historial, sizeof(RegistroHistorial), MAX_DIAS, archivo) != MAX_DIAS ||
                fread(&(*zonas)[i].head, sizeof(int), 1, archivo) != 1 ||
                fread(&(*zonas)[i].count, sizeof(int), 1, archivo) != 1 ||
                fread(&(*zonas)[i].actual, sizeof(Contaminacion), 1, archivo) != 1 ||
                fread(&(*zonas)[i].climaActual, sizeof(Clima), 1, archivo) != 1) {
                
                free(*zonas);
                *zonas = NULL;
                *limitesOMS = limBackup;
                *maxZonasPermitidas = maxBackup;
                fclose(archivo);
                return -1; // Archivo corrupto
            }
            (*zonas)[i].nombre[49] = '\0'; // Asegurar terminador nulo
            // Validar que head y count esten en rangos validos
            if ((*zonas)[i].head < 0 || (*zonas)[i].head >= MAX_DIAS ||
                (*zonas)[i].count < 0 || (*zonas)[i].count > MAX_DIAS) {
                free(*zonas);
                *zonas = NULL;
                *limitesOMS = limBackup;
                *maxZonasPermitidas = maxBackup;
                fclose(archivo);
                return -1; // Datos de historial corruptos
            }
        }
    } else {
        *zonas = NULL;
    }
    fclose(archivo);
    return numZonas;
}

int menu(void){
    limpiarPantalla();
    printf("\n----------------------------------------------------------\n");
    printf("            SISTEMA SIGPA-QUITO (MONITOREO DE AIRE)\n");
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
    int i;
    if (z->count == 0) return z->actual;
    Contaminacion prom = {0, 0, 0, 0};
    
    for (i = 0; i < z->count; i++){
        int pos = (z->head + i) % MAX_DIAS;
        prom.co2  += z->historial[pos].niveles.co2;
        prom.so2  += z->historial[pos].niveles.so2;
        prom.no2  += z->historial[pos].niveles.no2;
        prom.pm25 += z->historial[pos].niveles.pm25;
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
    time_t ahora = time(NULL);
    
    z->head = 0;
    z->count = MAX_DIAS;
    
    for (d = 0; d < MAX_DIAS; d++){
        time_t diaSim = ahora - (MAX_DIAS - d) * 24 * 3600;
        struct tm *infoTiempo = localtime(&diaSim);
        if (infoTiempo != NULL) {
            strftime(z->historial[d].fechaHora, 20, "%Y-%m-%d %H:%M:%S", infoTiempo);
        } else {
            strcpy(z->historial[d].fechaHora, "2026-01-01 00:00:00");
        }

        f = 1.0f + (float)((d * 7 + z->id * 13) % 21 - 10) / 100.0f;
        z->historial[d].niveles.co2  = base.co2  * f;
        z->historial[d].niveles.so2  = base.so2  * f;
        z->historial[d].niveles.no2  = base.no2  * f;
        z->historial[d].niveles.pm25 = base.pm25 * f;
    }
}

void encabezadoSeccion(const char *titulo){
    printf("\n  ======================================================================================\n");
    printf("    %s\n", titulo);
    printf("  ======================================================================================\n\n");
}

void encabezadoZona(int id, const char *nombre){
    printf("  >> Zona %d: %s\n\n", id, nombre);
}

int verificarYMostrarAlerta(FILE *stream, float val, float lim, const char *param, const char *rec, int prev){
    if (val > lim){
        if (prev)
            fprintf(stream, "  - MANANA: Nivel de %s estara alto (%.2f / Limite: %.0f)\n", param, val, lim);
        else
            fprintf(stream, "  - HOY: Nivel de %s esta alto (%.2f / Limite: %.0f)\n", param, val, lim);
        
        fprintf(stream, "    Sugerencia: %s\n\n", rec);
        return 1;
    }
    return 0;
}

int imprimirAlertasGenerales(FILE *stream, Zona *zona, Contaminacion limitesOMS, Contaminacion pred) {
    int alerta = 0;
    
    alerta += verificarYMostrarAlerta(stream, zona->actual.co2, limitesOMS.co2, 
                                     "Dioxido de Carbono (CO2)", 
                                     "Reducir uso de combustibles fosiles y fomentar movilidad no motorizada (bicicletas).", 0);
    alerta += verificarYMostrarAlerta(stream, zona->actual.so2, limitesOMS.so2, 
                                     "Dioxido de Azufre (SO2)", 
                                     "Inspeccionar industrias y proteger poblaciones vulnerables en refugios.", 0);
    alerta += verificarYMostrarAlerta(stream, zona->actual.no2, limitesOMS.no2, 
                                     "Dioxido de Nitrogeno (NO2)", 
                                     "Restringir trafico vehicular y promover uso de transporte publico (Salud Publica).", 0);
    alerta += verificarYMostrarAlerta(stream, zona->actual.pm25, limitesOMS.pm25, 
                                     "Particulas Finas (PM2.5)", 
                                     "Evitar actividades al aire libre y promover limpieza comunitaria de calles.", 0);

    alerta += verificarYMostrarAlerta(stream, pred.co2, limitesOMS.co2, 
                                     "Dioxido de Carbono (CO2)", 
                                     "Preparar restricciones de trafico y activar protocolo de emergencia ambiental.", 1);
    alerta += verificarYMostrarAlerta(stream, pred.so2, limitesOMS.so2, 
                                     "Dioxido de Azufre (SO2)", 
                                     "Notificar a industrias para reducir emisiones y preparar alerta sanitaria.", 1);
    alerta += verificarYMostrarAlerta(stream, pred.no2, limitesOMS.no2, 
                                     "Dioxido de Nitrogeno (NO2)", 
                                     "Planificar desvios de trafico e incrementar frecuencia de transporte publico.", 1);
    alerta += verificarYMostrarAlerta(stream, pred.pm25, limitesOMS.pm25, 
                                     "Particulas Finas (PM2.5)", 
                                     "Emitir aviso para proteger la salud comunitaria al aire libre.", 1);
                                     
    if (alerta == 0) {
        if (stream == stdout) {
            fprintf(stream, "  Todo en orden: La calidad del aire es buena hoy y manana.\n"
                            "  No hay alertas ni recomendaciones.\n");
        } else {
            fprintf(stream, "    Calidad del aire buena. Sin alertas activas.\n");
        }
    }
    return alerta;
}

int leerContaminacion(Contaminacion *c){
    int cancelado = 0;
    c->co2  = leerFlotante("  Nivel de Dioxido de Carbono (CO2) [0-2000]: ", 0, 2000, &cancelado);
    if (cancelado) return 0;
    c->so2  = leerFlotante("  Nivel de Dioxido de Azufre  (SO2) [0-500]:  ", 0, 500, &cancelado);
    if (cancelado) return 0;
    c->no2  = leerFlotante("  Nivel de Dioxido de Nitrogeno  (NO2) [0-500]:  ", 0, 500, &cancelado);
    if (cancelado) return 0;
    c->pm25 = leerFlotante("  Nivel de Particulas Finas (PM2.5) [0-500]:  ", 0, 500, &cancelado);
    if (cancelado) return 0;
    return 1;
}

int leerClima(Clima *c){
    int cancelado = 0;
    c->temperatura = leerFlotante("  Temperatura en grados (0 a 50):           ", 0, 50, &cancelado);
    if (cancelado) return 0;
    c->viento      = leerFlotante("  Velocidad del viento en km/h (0 a 200):   ", 0, 200, &cancelado);
    if (cancelado) return 0;
    c->humedad     = leerFlotante("  Porcentaje de humedad (0 a 100):          ", 0, 100, &cancelado);
    if (cancelado) return 0;
    return 1;
}

void registrarZona(Zona **zonas, int *numZonas, int maxZonasPermitidas, Contaminacion limitesOMS){
    Zona nuevaZona;
    Contaminacion base;
    int duplicado;

    if (*numZonas >= maxZonasPermitidas) {
        printf("\n  [!] Se ha alcanzado el tope maximo de zonas configurado (%d).\n", maxZonasPermitidas);
        return;
    }

    nuevaZona.id = *numZonas + 1;
    nuevaZona.head = 0;
    nuevaZona.count = 0;

    printf("\n  --- REGISTRAR NUEVA ZONA ---\n");
    do {
        duplicado = 0;
        printf("  Nombre de la zona (o escriba 'salir' para cancelar): ");
        leerCadena(nuevaZona.nombre, 50);
        if (compararSalir(nuevaZona.nombre)) {
            printf("\n  [!] Registro cancelado por el usuario.\n");
            return;
        }
        if (esCadenaVaciaOEspacios(nuevaZona.nombre)){
            printf("  [!] El nombre no puede estar vacio. Intentelo de nuevo.\n");
            continue;
        }

        int i;
        for (i = 0; i < *numZonas; i++) {
            if (strcmp((*zonas)[i].nombre, nuevaZona.nombre) == 0) {
                duplicado = 1;
                break;
            }
        }
        if (duplicado) {
            printf("  [!] Error: Ya existe otra zona con el nombre '%s'. Intentelo de nuevo.\n", nuevaZona.nombre);
        }
    } while (esCadenaVaciaOEspacios(nuevaZona.nombre) || duplicado);

    printf("\n  --- NIVELES ACTUALES DE CONTAMINACION (o escriba 'salir' para cancelar) ---\n");
    if (!leerContaminacion(&nuevaZona.actual)) {
        printf("\n  [!] Registro cancelado por el usuario.\n");
        return;
    }

    printf("\n  --- CONDICIONES CLIMATICAS DE HOY (o escriba 'salir' para cancelar) ---\n");
    if (!leerClima(&nuevaZona.climaActual)) {
        printf("\n  [!] Registro cancelado por el usuario.\n");
        return;
    }

    printf("\n  --- GENERAR HISTORIAL PASADO (o escriba 'salir' para cancelar) ---\n");
    printf("  Ingrese niveles de ejemplo para simular los 30 dias anteriores:\n");
    if (!leerContaminacion(&base)) {
        printf("\n  [!] Registro cancelado por el usuario.\n");
        return;
    }

    genHistorial(&nuevaZona, base);

    if (nuevaZona.count > 0) {
        int pos = (nuevaZona.head + nuevaZona.count - 1) % MAX_DIAS;
        nuevaZona.historial[pos].niveles = nuevaZona.actual;
        time_t ahora = time(NULL);
        struct tm *infoTiempo = localtime(&ahora);
        if (infoTiempo != NULL) {
            strftime(nuevaZona.historial[pos].fechaHora, 20, "%Y-%m-%d %H:%M:%S", infoTiempo);
        } else {
            strcpy(nuevaZona.historial[pos].fechaHora, "2026-06-25 00:00:00");
        }
    }

    Zona *temp = realloc(*zonas, (*numZonas + 1) * sizeof(Zona));
    if (!temp) {
        printf("  [ERROR] No se pudo asignar memoria para la nueva zona.\n");
        return;
    }
    *zonas = temp;
    (*zonas)[*numZonas] = nuevaZona;

    if (nuevaZona.actual.co2 > limitesOMS.co2) registrarAlertaLog(nuevaZona.nombre, "CO2", nuevaZona.actual.co2, limitesOMS.co2);
    if (nuevaZona.actual.so2 > limitesOMS.so2) registrarAlertaLog(nuevaZona.nombre, "SO2", nuevaZona.actual.so2, limitesOMS.so2);
    if (nuevaZona.actual.no2 > limitesOMS.no2) registrarAlertaLog(nuevaZona.nombre, "NO2", nuevaZona.actual.no2, limitesOMS.no2);
    if (nuevaZona.actual.pm25 > limitesOMS.pm25) registrarAlertaLog(nuevaZona.nombre, "PM2.5", nuevaZona.actual.pm25, limitesOMS.pm25);

    (*numZonas)++;
    guardarDatos(*zonas, *numZonas, limitesOMS, maxZonasPermitidas);
    printf("  Zona '%s' registrada exitosamente con ID %d.\n", nuevaZona.nombre, nuevaZona.id);
}

void registrarNiveles(Zona *zonas, int numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
    int idx;
    Contaminacion tempActual;
    Clima tempClima;

    if (!verificarZonas(numZonas)) return;

    printf("\n  --- ACTUALIZAR NIVELES DE CONTAMINACION ---\n\n");
    printf("  Zonas disponibles:\n");
    mostrarZonasResumidas(zonas, numZonas);

    printf("  Seleccione zona por ID (o ingrese 0 para volver): ");
    idx = validarIntRango(0, numZonas);
    if (idx == 0) {
        return;
    }
    idx--;

    printf("\n  Ingrese los nuevos niveles de contaminacion para '%s' (o escriba 'salir' para cancelar):\n", zonas[idx].nombre);
    if (!leerContaminacion(&tempActual)) {
        printf("\n  [!] Actualizacion cancelada por el usuario.\n");
        return;
    }

    printf("\n  --- NUEVO ESTADO CLIMATICO (o escriba 'salir' para cancelar) ---\n");
    if (!leerClima(&tempClima)) {
        printf("\n  [!] Actualizacion cancelada por el usuario.\n");
        return;
    }

    zonas[idx].actual = tempActual;
    zonas[idx].climaActual = tempClima;

    int pos = (zonas[idx].head + zonas[idx].count) % MAX_DIAS;
    zonas[idx].historial[pos].niveles = zonas[idx].actual;
    
    time_t ahora = time(NULL);
    struct tm *infoTiempo = localtime(&ahora);
    if (infoTiempo != NULL) {
        strftime(zonas[idx].historial[pos].fechaHora, 20, "%Y-%m-%d %H:%M:%S", infoTiempo);
    } else {
        strcpy(zonas[idx].historial[pos].fechaHora, "2026-06-25 00:00:00");
    }
    
    if (zonas[idx].count < MAX_DIAS) zonas[idx].count++;
    else zonas[idx].head = (zonas[idx].head + 1) % MAX_DIAS;

    if (zonas[idx].actual.co2 > limitesOMS.co2) registrarAlertaLog(zonas[idx].nombre, "CO2", zonas[idx].actual.co2, limitesOMS.co2);
    if (zonas[idx].actual.so2 > limitesOMS.so2) registrarAlertaLog(zonas[idx].nombre, "SO2", zonas[idx].actual.so2, limitesOMS.so2);
    if (zonas[idx].actual.no2 > limitesOMS.no2) registrarAlertaLog(zonas[idx].nombre, "NO2", zonas[idx].actual.no2, limitesOMS.no2);
    if (zonas[idx].actual.pm25 > limitesOMS.pm25) registrarAlertaLog(zonas[idx].nombre, "PM2.5", zonas[idx].actual.pm25, limitesOMS.pm25);

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
    int i;
    float fc;
    Contaminacion prom, pred;

    if (!verificarZonas(numZonas)) return;

    encabezadoSeccion("---ALERTAS Y RECOMENDACIONES---");

    for (i = 0; i < numZonas; i++){
        encabezadoZona(zonas[i].id, zonas[i].nombre);

        prom = calcPromedios(&zonas[i]);
        fc = factorClimatico(&zonas[i].climaActual);
        pred = calcPrediccion(&zonas[i], fc, prom);

        imprimirAlertasGenerales(stdout, &zonas[i], limitesOMS, pred);
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

void establecerLimites(Contaminacion *limitesOMS, int *maxZonasPermitidas, int numZonas){
    printf("\n  --- CONFIGURAR LIMITES MAXIMOS ---\n\n"
           "  Limites actuales permitidos:\n");
    mostrarLimites(*limitesOMS, *maxZonasPermitidas);

    int cancelado = 0;
    Contaminacion tempOMS = *limitesOMS;
    
    printf("\n  Ingrese los nuevos limites maximos permitidos (o escriba 'salir' para cancelar):\n");
    tempOMS.co2  = leerFlotante("  Dioxido de Carbono (CO2) [1-5000]:  ", 1, 5000, &cancelado);
    if (cancelado) { printf("\n  [!] Configuracion cancelada.\n"); return; }
    tempOMS.so2  = leerFlotante("  Dioxido de Azufre (SO2)  [1-500]:   ", 1, 500, &cancelado);
    if (cancelado) { printf("\n  [!] Configuracion cancelada.\n"); return; }
    tempOMS.no2  = leerFlotante("  Dioxido de Nitrogeno(NO2)[1-500]:   ", 1, 500, &cancelado);
    if (cancelado) { printf("\n  [!] Configuracion cancelada.\n"); return; }
    tempOMS.pm25 = leerFlotante("  Particulas Finas (PM2.5) [1-500]:   ", 1, 500, &cancelado);
    if (cancelado) { printf("\n  [!] Configuracion cancelada.\n"); return; }
    
    int minZonas = numZonas > 0 ? numZonas : 1;
    printf("  Tope maximo de zonas permitidas [%d-100] (o ingrese 0 para cancelar): ", minZonas);
    int nuevoMax = validarIntRango(0, 100);
    if (nuevoMax == 0) {
        printf("\n  [!] Configuracion cancelada.\n");
        return;
    }
    if (nuevoMax < minZonas) {
        printf("  [!] Error: El tope maximo de zonas no puede ser menor al numero de zonas registradas (%d).\n", numZonas);
        printf("  Configuracion cancelada.\n");
        return;
    }
    
    *limitesOMS = tempOMS;
    *maxZonasPermitidas = nuevoMax;

    printf("\n  Limites actualizados en el sistema.\n");
    mostrarLimites(*limitesOMS, *maxZonasPermitidas);
}

void inicializarDatosQuito(Zona **zonas, int *numZonas, Contaminacion limitesOMS, int maxZonasPermitidas){
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
    *zonas = malloc(5 * sizeof(Zona));
    if (*zonas == NULL) {
        printf("  [ERROR] No se pudo asignar memoria para inicializar zonas.\n");
        *numZonas = 0;
        return;
    }

    for (i = 0; i < 5; i++){
        (*zonas)[i].id = i + 1;
        strcpy((*zonas)[i].nombre, nombres[i]);
        (*zonas)[i].actual = datos[i];
        (*zonas)[i].climaActual = climas[i];
        genHistorial(&(*zonas)[i], datos[i]);
    }

    guardarDatos(*zonas, *numZonas, limitesOMS, maxZonasPermitidas);
    printf("\n  [INFO] Base de datos no encontrada. Se inicializo con 5 zonas de Quito, Ecuador.\n\n");
    for (i = 0; i < 5; i++) {
        printf("    ID %d: %s (Cargado)\n", (*zonas)[i].id, (*zonas)[i].nombre);
    }
}

void exportarReporteTXT(Zona *zonas, int numZonas, Contaminacion limitesOMS){
    int i;
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
    fprintf(archivo, "======================================================\n");
    {
        time_t ahora = time(NULL);
        struct tm *infoTiempo = localtime(&ahora);
        char fechaReporte[20];
        if (infoTiempo != NULL) {
            strftime(fechaReporte, sizeof(fechaReporte), "%Y-%m-%d %H:%M:%S", infoTiempo);
        } else {
            strcpy(fechaReporte, "Fecha no disponible");
        }
        fprintf(archivo, "Generado el: %s\n\n", fechaReporte);
    }
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
        imprimirAlertasGenerales(archivo, &zonas[i], limitesOMS, pred);
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

    limpiarPantalla();
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

    limpiarPantalla();
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
        limpiarPantalla();
        printf("\n----------------------------------------------------------\n");
        printf("  [!] No hay zonas registradas para modificar.\n");
        printf("----------------------------------------------------------\n");
        return;
    }

    do {
        limpiarPantalla();
        printf("\n----------------------------------------------------------\n");
        printf("                 EDITAR NOMBRE DE ZONA\n");
        printf("----------------------------------------------------------\n");
        printf("  Zonas disponibles:\n");
        mostrarZonasResumidas(zonas, numZonas);
        printf("  Seleccione zona por ID (1 a %d) o ingrese 0 para volver: ", numZonas);
        opc = validarIntRango(0, numZonas);

        if (opc == 0) {
            return;
        }

        idx = opc - 1;
        printf("\n  Zona seleccionada: %s\n", zonas[idx].nombre);
        printf("  Ingrese nuevo nombre de la zona: ");
        leerCadena(nuevoNombre, 50);

        if (esCadenaVaciaOEspacios(nuevoNombre)) {
            printf("  [!] El nombre no puede estar vacio. Edicion cancelada.\n");
            pausarPantalla();
            continue;
        }

        int duplicado = 0;
        for (i = 0; i < numZonas; i++) {
            if (i != idx && strcmp(zonas[i].nombre, nuevoNombre) == 0) {
                duplicado = 1;
                break;
            }
        }

        if (duplicado) {
            printf("  [!] Error: Ya existe otra zona con el nombre '%s'.\n", nuevoNombre);
            pausarPantalla();
            continue;
        }

        char nombreAnterior[50];
        strcpy(nombreAnterior, zonas[idx].nombre);
        strcpy(zonas[idx].nombre, nuevoNombre);

        guardarDatos(zonas, numZonas, limitesOMS, maxZonasPermitidas);
        printf("  [EXITO] Se actualizo el nombre de la zona ID %d:\n", zonas[idx].id);
        printf("          '%s' -> '%s'\n", nombreAnterior, zonas[idx].nombre);
        pausarPantalla();
        break;
    } while (1);
}

void eliminarZona(Zona **zonas, int *numZonas, Contaminacion limitesOMS, int maxZonasPermitidas) {
    int opc = 0;
    int idx = -1;
    int i;

    if (*numZonas == 0) {
        limpiarPantalla();
        printf("\n----------------------------------------------------------\n");
        printf("  [!] No hay zonas registradas para eliminar.\n");
        printf("----------------------------------------------------------\n");
        pausarPantalla();
        return;
    }

    limpiarPantalla();
    printf("\n----------------------------------------------------------\n");
    printf("                 ELIMINAR UNA ZONA\n");
    printf("----------------------------------------------------------\n");
    printf("  Zonas disponibles:\n");
    mostrarZonasResumidas(*zonas, *numZonas);
    printf("  Seleccione ID de la zona a eliminar (1 a %d) o 0 para cancelar: ", *numZonas);
    opc = validarIntRango(0, *numZonas);

    if (opc == 0) {
        printf("\n  Operacion cancelada.\n");
        pausarPantalla();
        return;
    }

    idx = opc - 1;
    char nombreEliminado[50];
    strcpy(nombreEliminado, (*zonas)[idx].nombre);

    printf("\n  [ATENCION] ¿Esta seguro de que desea eliminar la zona '%s' con ID %d?\n", nombreEliminado, opc);
    printf("  Esta accion no se puede deshacer.\n");
    printf("  1: Si, eliminar permanentemente\n");
    printf("  2: No, cancelar\n");
    printf("  >> ");
    int confirmacion = validarIntRango(1, 2);

    if (confirmacion == 2) {
        printf("\n  Operacion cancelada.\n");
        pausarPantalla();
        return;
    }

    for (i = idx; i < *numZonas - 1; i++) {
        (*zonas)[i] = (*zonas)[i + 1];
        (*zonas)[i].id = i + 1; // Reasignar ID consecutivamente
    }

    (*numZonas)--;

    if (*numZonas > 0) {
        Zona *temp = realloc(*zonas, (*numZonas) * sizeof(Zona));
        if (temp != NULL) {
            *zonas = temp;
        }
    } else {
        free(*zonas);
        *zonas = NULL;
    }

    guardarDatos(*zonas, *numZonas, limitesOMS, maxZonasPermitidas);
    printf("\n  [EXITO] La zona '%s' fue eliminada permanentemente.\n", nombreEliminado);
    pausarPantalla();
}

void menuConfiguracion(Zona **zonas, int *numZonas, Contaminacion *limitesOMS, int *maxZonasPermitidas) {
    int opc = 0;
    do {
        limpiarPantalla();
        printf("\n----------------------------------------------------------\n");
        printf("                 CONFIGURACION DEL SISTEMA\n");
        printf("----------------------------------------------------------\n");
        printf("  Estado Actual del Sistema:\n");
        printf("    Limites OMS: CO2: %.0f | SO2: %.0f | NO2: %.0f | PM2.5: %.0f\n",
               limitesOMS->co2, limitesOMS->so2, limitesOMS->no2, limitesOMS->pm25);
        printf("    Tope Maximo de Zonas: %d | Registradas: %d\n", *maxZonasPermitidas, *numZonas);
        printf("----------------------------------------------------------\n");
        printf("  1. Configurar limites de contaminacion y tope de zonas\n");
        printf("  2. Editar/renombrar una zona existente\n");
        printf("  3. Eliminar una zona existente\n");
        printf("  4. Regresar al menu principal\n");
        printf("----------------------------------------------------------\n");
        printf(">> ");
        opc = validarIntRango(1, 4);

        switch (opc) {
        case 1:
            establecerLimites(limitesOMS, maxZonasPermitidas, *numZonas);
            guardarDatos(*zonas, *numZonas, *limitesOMS, *maxZonasPermitidas);
            pausarPantalla();
            break;
        case 2:
            editarZona(*zonas, *numZonas, *limitesOMS, *maxZonasPermitidas);
            break;
        case 3:
            eliminarZona(zonas, numZonas, *limitesOMS, *maxZonasPermitidas);
            break;
        case 4:
            break;
        }
    } while (opc != 4);
}

void menuMonitoreoConsultas(Zona *zonas, int numZonas, Contaminacion limitesOMS) {
    int opc = 0;
    do {
        limpiarPantalla();
        printf("\n----------------------------------------------------------\n");
        printf("                   MONITOREO Y CONSULTAS\n");
        printf("----------------------------------------------------------\n");
        printf("  1. Ver resumen de todas las zonas (Monitoreo Actual)\n");
        printf("  2. Consultar una zona especifica en detalle\n");
        printf("  3. Ver prediccion a 24 horas global (todas las zonas)\n");
        printf("  4. Buscar zona por nombre (coincidencia parcial)\n");
        printf("  5. Filtrar zonas que superan limites de la OMS\n");
        printf("  6. Regresar al menu principal\n");
        printf("----------------------------------------------------------\n");
        printf(">> ");
        opc = validarIntRango(1, 6);

        switch (opc) {
        case 1:
            limpiarPantalla();
            monitoreoActual(zonas, numZonas, limitesOMS);
            pausarPantalla();
            break;
        case 2:
            consultarZonaDetalle(zonas, numZonas, limitesOMS);
            pausarPantalla();
            break;
        case 3:
            limpiarPantalla();
            prediccion24h(zonas, numZonas, limitesOMS);
            pausarPantalla();
            break;
        case 4:
            buscarZonasPorNombre(zonas, numZonas);
            pausarPantalla();
            break;
        case 5:
            filtrarZonasExcedidas(zonas, numZonas, limitesOMS);
            pausarPantalla();
            break;
        case 6:
            break;
        }
    } while (opc != 6);
}

void consultarZonaDetalle(Zona *zonas, int numZonas, Contaminacion limitesOMS) {
    int idx;
    float fc;
    Contaminacion prom, pred;

    if (!verificarZonas(numZonas)) return;

    limpiarPantalla();
    printf("\n----------------------------------------------------------\n");
    printf("             CONSULTAR DETALLE DE ZONA\n");
    printf("----------------------------------------------------------\n");
    printf("  Zonas disponibles:\n");
    mostrarZonasResumidas(zonas, numZonas);
    printf("  Seleccione zona por ID (1 a %d): ", numZonas);
    idx = validarIntRango(1, numZonas) - 1;

    limpiarPantalla();
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

    if (z->count == 0) {
        printf("  [NOTA] No se registran mediciones previas en el historial.\n\n");
    } else {
        printf("  [INFO] Historico basado en %d mediciones registradas.\n\n", z->count);
    }

    printf("  %-25s %10.1f %10.1f %10.1f\n", "CO2", z->actual.co2, prom.co2, pred.co2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "SO2", z->actual.so2, prom.so2, pred.so2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "NO2", z->actual.no2, prom.no2, pred.no2);
    printf("  %-25s %10.1f %10.1f %10.1f\n", "PM2.5", z->actual.pm25, prom.pm25, pred.pm25);
    printf("  ----------------------------------------------------------\n");

    printf("\n  Alertas y Recomendaciones Activas:\n");
    imprimirAlertasGenerales(stdout, z, limitesOMS, pred);
    printf("----------------------------------------------------------\n");
}

void registrarAlertaLog(const char *nombreZona, const char *contaminante, float valor, float limite) {
    // Rotacion del log: si supera 50 KB, truncar y empezar de nuevo
    FILE *logCheck = fopen("alertas.log", "rb");
    if (logCheck != NULL) {
        fseek(logCheck, 0, SEEK_END);
        long tamano = ftell(logCheck);
        fclose(logCheck);
        if (tamano > 50 * 1024) {
            FILE *logReset = fopen("alertas.log", "w");
            if (logReset != NULL) {
                fprintf(logReset, "--- Log rotado (archivo anterior superaba 50 KB) ---\n");
                fclose(logReset);
            }
        }
    }

    FILE *logFile = fopen("alertas.log", "a");
    if (logFile == NULL) {
        printf("  [ERROR] No se pudo abrir/crear el archivo de log 'alertas.log'.\n");
        return;
    }
    time_t ahora = time(NULL);
    struct tm *infoTiempo = localtime(&ahora);
    char fechaHora[20];
    if (infoTiempo != NULL) {
        strftime(fechaHora, sizeof(fechaHora), "%Y-%m-%d %H:%M:%S", infoTiempo);
    } else {
        strcpy(fechaHora, "2026-06-25 00:00:00");
    }
    fprintf(logFile, "[%s] ALERTA - Zona: %s | Contaminante: %s | Valor actual: %.2f | Limite OMS: %.2f\n",
            fechaHora, nombreZona, contaminante, valor, limite);
    fclose(logFile);
}