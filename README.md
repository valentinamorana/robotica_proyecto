# Light Show Interactivo

![Arduino](https://img.shields.io/badge/Arduino-UNO-00979D) ![FastLED](https://img.shields.io/badge/FastLED-WS2812B-E7352C) ![Android](https://img.shields.io/badge/Android-Java%20%C2%B7%20API%2026-3DDC84) ![Bluetooth](https://img.shields.io/badge/Bluetooth-HC--05-0082FC)

Sistema IoT de escritorio: una tira LED WS2812B reacciona al sonido ambiente y se controla desde una app Android por Bluetooth. Proyecto final de la materia **Robótica** (Universidad Abierta Interamericana, 2025).

**Autora:** Valentina Morana · **Videos:** [Robot respondiendo al sonido](https://youtu.be/arAJ2jzalk4) · [Cómo funciona la app](https://youtu.be/ibyro03ZqMY) · [Firmware en Arduino IDE](https://youtu.be/Q-ultrsjFdM) · [Playlist completa](https://www.youtube.com/playlist?list=PLV7U1PnDNScc9lIdke7sm5rW4sVpfUwcc)

---

## Funcionalidades

| Área | Qué resuelve |
|------|--------------|
| **Modo sonido** | Color y brillo reaccionan al volumen del ambiente (audio-rítmico RGB), con umbral configurable |
| **Modo manual** | Color fijo y brillo global elegidos desde la app o el Monitor Serie |
| **Arcoíris** | Animación cromática en toda la tira o solo en la zona seleccionada |
| **Selección de zona** | Izquierda, derecha, centro o todas las secciones de la tira |
| **Control IoT** | App Android que envía modos, colores, brillo y dirección por Bluetooth (HC-05) |
| **Estado seguro** | Las luces se apagan cuando no hay estímulo de sonido en modo sonido |

---

## Arquitectura

Sensado, razonamiento y actuación en un dispositivo de escritorio:

| Etapa | Elemento | Función |
|-------|----------|---------|
| **Sensado** | Micrófono KY-037 (pin A0) | Mide el nivel sonoro del ambiente |
| **Razonamiento** | Arduino UNO | Interpreta el sonido y los comandos, y decide el efecto |
| **Actuación** | Tira WS2812B (pin D6) | Emite luz, color y animaciones |
| **Control IoT** | HC-05 (D2/D3) + app Android | Envía modos, colores, brillo y dirección |

La app del celular cumple el rol de nodo de control: como la estación central que coordina una flota de drones, le indica al conjunto de LEDs qué hacer.

### Hardware

- Arduino UNO R3
- Tira WS2812B (el firmware está configurado para 59 LEDs)
- Módulo Bluetooth HC-05
- Micrófono KY-037
- Fuente de 5 V para tiras largas (~60 mA por LED a máximo brillo)
- Protoboard, jumpers y GND común entre módulos

```
WS2812B   DIN  → D6        VCC → 5 V (o fuente externa)   GND → GND
HC-05     TX   → D2        RX  → D3                        VCC → 5 V   GND → GND
Micrófono AO   → A0        VCC → 5 V                       GND → GND
```

> Si la tira tiene más de ~30 LEDs, alimentala con una fuente externa de 5 V y conectá su GND con el GND del Arduino.

---

## Tecnologías

| Componente | Tecnología |
|------------|------------|
| Firmware | Arduino C++ · librería FastLED · SoftwareSerial |
| App | Java · Android Studio · Gradle (AGP) · AppCompat |
| Comunicación | Bluetooth clásico (RFCOMM, SPP) con HC-05 |
| Plataforma Android | compileSdk 34 · minSdk 26 · Java 11 |

---

## Estructura del repositorio

```
robotica_proyecto/
├── Arduino/
│   └── Arduino_all_colors_V2/     # Firmware: modos, dirección, arcoíris y sonido
├── AndroidStudio/
│   └── LEDController/             # Proyecto Gradle completo (abrir esta carpeta en Android Studio)
│       └── app/src/main/java/com/example/ledcontroller/MainActivity.java
├── app-debug.apk                  # APK de debug listo para instalar
├── CH34x_driver.zip               # Driver USB-serial para chips CH340/CH34x (Windows)
├── Guia_de_despliegue/            # Guías paso a paso
│   ├── Guia_Arduino.txt           # Circuito, firmware y HC-05
│   └── Guia_Android.txt           # App Android (Android Studio)
```

---

## Instalación

### 1. Firmware (Arduino)

1. Instalá el **Arduino IDE** y la librería **FastLED** (*Sketch → Include Library → Manage Libraries → "FastLED"*).
2. **Driver CH34x (solo Windows):** muchos Arduino UNO clones usan un chip USB-serial CH340/CH341/CH34x en lugar del chip original. Si al conectar el Arduino no aparece ningún puerto COM en el Administrador de dispositivos, descomprimí e instalá el driver incluido en [`CH34x_driver.zip`](CH34x_driver.zip) y reiniciá el Arduino IDE.
3. Abrí `Arduino/Arduino_all_colors_V2/Arduino_all_colors_V2.ino`.
4. Ajustá `NUM_LEDS` a la cantidad real de LEDs de tu tira.
5. Compilá y subí el sketch.

Detalle en [`Guia_Arduino.txt`](Guia_de_despliegue/Guia_Arduino.txt).

### 2. Bluetooth (HC-05)

- Nombre sugerido: `LEDController` · PIN por defecto: `1234`
- Emparejá el HC-05 desde Android y anotá su dirección MAC.

### 3. App Android

- **Instalar el APK:** [`app-debug.apk`](app-debug.apk) es un build de debug que se instala en cualquier celular Android (hay que habilitar la instalación de orígenes desconocidos). **Importante:** este APK no tiene la MAC de ningún HC-05 (usa el placeholder `XX:XX:XX:XX:XX:XX`), así que al tocar "Conectar" va a avisar que hay que revisar `DEVICE_ADDRESS`. Para conectarlo a tu módulo tenés que compilar el proyecto con tu MAC, como se explica abajo.
- **Descargar el proyecto como zip:** en la [Release v1.0](https://github.com/valentinamorana/robotica_proyecto/releases/tag/v1.0) está `LEDController_ui_makeover.zip`, listo para descomprimir y abrir en Android Studio.
- **Compilar desde código (necesario para conectar):** abrí `AndroidStudio/LEDController` en Android Studio y poné la MAC de tu HC-05 en la constante `DEVICE_ADDRESS` de `MainActivity.java` (formato `XX:XX:XX:XX:XX:XX`). Después compilá y ejecutá en tu dispositivo. Cada persona necesita su propia MAC, porque es la de su módulo.
- **Sobre datos personales:** el repo no contiene la MAC de la autora. Sí quedó en versiones anteriores del historial de git, por lo que no se puede borrar sin reescribir el historial.

Detalle en [`Guia_Android.txt`](Guia_de_despliegue/Guia_Android.txt).

---

## Comandos Bluetooth / Serial

El firmware acepta comandos por Bluetooth o por el Monitor Serie a **9600 baudios**. Mayúsculas, espacios y `:` / `=` se normalizan automáticamente.

| Comando | Efecto |
|---|---|
| `RED`, `GREEN`, `BLUE`, `WHITE`, `YELLOW`, `CYAN`, `MAGENTA`, `PINK` | Color fijo (también `AMARILLO`, `CIAN` y `ROSA` en español) |
| `BRIGHT:0..255` | Brillo global |
| `LEFT` · `RIGHT` · `CENTER` · `ALL` | Zona de la tira donde se aplica el efecto |
| `SOUND ON` / `SOUND OFF` | Activa / desactiva el modo sonido |
| `SOUND STYLE BRIGHT` / `SOUND STYLE HUE` | Estilo del modo sonido cuando no es audio puro |
| `THRESH:0..1023` | Umbral de sensibilidad del micrófono |
| `RAINBOW` · `RAINBOW DIR` | Arcoíris en toda la tira o solo en la zona elegida |
| `TEST BARS` · `DIR?` · `MODE?` · `HELP` | Diagnóstico y ayuda |

Ejemplo desde el Monitor Serie: `CENTER` → `GREEN` → `BRIGHT:120`.

---

## Próximos pasos

- **Efecto enjambre:** sincronizar varias tiras LED como unidades de un mismo show.
- **Wi-Fi con ESP32:** reemplazar el HC-05 para ampliar la conectividad IoT.
- **Música procesada por IA:** coreografías que analizan el audio y generan respuestas visuales.

---

## Documentación

- **Tesis (PDF):** [Trabajo Final de Robótica](docs/Tesis_Morana_Valentina.pdf). Incluye la investigación sobre mapping drones y el desarrollo completo del prototipo.

## Marco teórico

El proyecto nace de una investigación sobre *mapping drones* y shows de luces autónomos: robótica aérea, swarm robotics, IoT, IA aplicada a la planificación y protocolos de seguridad. El prototipo traslada esos principios a una escala educativa y de escritorio.

---

## Agradecimientos

A **Martín Morana**, por el acompañamiento técnico durante el diseño, las pruebas y las mejoras del sistema.

---

## Licencia

Este proyecto está bajo la [Licencia MIT](LICENSE). Podés usar, copiar, modificar y redistribuir el código, incluso con fines comerciales, siempre que conserves el aviso de copyright. El software se entrega sin garantías.
