#include <Arduino.h>

// ===== Matrix Pushbutton 3x3 -> Matrix LED 3x3 (ESP32, framework Arduino) =====
// Baris tombol  : OUTPUT (diaktifkan LOW satu per satu saat scanning)
// Kolom tombol  : INPUT_PULLUP (pull-up internal, tanpa resistor eksternal)
// LED           : satu pin per LED (resistor 220 ohm), posisi sama dengan tombol

const uint8_t ROW_PINS[3] = {16, 17, 18};
const uint8_t COL_PINS[3] = {19, 21, 22};
const uint8_t LED_PINS[3][3] = {
    {13, 14, 25},
    {26, 27, 32},
    {33, 4, 23}};

const uint32_t DEBOUNCE_MS = 30; // waktu minimum sinyal harus stabil

bool rawState[3][3];       // pembacaan mentah terakhir (true = ditekan)
bool stableState[3][3];    // status stabil, sudah lolos debouncing
uint32_t lastChange[3][3]; // waktu terakhir pembacaan mentah berubah

void setup()
{
  Serial.begin(115200);

  for (int r = 0; r < 3; r++)
  {
    pinMode(ROW_PINS[r], INPUT); // baris dibiarkan high-Z sampai giliran di-scan
  }
  for (int c = 0; c < 3; c++)
  {
    pinMode(COL_PINS[c], INPUT_PULLUP); // tidak ditekan = HIGH, ditekan = LOW
  }
  for (int r = 0; r < 3; r++)
  {
    for (int c = 0; c < 3; c++)
    {
      pinMode(LED_PINS[r][c], OUTPUT);
      digitalWrite(LED_PINS[r][c], LOW);
      rawState[r][c] = false;
      stableState[r][c] = false;
      lastChange[r][c] = 0;
    }
  }
  Serial.println("Matrix pushbutton 3x3 siap");
}

void loop()
{
  uint32_t now = millis();

  for (int r = 0; r < 3; r++)
  {
    // Aktifkan baris r: tarik ke LOW, baris lain tetap high-Z
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(10); // beri waktu sinyal settle sebelum dibaca

    for (int c = 0; c < 3; c++)
    {
      bool raw = (digitalRead(COL_PINS[c]) == LOW); // LOW = kontak tertutup

      // Pembacaan mentah berubah -> mulai hitung ulang waktu stabil
      if (raw != rawState[r][c])
      {
        rawState[r][c] = raw;
        lastChange[r][c] = now;
      }

      // Sudah stabil minimal DEBOUNCE_MS dan berbeda dari status sah -> terima
      if ((now - lastChange[r][c]) >= DEBOUNCE_MS && stableState[r][c] != rawState[r][c])
      {
        stableState[r][c] = rawState[r][c];
        digitalWrite(LED_PINS[r][c], stableState[r][c] ? HIGH : LOW);
        Serial.printf("Tombol (%d,%d) %s -> LED (%d,%d) %s\n",
                      r + 1, c + 1, stableState[r][c] ? "DITEKAN" : "DILEPAS",
                      r + 1, c + 1, stableState[r][c] ? "ON" : "OFF");
      }
    }

    pinMode(ROW_PINS[r], INPUT); // lepas baris r sebelum scan baris berikutnya
  }
}
