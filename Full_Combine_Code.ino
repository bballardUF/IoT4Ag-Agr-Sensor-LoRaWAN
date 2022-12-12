/*
  This is a simple example show the Heltec.LoRa sended data in OLED.

  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  https://heltec.org
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include <Wire.h>
#include "Adafruit_VEML7700.h"
#include "Adafruit_AS726x.h"
#include "heltec.h"
#include "images.h"

#define BAND 915E6  //you can set band here directly,e.g. 868E6,915E6

#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet;

int pcounter = 0; 

//create the object
Adafruit_AS726x ams;

//buffer to hold raw values
uint16_t sensorValues[AS726x_NUM_CHANNELS];

//buffer to hold calibrated values (not used by default in this example)
float calibratedValues[AS726x_NUM_CHANNELS];

Adafruit_VEML7700 veml = Adafruit_VEML7700();

void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Test");
  while (!Serial) { delay(10); }
  Serial.println("Adafruit VEML7700 Test");

  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1)
      ;
  }
  Serial.println("Sensor found");
  while (!Serial)
    ;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //begin and make sure we can talk to the sensor
  
  if (!ams.begin()) {
    Serial.println("could not connect to sensor! Please check your wiring.");
    while (1)
      ;
  }
  

  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }

  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  veml.setLowThreshold(10000);
  veml.setHighThreshold(20000);
  veml.interruptEnable(true);

  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();

  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->display();
  delay(1000);

  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while(1) delay(1);
  } else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }
 
}

void loop() {
  //read the device temperature
  uint8_t temp = ams.readTemperature();

  //ams.drvOn(); //uncomment this if you want to use the driver LED for readings
  ams.startMeasurement();  //begin a measurement

  //wait till data is available
  
  bool rdy = false;
  while (!rdy) {
    delay(5000);
    rdy = ams.dataReady();
  }
  
  //ams.drvOff(); //uncomment this if you want to use the driver LED for readings

  //read the values!
  ams.readRawValues(sensorValues);
  ams.readCalibratedValues(calibratedValues);

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C");
  Serial.print(" Violet: ");
  Serial.print(sensorValues[AS726x_VIOLET]);
  Serial.print(" lumens (lm)");
  Serial.print(" Blue: ");
  Serial.print(sensorValues[AS726x_BLUE]);
  Serial.print(" lumens (lm)");
  Serial.print(" Green: ");
  Serial.print(sensorValues[AS726x_GREEN]);
  Serial.print(" lumens (lm)");
  Serial.print(" Yellow: ");
  Serial.print(sensorValues[AS726x_YELLOW]);
  Serial.print(" lumens (lm)");
  Serial.print(" Orange: ");
  Serial.print(sensorValues[AS726x_ORANGE]);
  Serial.print(" lumens (lm)");
  Serial.print(" Red: ");
  Serial.print(sensorValues[AS726x_RED]);
  Serial.print(" lumens (lm)");
  Serial.println();
  Serial.println();

  Serial.println("Packet 2: Light Data");
  Serial.print("raw ALS: ");
  Serial.println(veml.readALS());
  Serial.print("raw white: ");
  Serial.println(veml.readWhite());
  Serial.print("lux: ");
  Serial.println(veml.readLux());
  
  float tempC = ss.getTemp();
  uint16_t capread = ss.touchRead(0);

  Serial.println("Packet 1: Soil Sensor");
  Serial.print("Temperature: "); Serial.print(tempC); Serial.println(" *C");
  Serial.print("Capacitive: "); Serial.println(capread);

  uint16_t irq = veml.interruptStatus();
  if (irq & VEML7700_INTERRUPT_LOW) {
    Serial.println("** Low threshold");
  }
  if (irq & VEML7700_INTERRUPT_HIGH) {
    Serial.println("** High threshold");
  }

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);

  Heltec.display->drawString(0, 0, "Sending packet: ");
  Heltec.display->drawString(90, 0, String(counter));
  Heltec.display->display();

  if (pcounter == 0) {
    // send packet
    
    LoRa.beginPacket();
    
    LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);

    LoRa.print("Temperature: "); 
    LoRa.print(tempC); 
    LoRa.print(" *C ");
    LoRa.print("Capacitive: "); 
    LoRa.print(capread);
    
    //LoRa.print("Packet 1");   
    pcounter = 1;
    
    delay(1000);
    //LoRa.println();
    //LoRa.println();
  }
  else if (pcounter == 1) {
    
    LoRa.beginPacket();
    LoRa.print(" Yellow: ");                  // 9 Bytes
    LoRa.print(sensorValues[AS726x_YELLOW]);  //LoRa.print(" lm"); // 3 Bytes
    LoRa.print(" Orange: ");                  // 9 Bytes
    LoRa.print(sensorValues[AS726x_ORANGE]);  //LoRa.print(" lumens (lm)"); // 3 Bytes
    LoRa.print(" Red: ");                     // 6 Bytes
    LoRa.print(sensorValues[AS726x_RED]);     // LoRa.print(" lumens (lm)"); // 3 Bytes
    LoRa.endPacket();
    
    delay(1000);
    pcounter = 2;
  }  
  else if (pcounter == 2) {
    // Send sensor data
    LoRa.beginPacket();
    LoRa.print(" Violet: ");                  // 9 Bytes
    LoRa.print(sensorValues[AS726x_VIOLET]);  // 3 Bytes
    //LoRa.print(" lm");
    LoRa.print(" Blue: ");                  // 7 Bytes
    LoRa.print(sensorValues[AS726x_BLUE]);  // 3 bytes
    //LoRa.print(" lm");
    LoRa.print(" Green: ");                   // 8 Bytes
    LoRa.print(sensorValues[AS726x_GREEN]);   //LoRa.print(" lm"); // 3 Bytes
    LoRa.endPacket(); 
    delay(1000);
    pcounter = 3;
  }
  else {
    
    LoRa.beginPacket();
    LoRa.print(" raw ALS: ");
    LoRa.print(veml.readALS());
    LoRa.print(" raw white: ");
    LoRa.print(veml.readWhite());
    LoRa.print(" lux: ");
    LoRa.print(veml.readLux());
    LoRa.endPacket();
    pcounter = 0;
    delay(1000);
  }

  delay(1000);
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}