#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Servo.h>

#include <esp_now.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128 // pixel ความกว้าง
#define SCREEN_HEIGHT 64 // pixel ความสูง 

// กำหนดขาต่อ I2C กับจอ OLED
#define OLED_RESET     -1 //ขา reset เป็น -1 ถ้าใช้ร่วมกับขา Arduino reset
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x28, 0x18, 0xC0};
//---------PIR----------------
int irPin1=34;
int irPin2=35;
int count=0;
boolean state1 = true;
boolean state2 = true;
boolean insideState = false;
boolean outsideIr=false;
boolean isPeopleExiting=false;
int i=1;
int state_button =0;
int statepass = 1;


//--------NUMPAD-------------

#define ROW_NUM 4     // four rows
#define COLUMN_NUM 4  // four columns
#define SERVO_PIN 13
#define button 32
// #define OLED_RESET 16

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROW_NUM] = { 19, 18, 5, 17 };     // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = { 2, 0, 4, 16 };  // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

String password = String(random(1000, 9999));  // change your password here
String input_password;

Servo servoMotor;

int pos = 0;

int lastState = HIGH;

typedef struct struct_message {
    int id; // must be unique for each sender board
    int x;
    String y;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


//---------SERVO-------------




void setup() {
  Serial.begin(9600);
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
    // Serial.println("SSD1306 allocation failed");
  } else {
    // Serial.println("ArdinoAll OLED Start Work !!!");
  }
  //---------PIR----------
  Serial.begin(9600);
  pinMode(irPin1, INPUT);
  pinMode(irPin2, INPUT);

  //---------NUMPAD---------
  
  Serial.println(password);
  Serial.println("Enter the 4-digit code starting with * and ending with #.");
  input_password.reserve(32);  // maximum input characters is 33, change if needed
  servoMotor.attach(SERVO_PIN);

  pinMode(button, INPUT_PULLUP); // config GIOP21 as input pin and enable the internal pull-up resistor

  //---------SERVO----------


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  if(statepass = 1){
    myData.id = 1;
    myData.x = 3;
    myData.y = password; // password status

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent Password with success");
      statepass = 0;
    }
    else {
      Serial.println("Error sending the data");
    }

  }
}

