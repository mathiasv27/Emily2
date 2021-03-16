#define SD_CHIP_SELECT        53

#define DHTPIN                31

#define ONE_WIRE_TEMP         41 

#define RELAY_WARM_PIN        30
#define RELAY_COLD_PIN        28

#define ATOMISEUR_PIN         40

#define PH_METER_SENSOR_PIN   A10

#define DHTTYPE               DHT22     // DHT11 or DHT22

#define SENSOR_TYPE_TEMPERATURE 1 // 1= module DS18B20 , 2 = module DHT11
#define CALIBRATION_TEMP_DHT11 0.9 

const unsigned long BLINK_INTERVAL_TEMPERATURE = 5000;
const unsigned long BLINK_INTERVAL_HUMIDITY = 5000;
const unsigned long BLINK_INTERVAL_CO2 = 10000;

const unsigned long TEMPERATURE_MIN = 0;
const unsigned long TEMPERATURE_MAX = 50;
const unsigned long TEMPERATURE_DELAY_ON_SEC = 45;
const unsigned long TEMPERATURE_DELTA = 1.5;

const unsigned long HUMIDITY_MIN = 0;
const unsigned long HUMIDITY_MAX = 100;

struct Config {
  float temperature;
  float humidity;
};
