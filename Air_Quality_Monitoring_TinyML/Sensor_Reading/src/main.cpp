/*Includes necessary libraries*/
#include<Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_Sensor.h>
//#include <DHT.h>
#include <WiFi.h>
#include<math.h>
//#include<ThingSpeak.h>

#define Vcc 3.3    // Supply Voltage.
#define ADC 4095   //Max ADC value.
#define RLoad 9.6   //Load resistance.

#define atomCo2 427.0       //Constant for Atmospheric Co2 level.
#define Co2_a 116.6020682  //Coefficients for CO2 sensor calculation.
#define Co2_b -2.769034857
#define Co2_SensorPin 34   //Analog pin for Co2 sensor.
#define Co2_rZero 84    // R0 calculated from previois step.

#define CO_CleanAir_ratio 6.5 
#define CO_a 2990886.965       //Coefficients for CO sensor calculation.
#define CO_b -7.358
#define CO_SensorPin 35       //Analog pin for CO sensor.
#define CO_rZero 33        //R0 calculated from previous step.

//#define DHT_PIN 5        //Digital pin for DHT-11 sensor

//DHT dht(DHT_PIN, DHT11);  // Create Instance of the DHT-11 sensor

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String Weekdays[7]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
String Months[12]= {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

int predict_air_quality(double co, double co2) {
    // Weight matrix and bias values from your logistic regression model
    double weights[5][2] = {
        {-2.11130717, -0.02267492},
        {-0.90566874, -0.00308743},
        { 0.13311002,  0.0097678},
        {1.15271109,  0.00769154},
        {1.73115465,  0.00830161}
    };
    double biases[5] = {
        21.84339371,   9.70670356,  -2.92515529,  -9.55599913,
       -19.06894285
    };

    // Calculate probabilities for each air quality class
    double probabilities[5];
    for (int i = 0; i < 5; i++) {
        probabilities[i] = biases[i] + weights[i][0] * co + weights[i][1] * co2;
        probabilities[i] = sigmoid(probabilities[i]);
    }

    // Find the class with the highest probability
    int predicted_class = 0;
    double max_probability = probabilities[0];
    for (int i = 1; i < 5; i++) {
        if (probabilities[i] > max_probability) {
            max_probability = probabilities[i];
            predicted_class = i;
        }
    }

    return predicted_class;
}

void show_prediction(int air_quality){
    switch (air_quality)
    {
    case 0:
        //printf("GOOD");
        display.setTextSize(2);
        display.setCursor(40,45);
        display.print("GOOD");
        break;
    case 1:
        //printf("MODERATE");
        display.setTextSize(2);
        display.setCursor(35,45);
        display.print("MODERATE");
        break;
    case 2:
        //printf("BAD");
        display.setTextSize(2);
        display.setCursor(45,45);
        display.print("BAD");
        break;
    case 3:
        //printf("UNHEALTHY");
        display.setTextSize(2);
        display.setCursor(30,45);
        display.print("UNHEALTHY");
        break;
    case 4:
        //printf("HAZARDOUS");
        display.setTextSize(2);
        display.setCursor(30,45);
        display.print("HAZARDOUS");
        break;
    default:
    //printf("Undefined");
    display.setTextSize(2);
        display.setCursor(40,45);
        display.print("NULL");
        break;
    }
}


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
const char* SSID = "LOVE U 3000";
const char* PASSWORD = "AVATARRR";

WiFiClient client;


/*Declare ThingSpeak Channel Credentials*/
/*unsigned long  CHANNEL_ID = 2592404;
const char* CHANNEL_API_KEY = "KX0W99E3IUXJ0BNI";*/


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
  //ThingSpeak.begin(client); // Connect to ThingSpeak Server

  pinMode(Co2_SensorPin,INPUT); //Sets up sensor pins as inputs.
  pinMode(CO_SensorPin,INPUT);

  timeClient.begin();
  timeClient.setTimeOffset(19800);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("Allocation Failed");
    for(;;);
    }
   delay(2000);

   display.clearDisplay();
   display.setTextColor(WHITE);

}

void loop(){

delay(5000);

timeClient.update();

int currentHour= timeClient.getHours();
int currentMinute= timeClient.getMinutes();

display.clearDisplay();

//Time
  /*display.setTextSize(1);
  display.setCursor(50,0);
  display.print(currentHour);*/
  display.setTextSize(1);
  display.setCursor(62,0);
  display.print(":");
  display.setTextSize(1);
  display.setCursor(68,0);
  display.print(currentMinute);

  if(currentHour >= 00 && currentHour <12){
    display.setTextSize(1);
    display.setCursor(50,0);
    display.print(currentHour);
    display.setTextSize(1);
    display.setCursor(85,0);
    display.print("AM");
    }
  else if(currentHour >= 12){
    display.setTextSize(1);
    display.setCursor(50,0);
    display.print(currentHour-12);
    display.setTextSize(1);
    display.setCursor(85,0);
    display.print("PM");
    }


//Date
  time_t epochTime = timeClient.getEpochTime();

  String Weekday = Weekdays[timeClient.getDay()];

  struct tm *ptm = gmtime((time_t *)&epochTime);

  int MonthDay = ptm->tm_mday;

  int Month = ptm->tm_mon+1;
  String currentMonth = Months[Month-1];

  int currentYear = ptm->tm_year+1900;

  String currentDate = String(MonthDay) + "-" + String(currentMonth) + "-" + String(currentYear);


  display.setTextSize(1);
  display.setCursor(15,9);
  display.print(Weekday);
  display.setTextSize(1);
  display.setCursor(55,9);
  display.print(currentDate);

float Co2_rResis=0.0;
float CO_rResis=0.0;

/*Temperature, humidity, CO, and CO2 sensor values.*/
float Co2_SensorVal = analogRead(Co2_SensorPin);
Co2_rResis = getRs(Co2_SensorVal);

float CO_SensorVal = analogRead(CO_SensorPin);
CO_rResis = getRs(CO_SensorVal);

//float h = dht.readHumidity();
//float t = dht.readTemperature();

/*Calculates CO and CO2 PPM.*/
float Co2_PPM = get_Co2_PPM(Co2_rResis, Co2_rZero);
float CO_PPM = get_CO_PPM(CO_rResis, CO_rZero);

int air_quality = predict_air_quality(CO_PPM, Co2_PPM);


/*Prints sensor data to serial monitor.*/
/*Serial.print("Temperature : ");
Serial.print(t);
Serial.println(" °C");

Serial.print("Humidity : ");
Serial.print(h);
Serial.println(" %");*/

/*Serial.print("Co2_PM: ");
Serial.println(Co2_PPM);

Serial.print("CO_PPM: ");
Serial.println(CO_PPM);*/

  display.setTextSize(1);
  display.setCursor(0,25);
  display.print("Co2: ");

  display.setTextSize(1);
  display.setCursor(25,25);
  display.print(Co2_PPM);

  display.setTextSize(1);
  display.setCursor(75,25);
  display.print("CO: ");

  display.setTextSize(1);
  display.setCursor(100,25);
  display.print(CO_PPM);

  show_prediction(air_quality);

display.display();

delay(1000);

/*Sends data to ThingSpeak.*/
/*ThingSpeak.setField(1,t);
ThingSpeak.setField(2,h);
ThingSpeak.setField(3,Co2_PPM);
ThingSpeak.setField(4,CO_PPM);
ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);*/

//delay(10000);
}