void loop() {
  OLED.clearDisplay(); // ลบภาพในหน้าจอทั้งหมด
  OLED.setTextColor(WHITE);  //กำหนดข้อความสีขาว ฉากหลังสีดำ
  OLED.setCursor(0, 0); // กำหนดตำแหน่ง x,y ที่จะแสดงผล
  OLED.setTextSize(2); // กำหนดขนาดตัวอักษร
  // OLED.println("OLED"); // แสดงผลข้อความ ALL

  // OLED.setTextSize(1);
  // OLED.print("welcome to");
  // OLED.println(" All");
  // OLED.setTextColor(BLACK, WHITE); //กลับสีข้อความกับพื้นหลัง
  // OLED.print("www.arduinoall.com");
  OLED.setTextColor(WHITE, BLACK);
  // OLED.setCursor(60, 0);
  OLED.setTextSize(2);
  // OLED.println(var, DEC);

  // OLED.setCursor(0, 40);
  // OLED.setTextSize(2);
  // OLED.println("ArduinoALL"); // แสดงผลข้อความ ALL
  OLED.print("People: "); // แสดงผลข้อความ ALL
  OLED.println(count); // แสดงผลข้อความ ALL
  OLED.display(); // สั่งให้จอแสดงผล
  // delay(500);

  
  
  //---------NUMPAD----------
  char key = keypad.getKey();
  int currentState = digitalRead(button);



  if(lastState == HIGH && currentState == LOW){
    int timer = 0;
    while(timer < 100){
      if(state_button == 0){
        // Serial.println("The state changed!1");
        servoMotor.write(90);
        //count people
        if (!digitalRead(irPin1) && i==1 && state1){
            outsideIr=true;
            delay(100);
            i++;
            state1 = false;
          }
          if (!digitalRead(irPin2) && i==2 &&   state2){
            Serial.println("Entering into room");
            outsideIr=true;
            delay(100);
            i = 1 ;
            count++;
            Serial.print("No of persons inside the room: ");
            Serial.println(count);
            state2 = false;
          }
          if (!digitalRead(irPin2) && i==1 && state2 ){
            outsideIr=true;
            delay(100);
            i = 2 ;
            state2 = false;
          }
          if (!digitalRead(irPin1) && i==2 && state1 ){
            Serial.println("Exiting from room");
            outsideIr=true;
            delay(100);
            count--;
              Serial.print("No of persons inside the room: ");
              Serial.println(count);
            i = 1;
            state1 = false;
          }  
          if (digitalRead(irPin1)){
          state1 = true;
          }

          if (digitalRead(irPin2)){
          state2 = true;
          }
        if(digitalRead(button) == LOW && timer >= 10){
          Serial.println("1");
          servoMotor.write(pos);
          state_button = 0 ;
          timer =101;
        }
        if(count < 0){
            count = 0;
          }
        // state_button = 1;
        // servoMotor.write(pos);
      }
      // else if(currentState == HIGH){
      //   servoMotor.write(pos);
      //   state_button = 0 ;
      //   break;
      // }
      // lastState = HIGH;
      delay(100);
      timer += 1;
    }
    servoMotor.write(pos);
    state_button = 0 ;
  }

  lastState = currentState;
  // save the last state
  // lastState = currentState;

  if (key) {
    Serial.print(key);
    OLED.setCursor(0, 40);
    OLED.print(key);
    OLED.display();
    delay(500);
    if (key == '*') {
      input_password = "";  // clear input password

    } else if (key == '#') {
      Serial.println(input_password);
      
      if (password == input_password) {
        Serial.println("The password is correct, ACCESS GRANTED!");
        OLED.setCursor(0, 20);
        OLED.println("ACCESS GRANTED");
        OLED.display();
        servoMotor.write(90);
        int time = 0;
        while(time < 100){
          if (!digitalRead(irPin1) && i==1 && state1){
            outsideIr=true;
            delay(100);
            i++;
            state1 = false;
          }
          if (!digitalRead(irPin2) && i==2 &&   state2){
            Serial.println("Entering into room");
            outsideIr=true;
            delay(100);
            i = 1 ;
            count++;
            Serial.print("No of persons inside the room: ");
            Serial.println(count);
            state2 = false;
          }
          if (!digitalRead(irPin2) && i==1 && state2 ){
            outsideIr=true;
            delay(100);
            i = 2 ;
            state2 = false;
          }
          if (!digitalRead(irPin1) && i==2 && state1 ){
            Serial.println("Exiting from room");
            outsideIr=true;
            delay(100);
            count--;
              Serial.print("No of persons inside the room: ");
              Serial.println(count);
            i = 1;
            state1 = false;
          }  
          if (digitalRead(irPin1)){
          state1 = true;
          }

          if (digitalRead(irPin2)){
          state2 = true;
          }
          if(count < 0){
            count = 0;
          }
          delay(100);
          time += 1;
        }
        // delay(5000);
        servoMotor.write(pos);

      } else {
        Serial.println("The password is incorrect, ACCESS DENIED!");
      }

      input_password = "";  // clear input password
      OLED.setCursor(0, 20);
      OLED.println("ACCESS DENIED");
      OLED.display();
      delay(1000);
    } else {
      input_password += key;  // append new character to input password string
    }
  }
  //---------PIR-----------------



  if(count > 8){
    myData.id = 1;
    myData.x = 2;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent Count people with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    // delay(1000);
  }
  // if(statepass = 1){
  //   myData.id = 1;
  //   myData.x = 3;
  //   myData.y = password; // password status

  //   // Send message via ESP-NOW
  //   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
  //   if (result == ESP_OK) {
  //     Serial.println("Sent Password with success");
  //     statepass = 0;
  //   }
  //   else {
  //     Serial.println("Error sending the data");
  //   }

  // }

   
  //---------SERVO------------
}
