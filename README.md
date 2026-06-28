# Sistema de Monitoreo y Prediccion de Contaminacion del Aire (UDLA)

Este proyecto consiste en un sistema de gestion y prediccion de calidad del aire para entornos urbanos implementado en C. Permite registrar diferentes zonas geograficas, ingresar niveles de contaminantes (CO2, SO2, NO2, PM2.5), monitorear estados actuales, evaluar promedios de los ultimos 30 dias, pronosticar niveles futuros integrando factores climaticos y generar alertas con sus correspondientes recomendaciones de salud publica y mitigacion.

El software cuenta con soporte para persistencia binaria de datos, asegurando que los registros historicos y las configuraciones de limites de calidad del aire (basados en directrices de la OMS) se mantengan entre ejecuciones.

---

## Funcionalidades Principales

El sistema opera mediante una interfaz de consola modular y robusta que ofrece las siguientes opciones:

1. **Configuracion del Sistema**
   * Ajuste de limites aceptables para cada contaminante (CO2, SO2, NO2, PM2.5) bajo directrices personalizables o de la OMS.
   * Modificacion del tope maximo de zonas permitidas en el sistema (hasta un maximo de 100).
   * Edicion de nombres de zonas existentes con validacion para evitar duplicados.

2. **Registro de Nueva Zona**
   * Incorporacion dinamica de zonas urbanas utilizando asignacion dinamica de memoria (`realloc`).
   * Validacion de nombres vacios o con espacios en blanco.
   * Registro de niveles iniciales de contaminantes y condiciones climaticas de hoy.
   * Simulacion e inicializacion automatica de un historial de 30 dias de mediciones mediante una formula de fluctuacion determinista.

3. **Actualizacion Diaria (Niveles y Clima)**
   * Actualizacion de mediciones actuales de contaminacion y variables climaticas para una zona especifica.
   * Insercion del nuevo registro en una cola circular de tamaño fijo (`MAX_DIAS = 30`).
   * Registro automatico de eventos en una bitacora de auditoria (`alertas.log`) si alguna medicion excede los limites establecidos por la OMS, incluyendo la marca de tiempo de la medicion.

4. **Monitoreo y Consultas**
   * **Monitoreo Actual:** Visualizacion resumida y detallada de la contaminacion actual en todas las zonas del sistema, comparandolas frente a los limites parametrizados.
   * **Detalle de Zona:** Reporte completo de una zona especifica que contrasta en una sola tabla los valores de Hoy, Promedio Historico y Prediccion.
   * **Buscador de Zonas:** Busqueda de zonas especificas por nombre a traves de una coincidencia parcial de subcadenas insensible a mayusculas y minusculas.
   * **Filtro de Excesos:** Lista exclusiva de las zonas que superan los limites recomendados hoy, indicando que contaminante especifico excede y por que margen.

5. **Prediccion a 24 Horas**
   * Estimacion de los niveles de contaminacion para el dia de mañana mediante un algoritmo ponderado.
   * Calculo de un **Factor Climatico (fc)** dinamico basado en variables de temperatura, velocidad del viento y porcentaje de humedad.
   * Generacion de alertas preventivas tempranas si la prediccion sobrepasa los limites OMS.

6. **Exportacion de Reportes**
   * Generacion de un archivo de texto plano (`reporte_calidad_aire.txt`) con el estado actual, promedios historicos de 30 dias, factores climaticos, predicciones y recomendaciones estructuradas de cada una de las zonas urbanas monitoreadas.

7. **Persistencia de Datos**
   * Guardado y carga automatica en formato binario mediante el archivo `datos.dat`, previniendo la perdida de informacion historica.
   * Inicializacion por defecto con 5 zonas clave de la ciudad de Quito, Ecuador ("Centro Historico", "Belisario", "Carapungo", "El Camal" y "Guamani") en caso de no detectarse el archivo de datos previo.

---

## Mecanicas y Algoritmos Detallados

