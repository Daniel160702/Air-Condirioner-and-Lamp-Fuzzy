#include <RBDdimmer.h>

#define outputPin  12 
#define zerocross  5 

dimmerLamp dimmer(outputPin, zerocross);

#define ldrPin 2
const float gama = 0.7;
const float rl10 = 50;

void setup() {
  Serial.begin(9600); 
  dimmer.begin(NORMAL_MODE, ON); 
  Serial.println("Dimmer Program is starting...");
}

void loop() {
  // Baca nilai LDR dan hitung kecerahan
  int nilaiLDR = analogRead(ldrPin);
  nilaiLDR = map(nilaiLDR, 4095, 0, 1024, 0); //mengubah nilai pembacaan sensor LDR dari nilai ADC arduino menjadi nilai ADC ESP32
  float voltase = nilaiLDR / 1024.0 * 5.0;
  float resistansi = 2000.0 * voltase / (1.0 - voltase / 5.0);
  float kecerahan = pow(rl10 * 1e3 * pow(10, gama) / resistansi, (1.0 / gama));
  Serial.print("Kecerahan = ");
  Serial.println(kecerahan);
  
  // Kontrol dimmer berdasarkan nilai kecerahan
  if (kecerahan >= 0 && kecerahan < 150) {
    dimmer.setPower(90); // setPower(0-100%);
    Serial.println("lampValue -> 90%");
  } else if (kecerahan >= 150 && kecerahan < 200) {
    dimmer.setPower(50); // setPower(0-100%);
    Serial.println("lampValue -> 50%");
  } else if (kecerahan >= 200 && kecerahan <= 350) {
    dimmer.setPower(30); // setPower(0-100%);
    Serial.println("lampValue -> 30%");
  } else {
    dimmer.setPower(0); // Matikan lampu jika melebihi 350
    Serial.println("Lampu dimatikan");
  }
  
  delay(1000); // delay 1 detik
}
