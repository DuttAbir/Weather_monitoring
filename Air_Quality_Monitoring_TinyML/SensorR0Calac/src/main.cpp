#include<Arduino.h>


#define Vcc 3.3   //Supply Voltage.
#define ADC 4095  //Max ADC value.
#define RLoad 9.6 //Load Resistance in KOhm.

#define atomCo2 427.0  //Constant for Atmosphereic Co2 level.
#define Co2_a 116.6020682 //Coefficients for CO2 sensor calculation.
#define Co2_b -2.769034857
#define Co2_SensorPin 34  //Analog pin for CO2 sensor.


#define CO_CleanAir_ratio 6.5 //Ratio for CO sensor calculation in clean air.(Rs/R0)
#define CO_a 2990886.965   // Coefficients for CO sensor calculation.
#define CO_b -7.358
#define CO_SensorPin 35 //Analog pin for CO sensor.


/*Calculates the sensor resistance based on ADC readings.*/
float getRs(float SensorReadings){
  float Rs = (((ADC/SensorReadings)*Vcc)-1)*RLoad;
  return Rs;
}

/*Calculates the CO sensor resistance in clean air.*/
float get_CO_R0(float rs){
  float R0 = rs/CO_CleanAir_ratio;
  return R0;
}

/* Calculates the CO2 sensor resistance in clean air.*/
float get_Co2_R0(float rs){
  float R0 = rs * pow((atomCo2/Co2_a),(1/Co2_b));
  return R0;
}


/*Initializes serial communication and sets sensor pins as inputs.*/
void setup(){
  Serial.begin(9600);

  pinMode(Co2_SensorPin,INPUT);
  pinMode(CO_SensorPin,INPUT);
}



void loop(){

float Co2_rResis=0.0;
float CO_rResis=0.0;
float Co2_rZero=0.0;
float CO_rZero=0.0;


/*Reads sensor values multiple times to average.*/
for(int i =0;i<50;i++){

  float CO_SensorVal = analogRead(CO_SensorPin);
  float Co2_SensorVal = analogRead(Co2_SensorPin);
  
  CO_rResis = getRs(CO_SensorVal);
  Co2_rResis = getRs(Co2_SensorVal);

  CO_rZero = CO_rZero + get_CO_R0(CO_rResis);
  Co2_rZero = Co2_rZero + get_Co2_R0(Co2_rResis);

  CO_rResis = 0;
  Co2_rResis = 0;
}


/*Calculates resistances for both CO and CO2 sensors.
  Calculates R0 values for both sensors.
  Prints calculated resistances and R0 values.*/
  
float CO_SensorVal = analogRead(CO_SensorPin);
CO_rResis = getRs(CO_SensorVal);
Serial.print("CO_RS: ");
Serial.println(CO_rResis);
Serial.print("CO_R0: ");
Serial.println(CO_rZero/50);


float Co2_SensorVal = analogRead(Co2_SensorPin);
Co2_rResis = getRs(Co2_SensorVal);
Serial.print("Co2_RS: ");
Serial.println(Co2_rResis);
Serial.print("Co2_R0: ");
Serial.println(Co2_rZero/50);

delay(2000);
}