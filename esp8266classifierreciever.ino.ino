#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <HCSR04.h>

//pins for stepper driver
const int stepPin_M1      = D4;
const int directionPin_M1 = D3;
const int stepPin_M2      = D8;
const int directionPin_M2 = D7;
const int limit_switch1   = D1;
const int transmit        = D5;
const int echo            = D6;

float distance            = 0;
float unit_step_distance  = 1.946; //distance travelled by X-axis with each step in mm

//define stepper config fir accelstepper
AccelStepper stepper1(AccelStepper::DRIVER, stepPin_M1, directionPin_M1);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin_M2, directionPin_M2);
UltraSonicDistanceSensor distanceSensor(transmit, echo); 
elapsedMillis printTime;

//define objects
String output1 = {"Object1"};
String output2 = {"Object2"};
String obj;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int integer;
    char character[8];
} struct_message;

// Create a struct_message called myData
struct_message message;


// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("int: ");
  Serial.println(message.integer);
  Serial.print("Char: ");
  Serial.println(message.character);
  obj = message.character;
}

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT); 
  // Initialize Serial Monitor
  Serial.begin(115200);

  while(digitalRead(limit_switch1)==0)
  {
    Serial.println("Homming......");
    homing();
  }

 
  digitalWrite( directionPin_M1,LOW);
  stepper1.setMaxSpeed(800);
  stepper1.setAcceleration(300);
    
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(600);
    
    
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  
}
void HCSR04()
{
 distance = distanceSensor.measureDistanceCm()/10; //in mm
 distance = unit_step_distance*distance; 
 Serial.println("Object Distance in steps")
 Serial.println(distance);
 Serial.println("");

}



  
  void homing()
  {
    pinMode(directionPin_M1,OUTPUT);
    pinMode(stepPin_M1,OUTPUT);

    digitalWrite( directionPin_M1,HIGH);


    for(int i=1;i<1000;i++){
      
    digitalWrite(stepPin_M1,HIGH);
    delay(5);
    digitalWrite(stepPin_M1,LOW);
    delay(5);
  }

  }

void loop()
{  
   if(obj == output1 && distance<14)
   {
    float m1Speed;
    float m2Speed;
    if (printTime >= 200) 
     {  
     printTime = 0;
     m1Speed = stepper1.speed();
     m2Speed = stepper2.speed();
     Serial.print("speed_1: ");
     Serial.println(m1Speed);
     Serial.print("speed_2: ");
     Serial.println(m2Speed);
     Serial.print("Current_position_1: ");
     Serial.println(stepper1.currentPosition());
     Serial.print("Current_position_2: ");
     Serial.println(stepper2.currentPosition());
     }

     int position = stepper1.currentPosition();
     switch (position)
     {
       case 0: stepper1.moveTo(1220);
               stepper2.moveTo(500);
            
               
       break;
       delay(1000);

       case 1220: stepper1.moveTo(0);
                  stepper2.moveTo(0);
       break;
     }
      

    // if(stepper1.currentPosition()==0)
    //  {
    //   stepper1.moveTo(1220);
    //   Serial.print("Distance to go: ");
    //   Serial.println(stepper1.currentPosition());
    //  }
  
    //  if(stepper1.distanceToGo()==0)
    //  {
       
    //   stepper1.moveTo(0);
    //   Serial.print("Distance to go: ");
    //   Serial.println(stepper1.currentPosition());
    
      
     }
       stepper1.run();
       stepper2.run();
     }
   
   

