#include <esp_now.h>
#include <WiFi.h>


int LED_gas = 32;            /*LED pin defined*/
int LED_volume = 23 ;
int LED_human = 21;
int Sensor_gas = 35;    /*Digital pin 5 for sensor input*/
int buzzer = 22;
int Sensor_volume =34;
int buttonPin = 33;
int I = 0;
int buttonState = 0;
int lastState = LOW;  // the previous state from the input pin
int currentState; 
int first = 0;
int val = 0;
int buzzeron = 0;

uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x28, 0x18, 0xC0};

typedef struct struct_message {
    int id; // must be unique for each sender board
    int x;
} struct_message;

struct_message myData;
struct_message myData_rev;





esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData_rev, incomingData, sizeof(myData_rev));
  // Update the structures with the new incoming data
  Serial.printf("x value: %d \n", myData_rev.id);
  Serial.println();
  if(myData_rev.id == 1){
     digitalWrite(buzzer, LOW);
      digitalWrite (LED_human, HIGH) ; 
      buzzeron = 1;
  }
  
}

void setup() {
  Serial.begin(9600);  /*baud rate for serial communication*/
  WiFi.mode(WIFI_STA);

if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }


  esp_now_register_send_cb(OnDataSent);
  
  // // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // // // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_gas, OUTPUT);  /*LED set as Output*/
  pinMode(LED_volume, OUTPUT);  /*LED set as Output*/
  pinMode(LED_human, OUTPUT);
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {









  int sensor_Aout = analogRead(Sensor_gas);  /*Analog value read function*/
  val = analogRead(Sensor_volume);
  Serial.print("Volume Sensor: "); 
  Serial.println(val);
  Serial.print("Gas Sensor: ");  
  Serial.print(sensor_Aout);   /*Read value printed*/
  




  currentState = digitalRead(buttonPin);
  // Serial.print(currentState);

  if (first == 0){
    digitalWrite(buzzer, HIGH);
    first = 1;

  }
   
  if (lastState == HIGH && currentState == LOW){
    Serial.println("The button is pressed");
    digitalWrite (LED_gas, LOW) ; 
    digitalWrite (LED_volume, LOW) ; 
    digitalWrite (LED_human, LOW) ; 
    digitalWrite(buzzer, HIGH);
    buzzeron = 0 ;
    
    
    }
  else if (lastState == LOW && currentState == HIGH){
    Serial.println("The button is released");
   
  }
  lastState = currentState;


    if (sensor_Aout > 285) {    /*gas */ 
    // 1. 280.  2. 1600
    digitalWrite(buzzer, LOW);
    buzzeron = 1;
    // analogWrite(buzzer, 1);
    // tone(buzzer, 1000);
    Serial.println("Gas");  
    digitalWrite (LED_gas, HIGH) ; /*LED set HIGH if Gas detected */
    myData.id = 1;
    myData.x = 0;
     esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    }
  else {
    Serial.println("Error sending the data");
  }

  }

  if (val >2500 && buzzeron == 0){
    buzzeron = 1;
    digitalWrite(buzzer, LOW);
    // analogWrite(buzzer, 1);
    // tone(buzzer, 1000);
    digitalWrite (LED_volume, HIGH) ; /*LED set HIGH if Gas detected */
    myData.id = 1;
    myData.x = 1;
     esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  }




  delay(300);                 /*DELAY of 1 sec*/
}