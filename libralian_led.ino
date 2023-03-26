/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

uint8_t broadcastAddress1[] = {0x3C, 0x61, 0x05, 0x03, 0x6B, 0x74}; // maikan
uint8_t broadcastAddress2[] = {0x3C, 0x71, 0xBF, 0x0C, 0x45, 0xFC}; // winter

#define SCREEN_WIDTH 128 // pixel ความกว้าง
#define SCREEN_HEIGHT 64 // pixel ความสูง 
#define BUTTON_PIN 16
#define BUTTON_PIN2 4 

int lastState1 = HIGH; // the previous state from the input pin
int currentState1;    
int lastState2 = HIGH; // the previous state from the input pin
int currentState2;    

// กำหนดขาต่อ I2C กับจอ OLED
#define OLED_RESET     -1 //ขา reset เป็น -1 ถ้าใช้ร่วมกับขา Arduino reset
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int var = 0; // ตัวแปรสำหรับทดสอบแสดงผล
int oled = -1;
int room = 0;
String pass1 = "";
String pass2 = "";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  int x;
  String y;
}struct_message;

// Create a struct_message called myData
struct_message myData;
struct_message sentData;
// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;


// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2,};

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  // Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // // Update the structures with the new incoming data
  // boardsStruct[myData.id-1].x = myData.x;
  // boardsStruct[myData.id-1].y = myData.y;
  // Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  // Serial.printf("y value: %d \n", boardsStruct[myData.id-1].y);
  // Serial.println();
  if(myData.id == 1){
    room = 1;
    if(myData.x == 0){
      Serial.println("smoke"); 
      oled = 0;
      delay(5000);
      room = 0;
      oled = -1;    
    }
    else if (myData.x == 1){
      Serial.println("sound");
      oled = 1;
      delay(5000);
      room = 0;
      oled = -1;
    }
    else if (myData.x == 3){
      pass1 = myData.y;
      Serial.print("Room 1 password : ");
      Serial.println(pass1);
      room = 0;
    }
    else{
      Serial.println("people");
      oled = 2;
      delay(5000);
      room = 0;
      oled = -1;
    }    
  }
  else{
    room = 2;
    if(myData.x == 0){
      Serial.println("smoke"); 
      oled = 0;    
      delay(5000);
      oled = -1;
      room = 0; 
    }
    else if (myData.x == 1){
      Serial.println("sound");
      oled = 1;
      delay(5000);
      room = 0;
      oled = -1;
    }
    else if (myData.x == 3){
      pass2 = myData.y;
      Serial.print("Room 2 password : ");
      Serial.println(pass2);
      room = 0;
    }
    else{
      Serial.println("people");
      oled = 2;
      delay(5000);
      room = 0;
      oled = -1;
    }    
  }
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   pinMode(BUTTON_PIN2, INPUT_PULLUP);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("ArdinoAll OLED Start Work !!!");
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register second peer  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  // Init ESP-NOW

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  // memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  // peerInfo.channel = 0;  
  // peerInfo.encrypt = false;
}
 
void loop() {
  OLED.clearDisplay(); // ลบภาพในหน้าจอทั้งหมด
  OLED.setTextColor(WHITE, BLACK);  //กำหนดข้อความสีขาว ฉากหลังสีดำ
  OLED.setCursor(0, 0); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
  OLED.setTextSize(2); // กำหนดขนาดตัวอักษร
  OLED.print("Room : "); // แสดงผลข้อความ ALL
  if(room == 0){
    OLED.println("ALL");
  }
  else{
    OLED.println(room);
  }
  OLED.setCursor(0, 20);
  OLED.setTextSize(1);
  OLED.print("R1:");
  OLED.print(pass1);
  OLED.print(" R2:");
  OLED.println(pass2);
  OLED.setCursor(0, 40);
  OLED.setTextSize(2);
  if(oled == -1){
    OLED.println("Normal");
  }
  else if (oled == 0){
    OLED.println("Smoke");
  }
  else if (oled == 1){
    OLED.println("Too loud");
  }
  else{
    OLED.println("Many People");
  }
  //OLED.println(oled); // แสดงผลข้อความ ALL

  OLED.display(); // สั่งให้จอแสดงผล
   currentState1 = digitalRead(BUTTON_PIN);
   currentState2 = digitalRead(BUTTON_PIN2);
   //Serial.println(currentState2);

  if(lastState1 == LOW && currentState1 == HIGH){
    sentData.id = 1;
    sentData.x = random(0,50);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress1, (uint8_t *) &sentData, sizeof(sentData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }

    if(lastState2 == LOW && currentState2 == HIGH){
    sentData.id = 2;
    sentData.x = random(0,50);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress2, (uint8_t *) &sentData, sizeof(sentData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }

  // save the last state
  lastState1 = currentState1;
  lastState2 = currentState2;
  delay(1000);
  // delay(5000);  
}