/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature
 * sensor using DHT11/DHT-22 
 * http://www.mysensors.org/build/humidity
 */
//#define MY_DEBUG 
#define MY_RADIO_NRF24
#define MY_NODE_ID 10
 
#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>  


#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 3

#ifdef MY_DEBUG    
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)

#endif 
#ifndef MY_DEBUG
unsigned long SLEEP_TIME = 210000; // Sleep time between reads (in milliseconds)
#endif 

  
#define VBAT_PER_BITS 0.003363075  // Calculated volts per bit from the used battery montoring voltage divider.   Internal_ref=1.1V, res=10bit=2^10-1=1023, Eg for 3V (2AA): Vin/Vb=R1/(R1+R2)=470e3/(1e6+470e3),  Vlim=Vb/Vin*1.1=3.44V, Volts per bit = Vlim/1023= 0.003363075
#define VMIN 1.9  // Battery monitor lower level. Vmin_radio=1.9V
#define VMAX 3.3  //  " " " high level. Vmin<Vmax<=3.44

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 3     // what digital pin we're connected to

DHT dht(DHTPIN, DHTTYPE);
float lastTemp = -1;
float lastHum = -1;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
unsigned long WAIT_TIME = 5000L;

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int lastBatteryPcnt = -1;

void setup()  
{ 
  dht.begin();
  
  metric = getConfig().isMetric;
}

void before() {
    // This will execute before MySensors starts up
}

void presentation()  
{ 

  analogReference(INTERNAL);
  
  // Send the Sketch Version Information to the Gateway
  sendSketchInfo("Humidity", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
 }

void loop()      
{  
  wait(2000);
  int sensorValue = analogRead(BATTERY_SENSE_PIN);    // Battery monitoring reading
  
  float temperature =  dht.readTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } 
  #ifndef MY_DEBUG  
  else if (temperature != lastTemp) {
  #endif
  
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.readTemperature(true);
    }
    send(msgTemp.set(temperature, 1));
    
    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
    
  #ifndef MY_DEBUG  
  } 
  #endif
  
  #ifdef MY_DEBUG
  Serial.println("Reading humidity");  
  #endif
  
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } 
  #ifndef MY_DEBUG
  else if (humidity != lastHum) {
  #endif
  
    lastHum = humidity;
    send(msgHum.set(humidity, 1));
    
    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif  

  #ifndef MY_DEBUG
  } 
  #endif
  
  
  float Vbat  = sensorValue * VBAT_PER_BITS;
  
//  int batteryPcnt = sensorValue / 10;
  int batteryPcnt = static_cast<int>(((Vbat-VMIN)/(VMAX-VMIN))*100.);   
  
  #ifdef MY_DEBUG
  Serial.println(sensorValue);
  Serial.print("Battery Voltage: "); 
  Serial.print(Vbat); Serial.println(" V");
  Serial.print("Battery percent: "); 
  Serial.print(batteryPcnt); 
  Serial.println(" %");
  #endif

  #ifndef MY_DEBUG
  if (lastBatteryPcnt != batteryPcnt) {
  #endif

    lastBatteryPcnt = batteryPcnt;
    sendBatteryLevel(batteryPcnt);
  
  #ifndef MY_DEBUG
  }
  #endif
  
  if(isTransportOK()){
    sleep(SLEEP_TIME); //sleep a bit
  } 
  else {
    wait(WAIT_TIME); // transport is not operational, allow the transport layer to fix this
  } 
}


