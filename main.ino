#include <DHT.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include <SD.h>
#include <FS.h>
#include <JPEGDecoder.h>

TFT_eSPI tft = TFT_eSPI();

// ==========================================
// Bus SPI dedicato per la SD (HSPI), separato dal TFT (VSPI di default)
// ==========================================
SPIClass sdSPI(HSPI);
#define SD_SCK   25
#define SD_MISO  33
#define SD_MOSI  13
#define SD_CS     4

// Definizione dei pin
const int dhtPin = 32;
const int trigPin = 26;
const int echoPin = 35;
#define ledPin 2   // stesso pin, ora anche backlight TFT via PWM

unsigned long lastDetectedTime = 0;
const unsigned long timeout = 10000;  // 10 secondi

DHT dht(dhtPin, DHT11);

bool displayOn = false;

const int NUM_ORIZZ = 26;  // immagini "Xo.jpg"
const int NUM_VERT  = 28;  // immagini "Xv.jpg"

// ==========================================
// Frasi casuali
// ==========================================
const char* frasi[] = {
  "benvenuto"
};
const int NUM_FRASI = sizeof(frasi) / sizeof(frasi[0]);

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);
  delay(1000);
  Serial.println(F("\n--- Avvio Sistema ---"));

  dht.begin();
  randomSeed(analogRead(34));

  // Configurazione PWM per il backlight (massima luminosità di default)
  ledcAttach(ledPin, 5000, 8); // pin, frequenza 5kHz, risoluzione 8 bit (0-255)
  ledcWrite(ledPin, 0);        // parte spento, si accende in accendi()

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  Serial.println(F("Display inizializzato."));

  Serial.println(F("--- Fine Avvio. Inizio letture ---"));
}

void loop() {
  float distanza = calcolaDistanza();
  gestisciDisplay(distanza);
  delay(500);
}

float calcolaDistanza() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float durata = pulseIn(echoPin, HIGH);
  float distanza = durata * 0.034 / 2;
  Serial.println(distanza);
  return distanza;
}

void gestisciDisplay(float distanza) {
  if (distanza < 65) {
    if (!displayOn) {
      accendi();
      displayOn = true;
    }
    lastDetectedTime = millis();
  } else {
    if (millis() - lastDetectedTime >= timeout) {
      if (displayOn) {
        spegni();
        displayOn = false;
      }
    }
  }
}

// Accende il display: sceglie a caso tra 1 orizzontale o 2 verticali affiancate
void accendi() {
  ledcWrite(ledPin, 255); // backlight al massimo

  bool orizzontale = random(0, 2) == 0; // 50% orizzontale, 50% verticale

  if (orizzontale) {
    int n = random(1, NUM_ORIZZ + 1);
    String nomeFile = "/" + String(n) + "o.jpg";
    Serial.print(F("Modalita: orizzontale - "));
    Serial.println(nomeFile);

    caricaEDisegnaUnaImmagine(nomeFile.c_str());
  } else {
    int a = random(1, NUM_VERT + 1);
    int b;
    do {
      b = random(1, NUM_VERT + 1);
    } while (b == a); // evita di scegliere due volte la stessa

    String fileA = "/" + String(a) + "v.jpg";
    String fileB = "/" + String(b) + "v.jpg";
    Serial.print(F("Modalita: verticale - "));
    Serial.print(fileA);
    Serial.print(F(" + "));
    Serial.println(fileB);

    caricaEDisegnaDueImmaginiVerticali(fileA.c_str(), fileB.c_str());
  }

  // Lettura sensori DHT e overlay testo
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  } else {
    disegnaOverlay(t, h);
    Serial.print(F("Umidità: "));
    Serial.print(h);
    Serial.print(F("%  Temperatura: "));
    Serial.println(t);
  }

  Serial.println(F("Display acceso"));
}

void spegni() {
  Serial.println(F("Display spento"));
  tft.fillScreen(TFT_BLACK);
  ledcWrite(ledPin, 0); // backlight spento
}

// ==========================================
// Legge un file dalla SD in un buffer RAM (SD deve essere già aperta)
// ==========================================
uint8_t* leggiFileInRAM(const char *filename, size_t &sizeOut) {
  File f = SD.open(filename, FILE_READ);
  if (!f) {
    Serial.print(F("ERRORE: impossibile aprire "));
    Serial.println(filename);
    sizeOut = 0;
    return nullptr;
  }

  sizeOut = f.size();

  // DEBUG: dimensione file e memoria disponibile
  Serial.print(F("File: "));
  Serial.print(filename);
  Serial.print(F(" - Dimensione: "));
  Serial.print(sizeOut);
  Serial.print(F(" byte - RAM libera: "));
  Serial.print(ESP.getFreeHeap());
  Serial.println(F(" byte"));

  uint8_t *buffer = (uint8_t *)malloc(sizeOut);
  if (!buffer) {
    Serial.println(F("ERRORE: memoria insufficiente"));
    f.close();
    sizeOut = 0;
    return nullptr;
  }

  f.read(buffer, sizeOut);
  f.close();
  return buffer;
}

