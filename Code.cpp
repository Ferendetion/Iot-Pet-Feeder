/***************************************************
 * 1. Blynk Template & Auth
 **************************************************/
#define BLYNK_TEMPLATE_ID "" // Id Template
#define BLYNK_TEMPLATE_NAME "" // Nama Template
#define BLYNK_AUTH_TOKEN "" // Token
#define SERVO_PIN D2

/***************************************************
 * 2. Library
 **************************************************/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

/***************************************************
 * 3. WiFi Credentials
 **************************************************/
char ssid[] = ""; // Nama Wifi    
char pass[] = ""; // Password Wifi

// catatan: esp8266 tidak bisa disambungkan dengan wifi dengan bandwidth 5Ghz,, cukup sambungkan dengan wifi yang menggunakan 2,4Ghz

/***************************************************
 * 4. Servo
 **************************************************/
Servo servo1;        
int servoPin = D2;   // Pastikan servo terhubung ke D2

/***************************************************
 * 5. NTP Client
 *    - pool.ntp.org dengan offset GMT+7
 *    - update setiap 60 detik\
 **************************************************/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // Waktu menyesuakan tempat masing masing, disini saya menggunakan WIB

/***************************************************
 * 6. Alarm Times & Status
 **************************************************/
// Untuk testing, pastikan jam alarm disamakan dengan waktu nyata dari NTP
// Misal: atur alarm 1 menit ke depan dari waktu nyata saat testing
int pagiHour = 7;         // Ganti jamnya sesuai yang kalian inginkan
int pagiMinute = 59;     // Ganti jamnya sesuai yang kalian inginkan
bool isPagiActive = false; // Diatur lewat V2

int siangHour = 14;      // Ganti jamnya sesuai yang kalian inginkan
int siangMinute = 0;    // Ganti jamnya sesuai yang kalian inginkan
bool isSiangActive = false; // Diatur lewat V3

int soreHour = 16;      // Ganti jamnya sesuai yang kalian inginkan
int soreMinute = 0;    // Ganti jamnya sesuai yang kalian inginkan
bool isSoreActive = false; // Diatur lewat V4

/***************************************************
 * 7. BLYNK_WRITE - Handle Virtual Pins
 **************************************************/
// (a) V1: Manual Servo Control (buka/tutup servo)
BLYNK_WRITE(V1) {
  int angle = param.asInt(); 
  servo1.write(angle);  
  // Kirim nilai angle ke V0 untuk indikator/gauge
  Blynk.virtualWrite(V0, angle);
  Serial.print("Manual servo angle: ");
  Serial.println(angle);
}

// (b) V2: Alarm Pagi ON/OFF
BLYNK_WRITE(V2) {
  int state = param.asInt();
  isPagiActive = (state == 1);
  Serial.print("Alarm Pagi: ");
  Serial.println(isPagiActive ? "Aktif" : "Nonaktif");
}

// (c) V3: Alarm Siang ON/OFF
BLYNK_WRITE(V3) {
  int state = param.asInt();
  isSiangActive = (state == 1);
  Serial.print("Alarm Siang: ");
  Serial.println(isSiangActive ? "Aktif" : "Nonaktif");
}

// (d) V4: Alarm Sore ON/OFF
BLYNK_WRITE(V4) {
  int state = param.asInt();
  isSoreActive = (state == 1);
  Serial.print("Alarm Sore: ");
  Serial.println(isSoreActive ? "Aktif" : "Nonaktif");
}

/***************************************************
 * BLYNK_CONNECTED - Sinkronisasi status Virtual Pin
 **************************************************/
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2, V3, V4);
}

/***************************************************
 * 8. Setup
 **************************************************/
void setup() {
  Serial.begin(9600); // Pastikan gunakan serial begin di 9600 pada serial monitor
  Serial.println("Booting...");
  
  // Inisialisasi Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Menghubungkan ke Blynk...");
  
  // Inisialisasi servo
  servo1.attach(servoPin);
  
  // Mulai NTP
  timeClient.begin();
  timeClient.update();
  Serial.println("NTP Client sudah mulai.");
}

/***************************************************
 * 9. Loop
 **************************************************/
void loop() {
  Blynk.run();
  timeClient.update();

  // Ambil waktu saat ini
  int currentHour   = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  // Debug: tampilkan waktu saat ini setiap loop
  Serial.print("Waktu NTP: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.println(currentSecond);

  // Cek Alarm Pagi, gunakan range detik agar tidak terlewat (misal currentSecond < 2)
  if (isPagiActive && currentHour == pagiHour && currentMinute == pagiMinute && currentSecond < 2) {
    feedNow("Pagi");
  }

  // Cek Alarm Siang
  if (isSiangActive && currentHour == siangHour && currentMinute == siangMinute && currentSecond < 2) {
    feedNow("Siang");
  }

  // Cek Alarm Sore
  if (isSoreActive && currentHour == soreHour && currentMinute == soreMinute && currentSecond < 2) {
    feedNow("Sore");
  }

  delay(1000); // Perulangan tiap 1 detik
}

/***************************************************
 * 10. feedNow() - Fungsi untuk memberi makan
 **************************************************/
void feedNow(String waktu) {
  Serial.print("feedNow() dipanggil untuk ");
  Serial.println(waktu);

  // Buka Servo (misalnya ke 180 derajat) selama 2 detik
  servo1.write(180);
  Blynk.virtualWrite(V0, 180);
  Serial.println("Servo dibuka (180°) selama 2 detik...");
  
  delay(3000); // Tahan 3 detik

  // Tutup servo ke 0 derajat
  servo1.write(0);
  Blynk.virtualWrite(V0, 0);
  Serial.println("Servo ditutup (0°).");

  // Delay agar alarm tidak dipicu ulang dalam 1 menit
  delay(60000);
}
