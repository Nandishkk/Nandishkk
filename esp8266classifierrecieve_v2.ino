#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <HCSR04.h>
#include <MultiStepper.h>

//pins for stepper driver
const int stepPin_M1            = D4;
const int directionPin_M1       = D3;
const int stepPin_M2            = D8;
const int directionPin_M2       = D7;
const int limit_switch1         = D1;
const int trigger               = D5;
const int echo                  = D6;
const int tray1                 = 200;
const int tray2                 = 400;  

// float distance                  = 0;
const float unit_step_distance  = 1.946/0.5;  //distance travelled by X-axis with each step in mm
// float pickupdistance            = 1220-distance;


//define stepper config fir accelstepper
AccelStepper stepper1(AccelStepper::DRIVER, stepPin_M1, directionPin_M1);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin_M2, directionPin_M2);
UltraSonicDistanceSensor distanceSensor(trigger, echo); 
elapsedMillis printTime;
MultiStepper steppers;

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

  // while(digitalRead(limit_switch1)==0)
  // {
  //   Serial.println("Homming......");
  //   homing();
  // }

 
  
  stepper1.setMaxSpeed(800);
  stepper1.setAcceleration(300);
  stepper1.setCurrentPosition(0);
    
  stepper2.setMaxSpeed(800);
  stepper2.setAcceleration(500);
  stepper2.setCurrentPosition(0);
  
  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2); 
    
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
void HCSR04(float &dist)
{
 dist = distanceSensor.measureDistanceCm()*10; //in mm
 dist = unit_step_distance*dist; 
 
}
  

  // void stepper_moniter()
  // { 
  
  //   float m1Speed;
  //   float m2Speed;
  //   if (printTime >= 200) 
  //    {  
  //    printTime = 0;
  //    m1Speed = stepper1.speed();
  //    m2Speed = stepper2.speed();
  //    Serial.print("speed_1: ");
  //    Serial.println(m1Speed);
  //    Serial.print("speed_2: ");
  //    Serial.println(m2Speed);
  //    Serial.print("Current_position_1: ");
  //    Serial.println(stepper1.currentPosition());
  //    Serial.print("Current_position_2: ");
  //    Serial.println(stepper2.currentPosition());
  //    }
    
     
    //  if (position == target)
    //  {
    //    delay(1000);
    //    stepper1.moveTo(tray1);
    //  }
    //  while(position!=tray1)
    //  {
    //    stepper1.run();
    //  }
         
    //  }

  
  

void loop()
{ 
  
  float max_x = 1220*2;
  float distance;
  HCSR04(distance);
  float target = max_x-distance;
  long positions[2]; // Array of desired stepper positions
  positions[0] = target;
  positions[1] = 600*4;
  
  
  if (obj==output1)
  {

  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper1.currentPosition());
  Serial.println(stepper2.currentPosition());
  yield();

 
  positions[1] = 0;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper2.currentPosition());
  yield();


  positions[0] = (tray1)*2;
  positions[1] = 600*4;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper2.currentPosition());
  yield();

  
  positions[1] = 0;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper1.currentPosition());
  Serial.println(stepper2.currentPosition());
  yield();

}
else if(obj==output2)
{
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper1.currentPosition());
  Serial.println(stepper2.currentPosition());
  yield();

 
  positions[1] = 0;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper2.currentPosition());
  yield();


  positions[0] = (tray2)*2;
  positions[1] = 600*4;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper2.currentPosition());
  yield();

  
  positions[1] = 0;
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
  Serial.println(stepper1.currentPosition());
  Serial.println(stepper2.currentPosition());
  yield();
}


}


    


   
   

