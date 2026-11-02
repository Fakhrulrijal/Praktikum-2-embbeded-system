#include <WiFi.h>
#include "DHTesp.h"

// Definisi Pin Periferal
const int DHT_PIN = 15;
const int LED_NORMAL = 2;
const int LED_ALARM = 4;
const int BUZZER_PIN = 5;

// Batas Kritis (Threshold)
const float AMBANG_SUHU_KRITIS = 35.0; // Derajat Celcius

// Konfigurasi Virtual WiFi Wokwi (Opsional/Non-blocking)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

DHTesp dhtSensor;

// Variabel Penjadwalan Waktu (Non-blocking Timer)
unsigned long waktuSebelumnya = 0;
const unsigned long intervalBaca = 2000; // Baca sensor setiap 2 detik

void setup() {
  Serial.begin(115200);
  Serial.println("--- Inisialisasi Node IoT ESP32 (Stand-alone Mode) ---");
  pinMode(LED_NORMAL, OUTPUT);
  pinMode(LED_ALARM, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Inisialisasi Sensor DHT22
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Inisialisasi Wi-Fi Non-blocking dengan Timeout (Aman untuk Akun Gratis)
  Serial.print("Memeriksa koneksi Wi-Fi: ");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(200);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Wi-Fi Connected!] IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[Mode Offline / Stand-alone] Melanjutkan simulasi tanpa gateway internet.");
  }
}

void loop() {
  unsigned long waktuSekarang = millis();

  // Jalankan pembacaan sensor setiap 2 detik (Non-blocking) 
  if (waktuSekarang - waktuSebelumnya >= intervalBaca) {
    waktuSebelumnya = waktuSekarang;

    // Ambil data dari sensor DHT22 
TempAndHumidity data = dhtSensor.getTempAndHumidity();

// Validasi pembacaan sensor 
    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("[Error] Gagal membaca data dari sensor DHT22!");
      return;
    }

// 2. Baca nilai RSSI (Gunakan nilai riil jika terkoneksi, atau nilai simulasi jika offline) 
    int rssi = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -65; // Mock RSSI jika offline
    
// Tampilkan data pembacaan sensor beserta nilai RSSI 
Serial.printf("[Sensor Data] Suhu: %.2f °C | Kelembaban: %.2f %% | RSSI: %d dBm\n", 
                  data.temperature, data.humidity, rssi);

    // Logika Respon Real-Time (Aktuator Lokal & serial log)
    if (data.temperature > AMBANG_SUHU_KRITIS) {
      digitalWrite(LED_ALARM, HIGH);
      digitalWrite(LED_NORMAL, LOW);
      tone(BUZZER_PIN, 1000); // Bunyikan alarm 1kHz
      Serial.println(">>> PERINGATAN: Suhu Melebihi Batas Kritis! <<<");
    } else {
      digitalWrite(LED_ALARM, LOW);
      digitalWrite(LED_NORMAL, HIGH);
      noTone(BUZZER_PIN);     // Matikan buzzer
      Serial.println("Status: Lingkungan Normal & Terkendali.");
    }
  }

  // Task background lain dapat berjalan di sini tanpa terhalang delay()

}

