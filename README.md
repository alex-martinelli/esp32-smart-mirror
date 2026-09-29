# Cornice digitale interattiva con ESP32

Una cornice digitale basata su **ESP32** che si accende quando qualcuno si avvicina.
Un sensore a ultrasuoni rileva la presenza, il display TFT mostra una foto (o due) presa da una scheda SD, con sopra una frase casuale e la lettura di temperatura e umidità.

## Come funziona

1. Ogni 500 ms il sensore a ultrasuoni misura la distanza.
2. Se la distanza è **inferiore a 65 cm**, il display si accende:
   - con probabilità 50% mostra **una foto orizzontale** a schermo intero (480x320);
   - con probabilità 50% mostra **due foto verticali affiancate** (240x320 ciascuna), sempre diverse tra loro.
3. Sopra le immagini vengono scritti:
   - una **frase casuale** in alto a sinistra (rosso, con a capo automatico);
   - **temperatura e umidità** (DHT11) in basso a destra.
4. Quando nessuno è più rilevato per **10 secondi**, lo schermo si spegne e il backlight va a zero.

> Nota: frase e dati del DHT11 vengono disegnati solo se la lettura del sensore va a buon fine.
> Se il DHT11 non risponde, compaiono solo le foto.

## Hardware

- ESP32 (es. ESP32 Dev Module)
- Display TFT SPI 480x320 (es. driver ILI9488 o ST7796 — **TODO: indicare il modello usato**)
- Modulo microSD (SPI)
- Sensore a ultrasuoni (tipo HC-SR04)
- Sensore DHT11 (temperatura e umidità)

## Collegamenti

| Componente | Segnale | Pin ESP32 |
|---|---|---|
| Sensore a ultrasuoni | TRIG | GPIO 26 |
| Sensore a ultrasuoni | ECHO | GPIO 35 |
| DHT11 | DATA | GPIO 32 |
| Backlight TFT (PWM) | LED | GPIO 2 |
| Modulo SD | SCK | GPIO 25 |
| Modulo SD | MISO | GPIO 33 |
| Modulo SD | MOSI | GPIO 13 |
| Modulo SD | CS | GPIO 4 |
| Display TFT | vedi sotto | **TODO** |

**Bus SPI separati.** La SD usa il bus **HSPI** con i pin sopra. Il display TFT usa il bus **VSPI** (pin di default dell'ESP32: SCK 18, MISO 19, MOSI 23), gestito da TFT_eSPI.

**Attenzione al pin ECHO.** L'ESP32 lavora a 3,3 V. Se il tuo sensore è un HC-SR04 alimentato a 5 V, l'uscita ECHO va abbassata con un partitore di tensione (ad esempio 1 kΩ + 2 kΩ) oppure si usa una versione a 3,3 V del sensore.

## Librerie necessarie

Da installare tramite il Gestore Librerie di Arduino IDE:

- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) (Bodmer)
- [JPEGDecoder](https://github.com/Bodmer/JPEGDecoder) (Bodmer)
- DHT sensor library (Adafruit) e Adafruit Unified Sensor

Incluse nel core ESP32: `SPI`, `SD`, `FS`.

**Core ESP32 richiesto:** versione **3.x** (lo sketch usa `ledcAttach()`, che non esiste nella 2.x).

## Configurazione del display (TFT_eSPI)

TFT_eSPI non legge i pin dallo sketch, ma dal file `User_Setup.h` nella cartella della libreria.
Nella cartella [`config/`](config/) di questo repository si trova una copia del file usato per questo progetto: sostituiscilo a quello della libreria (`Documenti/Arduino/libraries/TFT_eSPI/User_Setup.h`) prima di compilare.

> **TODO:** aggiungere `config/User_Setup.h` al repository.

Il display viene inizializzato con `setRotation(1)` (orizzontale, 480x320).

## Preparazione della scheda SD

Formatta la microSD in **FAT32** e copia nella **root** (senza sottocartelle) le immagini JPEG con questi nomi:

| Tipo | Nome file | Risoluzione | Quantità |
|---|---|---|---|
| Orizzontali | `1o.jpg` … `26o.jpg` | 480x320 | 26 |
| Verticali | `1v.jpg` … `28v.jpg` | 240x320 | 28 |

Per cambiare il numero di immagini modifica le costanti `NUM_ORIZZ` e `NUM_VERT` nello sketch.

Suggerimenti:
- usa JPEG **baseline** (non progressive);
- tieni i file leggeri: ogni immagine viene caricata interamente in RAM prima di essere decodificata (la dimensione e la RAM libera vengono stampate sul monitor seriale).

## Compilazione e caricamento

1. Clona il repository e apri `official/official.ino` con Arduino IDE.
2. Installa le librerie e il core ESP32 3.x indicati sopra.
3. Copia `User_Setup.h` come descritto nella sezione sul display.
4. Seleziona la scheda **ESP32 Dev Module** e la porta corretta.
5. Carica lo sketch. Apri il monitor seriale a **9600 baud** per vedere i log.

## Personalizzazione

| Cosa | Dove nello sketch |
|---|---|
| Distanza di attivazione (65 cm) | `gestisciDisplay()`: `distanza < 65` |
| Tempo di spegnimento (10 s) | costante `timeout` |
| Frasi mostrate | array `frasi[]` |
| Dimensione e colore del testo | `disegnaOverlay()` |
| Pin dei componenti | costanti in cima al file |

## Risoluzione dei problemi

- **Schermo bianco o nero:** controlla `User_Setup.h` (driver e pin del display) e il collegamento del backlight.
- **`ERRORE: SD non disponibile`:** verifica cablaggio, formattazione FAT32 e che i pin SD corrispondano a quelli sopra.
- **`ERRORE: impossibile aprire ...`:** il file non esiste o ha un nome diverso da quelli attesi.
- **`ERRORE: memoria insufficiente`:** l'immagine è troppo pesante, riducine dimensione o qualità.
- **`ERRORE: decodifica JPEG fallita`:** salva il file come JPEG baseline.
- **Distanze strane o sempre alte:** controlla l'alimentazione del sensore e il partitore sul pin ECHO.
- **Nessuna frase o temperatura a schermo:** il DHT11 non risponde, controlla il collegamento sul GPIO 32.

## Licenza

**TODO:** scegliere una licenza (ad esempio MIT) e aggiungere il file `LICENSE`.