### 1. Modelo de Datos y Estructuras en C

El sistema se fundamenta en cuatro estructuras de datos anidadas definidas en `funciones.h`:

* **`Clima`**: Almacena variables de temperatura (C), velocidad del viento (km/h) y porcentaje de humedad.
* **`Contaminacion`**: Contiene los valores cuantitativos de CO2, SO2, NO2 y PM2.5.
* **`RegistroHistorial`**: Estructura que asocia una marca de tiempo en formato de cadena (`fechaHora`) con una estructura de `Contaminacion`.
* **`Zona`**: Nodo principal que contiene:
  * Identificador unico (`id`) y `nombre` de la zona.
  * Un historial circular de tipo `RegistroHistorial` limitado a `MAX_DIAS = 30`.
  * Indices de control de la cola historica (`head` y `count`).
  * Estado de contaminacion `actual` y `climaActual`.

### 2. Algoritmo de Prediccion Ponderada y Climatica

El pronostico para las proximas 24 horas combina el promedio historico de la zona, el estado actual de contaminacion y el impacto meteorologico local.

#### Paso A: Calculo del Factor Climatico (fc)
La velocidad del viento actua como elemento de dispersion de particulas, mientras que la temperatura y la humedad altas tienden a concentrar o catalizar contaminantes:

$$fc = \text{clamp}\left(1.0 - \frac{\text{viento}}{200.0}, 0.5, 1.5\right) \times \text{clamp}(1.0 + (\text{temperatura} - 20.0) \times 0.01, 0.8, 1.2) \times \text{clamp}(1.0 + (\text{humedad} - 50.0) \times 0.005, 0.85, 1.15)$$

*El valor final de fc queda limitado (clamp) para evitar variaciones extremas e irreales.*

#### Paso B: Calculo de la Prediccion
Combina el nivel de hoy afectado por el factor climatico (con una ponderacion del 60%) y el promedio de los ultimos 30 dias (con una ponderacion del 40%):

$$\text{Prediccion} = (0.6 \times \text{Valor Actual} \times fc) + (0.4 \times \text{Promedio Historico})$$

### 3. Registro de Log de Alertas

Cuando un usuario registra una zona o actualiza los niveles de contaminacion y estos superan el limite de la OMS configurado en el sistema, se invoca la funcion `registrarAlertaLog`. Esta funcion abre el archivo `alertas.log` en modo "append" (`a`) e inserta una linea con el siguiente formato:

`[AAAA-MM-DD HH:MM:SS] ALERTA - Zona: <NombreZona> | Contaminante: <Tipo> | Valor actual: <Valor> | Limite OMS: <Limite>`

---

## Estructura del Codigo Fuente

El repositorio se compone de tres archivos principales:

* **`main.c`**: Punto de entrada del programa. Administra el bucle principal de ejecucion y la carga inicial del archivo de persistencia binaria (`datos.dat`).
* **`funciones.h`**: Contiene la declaracion de las estructuras de datos (`Clima`, `Contaminacion`, `RegistroHistorial`, `Zona`), la definicion de constantes y los prototipos de todas las funciones operativas del sistema.
* **`funciones.c`**: Implementacion de la logica de negocio, algoritmos matematicos, interaccion de consola (entradas/salidas) y manejo de archivos.

---

## Instrucciones de Compilacion y Ejecucion

### Requisitos Previos
* Un compilador de C compatible con el estandar C99 o superior (como GCC, Clang o MSVC).

### Compilacion mediante GCC (Linux/macOS/Windows MinGW)
Abra una terminal en la raiz del proyecto y ejecute:

```bash
gcc -Wall -Wextra -std=c99 main.c funciones.c -o sistema_calidad_aire
```

### Ejecucion del Sistema
En sistemas Windows:
```cmd
sistema_calidad_aire.exe
```

En sistemas Linux/macOS:
```bash
./sistema_calidad_aire
```
