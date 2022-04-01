#include "Hive.h"
#include "ClientWrapper.h"
#include "HX711.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

#define calibration_factor -22350.0 //wsp. kalibracji
#define DOUT  32
#define CLK  33
#define transistor 13

float humidity = 0;
float tempC = 0;
float weight;

Weather sensor;
HX711 scale;

const String ID = "e62b5b3d-aaaa-aaaa-885c-ce6ad2e9750a";

ClientWrapper client;

void setup() {
  Serial.begin(115200);
  randomSeed(33);

  client.init("matek", "matek1234", "http://51.68.141.235:8088/");

  pinMode(transistor,OUTPUT);
  digitalWrite(transistor, HIGH);
    
  sensor.begin();
    
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();
}

void loop() {
  humidity = sensor.getRH();

  tempC = sensor.getTemp();
  
  weight = scale.get_units(), 2; //kilograms [kg]
  Serial.println(weight);
  
  Hive h1 = Hive(2, ID, tempC, humidity, weight);
  client.updateSingleHive(h1);

  delay(5000);
  //delay(5000);
}

float randomFloat(int a, int b) {
  return random(a * 100, b * 100) / 100.0;
}
