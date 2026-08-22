#include <Adafruit_ADS1X15.h>

#include <Wire.h>


Adafruit_ADS1X15 ads;

void setup() {
Serial.begin(115200);
if (!ads.begin()){
  Serial.print("ADS not found");
  while(1);
}
ads.setGain(GAIN_ONE);
ads.setDataRate(RATE_ADS1115_250SPS);
Serial.print("Start");
}

void loop() {
int16_t raw_f=ads.readADC_SingleEnded(0);
Serial.print("F, ");
Serial.println(raw_f);
}
