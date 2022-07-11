#include <test3_inferencing.h>







#include <esp_now.h>
#include <WiFi.h>


#include "esp_camera.h"
#include "esp_timer.h"
#include "Arduino.h"
#include "img_converters.h"
#include "image_util.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <HCSR04.h>
          // read and write from flash memory

/* Modify the following line according to your project name
   Do not forget to import the library using "Sketch">"Include Library">"Add .ZIP Library..."
*/


// define the number of bytes you want to access


// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define Conveyor          14

#define trigPin           12
#define echoPin           13
#define sense             15
int steps;


const float unit_step_steps  = 1.946/0.5; 
// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t Receiver_Address1[] = {0xAC, 0x0B, 0xFB, 0xDD, 0x84, 0xC5};

typedef struct struct_message {
  int integer;
  char character[12];
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


int pictureNumber = 0;

dl_matrix3du_t *resized_matrix = NULL;
size_t out_len = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
ei_impulse_result_t result = {0};
String label = "uncertain";
bool sd_card = true;



void sender(){
  
  if(label == "snickers "){
   strcpy(message.character,"snickers");
    Serial.println("snickers sent");
   message.integer = steps;
       
  }
  
    if(label == "mentos"){
   strcpy(message.character,"mentos");
    Serial.println("sent mentos");
    message.integer = steps;
  }
  
  if(label == "Eclairs"){
    strcpy(message.character,"eclairs");
    Serial.println("sent eclairs");
    message.integer = steps;
  }
    
  
 
  esp_err_t outcome = esp_now_send(0, (uint8_t *) &message, sizeof(struct_message));
   
  if (outcome == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
  }


  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);

  pinMode(Conveyor,OUTPUT);
  digitalWrite(Conveyor,HIGH);

  pinMode(sense,INPUT_PULLUP);
  

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
 

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

   if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
 
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

  

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  
}

void take_picture()
{
  camera_fb_t * fb = NULL;
  bool s;

  // --- Taking picture ---

  int64_t time_start = esp_timer_get_time();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  int64_t time_end = esp_timer_get_time();
  Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb->len), (uint32_t)((time_end - time_start) / 1000));

  // --- Save Orginal Picture ---

  // if (sd_card) {
  //   // initialize EEPROM with predefined size
  //   EEPROM.begin(EEPROM_SIZE);
  //   pictureNumber = EEPROM.read(0) + 1;

  //   // Path where new picture will be saved in SD Card
  //   String path = "/original_" + String(pictureNumber) + ".jpg";

  //   fs::FS &fs = SD_MMC;
  //   Serial.printf("Picture file name: %s\n", path.c_str());

  //   File file = fs.open(path.c_str(), FILE_WRITE);
  //   if (!file) {
  //     Serial.println("Failed to open file in writing mode");
  //   }
  //   else {
  //     file.write(fb->buf, fb->len); // payload (image), payload length
  //     Serial.printf("Saved file to path: %s\n", path.c_str());
  //     EEPROM.write(0, pictureNumber);
  //     EEPROM.commit();
  //   }
  //   file.close();
  // }

  // --- Convert frame to RGB888  ---

  Serial.println("Converting to RGB888...");
  time_start = esp_timer_get_time();
  
  // Allocate rgb888_matrix buffer
  dl_matrix3du_t *rgb888_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  s = fmt2rgb888(fb->buf, fb->len, fb->format, rgb888_matrix->item);
  time_end = esp_timer_get_time();
  Serial.printf("Done in %ums\n", (uint32_t)((time_end - time_start) / 1000));

  // --- Resize the RGB888 frame to 96x96 in this example ---

  Serial.println("Resizing the frame buffer...");
  time_start = esp_timer_get_time();
  resized_matrix = dl_matrix3du_alloc(1, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3);
  image_resize_linear(resized_matrix->item, rgb888_matrix->item, EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, 3, fb->width, fb->height);
  time_end = esp_timer_get_time();
  Serial.printf("Done in %ums\n", (uint32_t)((time_end - time_start) / 1000));

  // --- Free memory ---

  dl_matrix3du_free(rgb888_matrix);
  esp_camera_fb_return(fb);
  // wait till it reaches pickup point
  while(digitalRead(sense)==LOW)
  {
    delay(50);
  }
  digitalWrite(Conveyor,LOW);

  
 
  // --- Call classifier function ---
 
  classify();
  Serial.print(label);
  // --- Call sender function ---
  sender();

 
 
   dl_matrix3du_free(resized_matrix);



  // --- Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 ---
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
 
}
int raw_feature_get_data(size_t offset, size_t out_len, float *signal_ptr)
{
  size_t pixel_ix = offset * 3;
  size_t bytes_left = out_len;
  size_t out_ptr_ix = 0;

  // read byte for byte
  while (bytes_left != 0) {
    // grab the values and convert to r/g/b
    uint8_t r, g, b;
    r = resized_matrix->item[pixel_ix];
    g = resized_matrix->item[pixel_ix + 1];
    b = resized_matrix->item[pixel_ix + 2];

    // then convert to out_ptr format
    float pixel_f = (r << 16) + (g << 8) + b;
    signal_ptr[out_ptr_ix] = pixel_f;

    // and go to the next pixel
    out_ptr_ix++;
    pixel_ix += 3;
    bytes_left--;
  }
  return 0;
}

void HCSR04(int &steps,int &cm)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pinMode(echoPin, INPUT);
  long duration = pulseIn(echoPin, HIGH);
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

  steps = (cm*10)*unit_step_steps;
  
  delay(50);
  

 
}

void classify()
{
  Serial.println("Getting signal...");
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_WIDTH;
  signal.get_data = &raw_feature_get_data;

  Serial.println("Run classifier...");
  // Feed signal to the classifier
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false /* debug */);

  // Returned error variable "res" while data object.array in "result"
  ei_printf("run_classifier returned: %d\n", res);
  if (res != 0)
    return;

  // print the predictions
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: \t%f\r\n", result.classification[ix].label, result.classification[ix].value);
    
    if(result.classification[ix].value > 0.7){
      label = String(result.classification[ix].label);
    }
     
  }
   
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %f\r\n", result.anomaly);
#endif
}




void loop() {

  float max_x = 1320*2;
  int cm;

  HCSR04(steps, cm);
  if(cm <= 13)
  {
  
  take_picture();
  while(digitalRead(sense)==HIGH)
  {
    delay(50);
  }
  digitalWrite(Conveyor,HIGH);
  delay(100);
  }
    }
