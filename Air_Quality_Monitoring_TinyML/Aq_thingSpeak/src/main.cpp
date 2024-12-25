#include<Arduino.h>
#include<WiFi.h>
#include<math.h>
#include<ThingSpeak.h>

#define Vcc 3.3    // Supply Voltage.
#define ADC 4095   //Max ADC value.
#define RLoad 9.6   //Load resistance.

#define atomCo2 427.0       //Constant for Atmospheric Co2 level.
#define Co2_a 116.6020682  //Coefficients for CO2 sensor calculation.
#define Co2_b -2.769034857
#define Co2_SensorPin 34   //Analog pin for Co2 sensor.
#define Co2_rZero 117    // R0 calculated from previois step.

#define CO_CleanAir_ratio 6.5 
#define CO_a 2990886.965       //Coefficients for CO sensor calculation.
#define CO_b -7.358
#define CO_SensorPin 35       //Analog pin for CO sensor.
#define CO_rZero 39 

/*Calculates sensor resistance based on ADC readings, Vcc, and RLoad.*/
float getRs(float SensorReadings){
  float Rs = (((ADC/SensorReadings)*Vcc)-1)*RLoad;
  return Rs;
}

/* Calculates CO2 PPM based on sensor resistance and calibration value.*/
float get_Co2_PPM(float rs, float rz){
  float ppm = Co2_a * pow((rs/rz),Co2_b);
  return ppm;
}

/*Calculates CO PPM based on sensor resistance and calibration value.*/
float get_CO_PPM(float rs, float rz){
  double ppm = CO_a * pow((rs/rz),CO_b);
  return ppm;
}

#define WiFi_timeOut 20000   

/*Declare Wifi Credentials*/
const char* SSID = "POCO M3";
const char* PASSWORD = "12345678";

WiFiClient client;


/*Declare ThingSpeak Channel Credentials*/
unsigned long  CHANNEL_ID = 2777464;
const char* CHANNEL_API_KEY = "Y44Y0E8WVEE9ILQJ";


/*intialize WiFi connection*/

void WiFi_connect(){
  Serial.print("connecting to the WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  unsigned long attempTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - attempTime < WiFi_timeOut){
    Serial.print(".");
    delay(100);
  }

  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Failed");
  }
  else{
    Serial.print("Connected to IP: ");
    Serial.println(WiFi.localIP());
  }
}


void setup(){

  Serial.begin(9600); //Initializes serial communication.

  //dht.begin(); //Begins DHT sensor.

  WiFi_connect(); // Connects to the specified WiFi network.
  ThingSpeak.begin(client); // Connect to ThingSpeak Server

  pinMode(Co2_SensorPin,INPUT); //Sets up sensor pins as inputs.
  pinMode(CO_SensorPin,INPUT);

}

void loop(){

  float Co2_rResis=0.0;
  float CO_rResis=0.0;

  /*Temperature, humidity, CO, and CO2 sensor values.*/
  float Co2_SensorVal = analogRead(Co2_SensorPin);
  Co2_rResis = getRs(Co2_SensorVal);

  float CO_SensorVal = analogRead(CO_SensorPin);
  CO_rResis = getRs(CO_SensorVal);

  float Co2_PPM = get_Co2_PPM(Co2_rResis, Co2_rZero);
  float CO_PPM = get_CO_PPM(CO_rResis, CO_rZero);

  Serial.print("Co2_PM: ");
  Serial.println(Co2_PPM);

  Serial.print("CO_PPM: ");
  Serial.println(CO_PPM);

  ThingSpeak.setField(1,Co2_PPM);
  ThingSpeak.setField(2,CO_PPM);
  ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);

  delay(10000);

}

