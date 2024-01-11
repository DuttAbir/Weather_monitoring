
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ThingSpeak.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "LOVE U 3000";
const char *password = "AVATARRR";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String Weekdays[7]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
String Months[12]= {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

const int Yellow = 12;
const int Green = 13;
const int Blue = 15;

WiFiClient client;
long MyChannelNumber = 2372339;
const char myWriteAPIKey[] = "VDK7O9C775NC274S";

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  Serial.print("connecting to.....");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  timeClient.begin();
  timeClient.setTimeOffset(19800);
  
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("Allocation Failed");
    for(;;);
    }
   delay(2000);

   display.clearDisplay();
   display.setTextColor(WHITE);

   pinMode(Yellow, OUTPUT);
   pinMode(Green, OUTPUT);
   pinMode(Blue, OUTPUT);

   ThingSpeak.begin(client);
   
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000);

  timeClient.update();

  int currentHour= timeClient.getHours();
  int currentMinute= timeClient.getMinutes();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if(isnan(t)|| isnan(h)){
    Serial.println("Failed to read from DHT sensor");
    }

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
  display.setCursor(10,9);
  display.print(Weekday);
  display.setTextSize(1);
  display.setCursor(40,9);
  display.print(currentDate);
  

//Temperature
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Temp: ");

  display.setTextSize(2);
  display.setCursor(45,20);
  display.print(t);

  display.setTextSize(1);
  display.cp437(true);

  display.setTextSize(1);
  display.print("C");

//Humidity
  display.setTextSize(1);
  display.setCursor(0,45);
  display.print("HMDTY: ");

  display.setTextSize(2);
  display.setCursor(45,45);
  display.print(h);

  display.setTextSize(1);
  display.print("%");
  

  display.display();

  if(t>=26){
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    digitalWrite(15, LOW);
  }
  else if (t>=22 && t<26){
    digitalWrite(13, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(15, LOW);
  }
  else if (t<22){
    digitalWrite(15, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW); 
  }
  
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  
  /*ThingSpeak.writeField(MyChannelNumber, 1, t,myWriteAPIKey);
  ThingSpeak.writeField(MyChannelNumber, 2, h,myWriteAPIKey);*/

  ThingSpeak.writeFields(MyChannelNumber, myWriteAPIKey);
}
  
