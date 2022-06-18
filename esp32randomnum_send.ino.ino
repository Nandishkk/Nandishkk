#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t Receiver_Address1[] = {0xAC, 0x0B, 0xFB, 0xDD, 0x84, 0xC5};

typedef struct struct_message {
  int integer;
  char character[8];
} struct_message;

struct_message message;

void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char address[18];
  Serial.print("Sent to: ");
  snprintf(address, sizeof(address), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(address);
  Serial.print(" status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(data_sent);
   
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  memcpy(peerInfo.peer_addr, Receiver_Address1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}
 
  void loop() { 
  message.integer = random(0,2);
  if(message.integer == 0)
  {
    strcpy(message.character, "Object1");
  }
    if(message.integer == 1)
  {
    strcpy(message.character, "Object2");
  }

 
  esp_err_t outcome = esp_now_send(0, (uint8_t *) &message, sizeof(struct_message));
   
  if (outcome == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(10000);
}
