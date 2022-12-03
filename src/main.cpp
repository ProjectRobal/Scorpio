#include <Arduino.h>


#include "ADCDMA.hpp"


#define SAMP 3000




DMAADC<SAMP> adc(2.f);

void setup() {
  // put your setup code here, to run once:

  Serial.begin();

  while(adc.check_error())
  {
    adc=DMAADC<SAMP>(2.f);

    Serial.println("Cannot init ADC!");

    delay(1000);
  }

}


uint8_t channel=0;

void loop() {
  adc.sample();

  // use to set between channel
  if(Serial.available())
  {

    channel=Serial.read();

    if(!(channel=='\n' || channel=='\r'))
    {

    channel-=48;

    if(channel>=3)
    {
      channel=2;     
    }

    Serial.print("Channel ");
    Serial.print(channel);
    Serial.println(" selected");
    delay(2000);

    }
  }

  
  for(uint16_t i=channel; i<SAMP;i+=3)
  {
    Serial.println(adc[i]);
  }

  delay(1000);

}