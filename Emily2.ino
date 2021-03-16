/* Logiciel Emily 2
   Pilotage de la chambre de fermentation
   --------------------------------------
   Author : Mathias VITALIS
   2021
*/

// ------------------------------------------------
// Headers to include
// ------------------------------------------------
#include "Emily2.h"

#include <EEPROM.h>

#include <SdFat.h>
#include <SPI.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <jsonlib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_CCS811.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <DS3231.h>



// ------------------------------------------------
// Program Globals
// ------------------------------------------------

SdFat SD;

//DHT_Unified dht(DHTPIN, DHTTYPE);
DHT dht(DHTPIN, DHTTYPE);

Adafruit_CCS811 ccs;

Config config;

OneWire oneWire(ONE_WIRE_TEMP);
DallasTemperature tempSensors(&oneWire);

DS3231 Clock;

unsigned long _millisTemp;


/**
   MANAGE CONFIG FILE
*/

/**
   Loads the configuration from a file
*/
void loadConfiguration(const char *filename, Config &config) {
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
  }

  String jsonStr = "";
  while (file.available()) {
    jsonStr += (char)file.read();
  }
  if (jsonExtract(jsonStr, "temperature").toFloat())
    config.temperature = jsonExtract(jsonStr, "temperature").toFloat();
  else
    config.temperature = 20.00;
  if (jsonExtract(jsonStr, "humidity").toFloat())
    config.humidity = jsonExtract(jsonStr, "humidity").toFloat();
  else
    config.humidity = 60;
  file.close();
}

/**
   Saves the configuration to a file
*/
void saveConfiguration(const char *filename, const Config &config) {
  SD.remove(filename);
  // Open file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  String jsonStr = "";
  jsonStr = "{\"temperature\":";
  jsonStr += String(config.temperature);
  jsonStr += ",";
  jsonStr += "\"humidity\":";
  jsonStr += String(config.humidity);
  jsonStr += "}";

  file.print(jsonStr);

  // Close the file (File's destructor doesn't close the file)
  file.close();
}



void setup()
{
  // ------------------------------------------------
  // Initialize
  // ------------------------------------------------
  Serial.begin(9600);
  //delay(1000);  // NOTE: Some devices require a delay after Serial.begin() before serial port can be used


  // ------------------------------------------------
  // Create graphic elements : Home
  // ------------------------------------------------


  Wire.begin();

  drawHomeScreen();


  /**
     SD CARD INIT
  */
  pinMode(SD_CHIP_SELECT, OUTPUT);

  SPI.begin();
  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println(F("SD: Begin error\n"));
    return;
  } else {
    Serial.println(F("SD: OK!\n"));
  }



  /**
     TEMPERATURE HUMIDITY SENSOR
  */

  // Initialize device.
  dht.begin();
  // Print temperature sensor details.
  
  /**
     CCS881 Setup
  */

  if (!ccs.begin()) {
    Serial.println("Failed to start sensor CCS811! Please check your wiring.");
  }
  ccs.setDriveMode(1);
  //calibrate temperature sensor
  while (!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  /**
     OneWire Temp Sensor
  */

  if (SENSOR_TYPE_TEMPERATURE == 1) {
    tempSensors.begin();
  }

  /**
       JSON CONFIG FILE
  */

  Serial.println(F("Loading configuration..."));
  loadConfiguration("/config.ini", config);

  static char temp_cible[3];
  static char hygro_cible[3];

  itoa(config.temperature, temp_cible, 10);
  itoa(config.humidity, hygro_cible, 10);

  //Relay output

  pinMode(RELAY_WARM_PIN, OUTPUT);
  pinMode(RELAY_COLD_PIN, OUTPUT);


  // ------------------------------------------------
  // Create graphic elements
  // ------------------------------------------------

  drawTargetTemperature(config.temperature);
  drawTargetHygrometry(config.humidity);

  _millisTemp = millis();

}

// -----------------------------------
// Main event loop
// -----------------------------------
void loop()
{

  // ------------------------------------------------
  // Update GUI Elements
  // ------------------------------------------------

  updateTemperature();
  updateHumidity();
  updateCO2();
  setDate();
  drawClock();


  // ------------------------------------------------
  // Periodically call GUIslice update function
  // ------------------------------------------------
  touchDetect();
}


/**
   Update Temperature
*/

