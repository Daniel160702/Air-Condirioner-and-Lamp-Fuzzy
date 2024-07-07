#include <ESP8266WiFi.h>
#include <espnow.h>

const int sensorPinIn = D5;    // Pin sensor infrared masuk
const int sensorPinOut = D7;   // Pin sensor infrared keluar
int previousStateIn = LOW;
int previousStateOut = LOW;
int peopleInside = 0; // Jumlah orang dalam ruangan
bool isSensorBlocked = false;
unsigned long lastSensorTime = 0; // Waktu terakhir deteksi sensor

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xBC, 0XFF, 0x4D, 0xF7, 0xFF, 0xEC};

// Structure example to send data
typedef struct struct_message {
  int peopleInside;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(sensorPinIn, INPUT);
  pinMode(sensorPinOut, INPUT);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  int currentStateIn = digitalRead(sensorPinIn);
  int currentStateOut = digitalRead(sensorPinOut);
  unsigned long currentTime = millis(); // Waktu sekarang

  // Deteksi sensor masuk
  if (!isSensorBlocked && currentStateIn == HIGH && previousStateIn == LOW) {
    // Sensor masuk mendeteksi
    peopleInside++; // Tambahkan jumlah orang dalam ruangan
    sendPeopleCount(); // Mengirim jumlah orang ke receiver
    isSensorBlocked = true;  // Blokir sensor keluar
    lastSensorTime = currentTime; // Catat waktu deteksi
  }

  // Deteksi sensor keluar
  if (!isSensorBlocked && currentStateOut == HIGH && previousStateOut == LOW) {
    // Sensor keluar mendeteksi
    if (peopleInside > 0) {
      peopleInside--; // Kurangi jumlah orang dalam ruangan jika ada yang keluar
      sendPeopleCount(); // Mengirim jumlah orang ke receiver
    }
    isSensorBlocked = true;  // Blokir sensor masuk
    lastSensorTime = currentTime; // Catat waktu deteksi
  }

  // Membuka sensor setelah 2 detik terakhir deteksi
  if (isSensorBlocked && currentTime - lastSensorTime >= 2000) {
    isSensorBlocked = false; // Buka kembali sensor setelah delay
  }

  // Update status sensor sebelumnya
  previousStateIn = currentStateIn;
  previousStateOut = currentStateOut;

  // Mengirim jumlah orang dalam ruangan setiap 2 detik
  if ((millis() - lastTime) > timerDelay) {
    sendPeopleCount();
    lastTime = millis();
  }
}

// Mengirim jumlah orang dalam ruangan melalui ESP-NOW
void sendPeopleCount() {
  myData.peopleInside = peopleInside;

  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}