// Carica e disegna UNA immagine orizzontale a schermo intero (480x320)
void caricaEDisegnaUnaImmagine(const char *filename) {
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  bool sdOk = SD.begin(SD_CS, sdSPI, 1000000);

  if (!sdOk) {
    Serial.println(F("ERRORE: SD non disponibile"));
    return;
  }

  size_t size;
  uint8_t *buffer = leggiFileInRAM(filename, size);

  SD.end();
  sdSPI.end();

  if (buffer) {
    tft.fillScreen(TFT_BLACK);
    if (JpegDec.decodeArray(buffer, size)) {
      renderJPEG(0, 0);
    } else {
      Serial.println(F("ERRORE: decodifica JPEG fallita"));
    }
    free(buffer);
  }
}

// Carica e disegna DUE immagini verticali affiancate (240x320 ciascuna)
// Un buffer alla volta, per dimezzare il picco di RAM richiesta
void caricaEDisegnaDueImmaginiVerticali(const char *fileA, const char *fileB) {
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  bool sdOk = SD.begin(SD_CS, sdSPI, 1000000);

  if (!sdOk) {
    Serial.println(F("ERRORE: SD non disponibile"));
    return;
  }

  // Legge e libera SUBITO l'immagine A, prima di leggere B
  size_t sizeA;
  uint8_t *bufferA = leggiFileInRAM(fileA, sizeA);

  tft.fillScreen(TFT_BLACK);

  if (bufferA) {
    if (JpegDec.decodeArray(bufferA, sizeA)) {
      renderJPEG(0, 0);
    } else {
      Serial.println(F("ERRORE: decodifica JPEG fallita (A)"));
    }
    free(bufferA);
    bufferA = nullptr; // libera la RAM subito, prima di leggere B
  }

  // Ora legge B (con la RAM di A già liberata)
  size_t sizeB;
  uint8_t *bufferB = leggiFileInRAM(fileB, sizeB);

  SD.end();
  sdSPI.end();

  if (bufferB) {
    if (JpegDec.decodeArray(bufferB, sizeB)) {
      renderJPEG(240, 0);
    } else {
      Serial.println(F("ERRORE: decodifica JPEG fallita (B)"));
    }
    free(bufferB);
    bufferB = nullptr;
  }
}

// Disegna la frase casuale in alto a sinistra, temp/umidità in alto a destra
// Disegna la frase casuale in alto a sinistra, temp/umidità in basso a destra
void disegnaOverlay(float t, float h) {
  // Frase casuale, con andata a capo automatica, testo grande
  tft.setTextSize(3);
  int indice = random(0, NUM_FRASI);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  stampaConAndataACapo(frasi[indice], 5, 5, 25); // meno caratteri per riga, dato il testo più grande

  // Temperatura e umidità in basso a destra, testo più piccolo
  tft.setTextSize(2);

  String tempText = String(t, 1) + " C";
  String humText = String(h, 1) + " %";

  int textWidthTemp = 6 * 2 * tempText.length();
  int textWidthHum = 6 * 2 * humText.length();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(tft.width() - textWidthTemp - 5, tft.height() - 45);
  tft.print(tempText);

  tft.setCursor(tft.width() - textWidthHum - 5, tft.height() - 25);
  tft.print(humText);
}

// Stampa un testo andando a capo automaticamente ogni N caratteri, senza spezzare le parole
void stampaConAndataACapo(const char *testo, int x, int y, int maxCharPerRiga) {
  String s = String(testo);
  int lineHeight = 28; // era 20, corretto per textSize(3) — non più per textSize(2)
  int riga = 0;
  int start = 0;

  while (start < (int)s.length()) {
    int fine = start + maxCharPerRiga;
    if (fine >= (int)s.length()) {
      fine = s.length();
    } else {
      int ultimoSpazio = s.lastIndexOf(' ', fine);
      if (ultimoSpazio > start) fine = ultimoSpazio;
    }

    String linea = s.substring(start, fine);
    linea.trim();

    tft.setCursor(x, y + riga * lineHeight);
    tft.print(linea);

    start = fine;
    while (start < (int)s.length() && s[start] == ' ') start++;
    riga++;
  }
}

// Funzione che copia i blocchi (MCU) decodificati sul display
void renderJPEG(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  uint32_t min_w = min((uint32_t)mcu_w, (uint32_t)(max_x % mcu_w == 0 ? mcu_w : max_x % mcu_w));
  uint32_t min_h = min((uint32_t)mcu_h, (uint32_t)(max_y % mcu_h == 0 ? mcu_h : max_y % mcu_h));

  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  max_x += xpos;
  max_y += ypos;

  while (JpegDec.read()) {
    pImg = JpegDec.pImage;

    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    if (win_w != mcu_w) {
      uint16_t *cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++) {
        memcpy(cImg, pImg + h * mcu_w, win_w << 1);
        cImg += win_w;
      }
    }

    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height()) {
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    } else if ((mcu_y + win_h) >= tft.height()) {
      JpegDec.abort();
    }
  }

  tft.setSwapBytes(swapBytes);
}