void updateTemperature()
{
  char msg[5] = "";

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  float temperatureSensorValue = 0;

  // Si BLINK_INTERVAL_1 ou plus millisecondes se sont écoulés
  if (currentMillis - previousMillis >= BLINK_INTERVAL_TEMPERATURE) {

    // keep value of millis() in memory
    previousMillis = currentMillis;

    if (SENSOR_TYPE_TEMPERATURE == 1) {
      tempSensors.requestTemperatures(); // Send the command to get temperature readings

      if (isnan(tempSensors.getTempCByIndex(0))) {
        Serial.println(F("Error reading temperature!"));
        temperatureSensorValue = -50;
        switchRelay(RELAY_WARM_PIN, false);
        drawTempNull();
        switchRelay(RELAY_COLD_PIN, false);
      } else {
        temperatureSensorValue = tempSensors.getTempCByIndex(0);
      }
    } else {
      // Get temperature event and print its value.
      float temp;
      temp = dht.readTemperature();
      if (isnan(temp)) {
        Serial.println(F("Error reading temperature!"));
        switchRelay(RELAY_WARM_PIN, false);
        drawTempNull();
        switchRelay(RELAY_COLD_PIN, false);
        temperatureSensorValue = -50;
      }
      else {
        temperatureSensorValue = temp - CALIBRATION_TEMP_DHT11;
      }
    }
    Serial.print(F("Temperature: "));
    Serial.print(temperatureSensorValue);
    Serial.println(F("°C"));

    // Get temperature event and print its value.
    
    if (temperatureSensorValue > -50) {
      Serial.print(F("Temperature: "));
      Serial.print(temperatureSensorValue);
      Serial.println(F("°C"));

      /**
         RELAYS
      */

      if (millis() - _millisTemp > TEMPERATURE_DELAY_ON_SEC * 1000) {
        if (temperatureSensorValue >= config.temperature + TEMPERATURE_DELTA ) {
          switchRelay(RELAY_COLD_PIN, true);
          switchRelay(RELAY_WARM_PIN, false);
          drawTempCold();
        } else if ( temperatureSensorValue <= config.temperature - TEMPERATURE_DELTA ) {
          switchRelay(RELAY_COLD_PIN, false);
          switchRelay(RELAY_WARM_PIN, true);
          drawTempWarm();
        }
        else {
          switchRelay(RELAY_COLD_PIN, false);
          switchRelay(RELAY_WARM_PIN, false);
          drawTempNull();
        }
        _millisTemp = millis();
      }
      drawTemperature(temperatureSensorValue);

    }


  }
}




void updateCO2() {

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  // Si BLINK_INTERVAL_1 ou plus millisecondes se sont écoulés
  if (currentMillis - previousMillis >= BLINK_INTERVAL_CO2) {

    // Garde en mémoire la valeur actuelle de millis()
    previousMillis = currentMillis;

    if (ccs.available()) {
      float temp = ccs.calculateTemperature();
      if (!ccs.readData()) {
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print("ppb Temp:");
        Serial.println(temp);

        drawCO2(ccs.geteCO2(), ccs.getTVOC());
      }
      else {
        Serial.println("CCS811 ERROR!");
      }
    }
  }

}



/**
   UPDATE HUMIDITY
*/
void updateHumidity()
{
  char msg[5] = "";

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  // Si BLINK_INTERVAL_1 ou plus millisecondes se sont écoulés
  if (currentMillis - previousMillis >= BLINK_INTERVAL_HUMIDITY) {

    // Garde en mémoire la valeur actuelle de millis()
    previousMillis = currentMillis;

    // Get humidity event and print its value.
    float hum;
    hum = dht.readHumidity();
    if (isnan(hum)) {
      Serial.println(F("Error reading humidity!"));
      Serial.print(hum);
      digitalWrite(ATOMISEUR_PIN, LOW);
      drawHygroOff();
    }
    else {
      Serial.print(F("Humidity: "));
      Serial.print(hum);
      Serial.println(F("%"));

      /**
         ATOMISEUR
      */

      if (hum >= config.humidity) {
        //Set Humidity Icon OFF
        digitalWrite(ATOMISEUR_PIN, LOW);
        drawHygroOff();
      }
      else {
        //Set Humidity Icon ON
        digitalWrite(ATOMISEUR_PIN, HIGH);
        drawHygroOn();
      }

      dtostrf(hum, 3, 0, msg);
      strcat(msg, "%");
      //Set Gauge
      drawHygrometry(round(hum));
    }
  }



}


void switchRelay(int TYPE, bool state)
{
  if (state == true) {
    digitalWrite(TYPE, HIGH);
  } else {
    digitalWrite(TYPE, LOW);
  }
}


/**
   PH METER SENSOR GET VALUE
*/

float getPhMeterSensorValue() {
  int sensorValue = 0;
  unsigned long int avgValue;
  float b;
  int buf[10], temp;

  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(PH_METER_SENSOR_PIN);
    delay(10);
  }
  //Get 10 values for average
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  //Calculate average value
  avgValue = 0;
  for (int i = 2; i < 8; i++)  //take the average value of 6 center sample
    avgValue += buf[i];
  float pHVol = (float)avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  // float phValue = -5.70 * pHVol + 21.34;
  float phValue = 3.5 * pHVol; //convert the millivolt into pH value
  Serial.print("PH VALUE = ");
  Serial.println(phValue);

  drawPH(phValue);

  return phValue;
}
