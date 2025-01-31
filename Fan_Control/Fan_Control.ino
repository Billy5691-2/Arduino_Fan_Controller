#include <pwm.h>

// Temperature control variables
const int thermistorPin = 0; //Pin of Thermistor output
int rawTempVoltage; // Raw output from thermistor pin
const float r1 = 10000; // Fixed resistor value, 10,000 Ohms
float logR2, r2, temp, avgTemp;
const float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07; //Stein Heart Equation Values

const int RT_Len = 10;
float rollingTemp[RT_Len];
int tempIndex = 0;
//float rolling_temp[RT_Len] = {0.0, 0.0, 0.0, 0.0, 0.0};
//std::fill(std::begin(rolling_temp), std::end(rolling_temp), 0);


// Fan control variables
const word PWM_FREQ_HZ = 25000;
PwmOut pwm(D9);




const int TEMP_MIN = 2;
const int TEMP_MAX = 40;
float PWMDuty;
int PWMDuty_int;



void setup() {
  Serial.begin(9600);

  pwm.begin(PWM_FREQ_HZ, 20.0f);
  PWMDuty = 20.0;
  delay(2000);

  Serial.print("Program Start");
  Serial.println("");


  //for (int i=0; i < RT_Len; i++) rolling_temp[i] = 0;

  std::fill(std::begin(rollingTemp), std::end(rollingTemp), 0);

}

void temperature() {
  //Reads the voltage from an analog pin connected to a thermistor and converts it to a temperature in degrees C
  rawTempVoltage = analogRead(thermistorPin);

  r2 = r1 * (1023.0 / (float)rawTempVoltage - 1);
  logR2 = log(r2);
  temp = (1.0 / (A + B*logR2 + C*logR2*logR2*logR2));
  temp = temp - 273.15;
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
}

void update_avg_temp() {
  //Updates the rolling average temp;
  float total = 0;
  rollingTemp[tempIndex] = temp;
  
  for (int i=0; i<RT_Len; i++){
    total += rollingTemp[i];
  }

  tempIndex ++;
  if (tempIndex > RT_Len) tempIndex = 0;
  
  avgTemp = total / RT_Len;

  Serial.print("Average Temperature: ");
  Serial.print(avgTemp);
  Serial.println(" C");

}

void setPWMDuty(float duty){
  pwm.pulse_perc(duty);
}

void fanControl() {
  //Change the PWM duty cycle to control fan speed
  //Changes speed based on temperature
  //Below 20C, the fan runs at minimum speed
  //Above 40C, the fan runs at full speed
  //In between, the fan runs at a relative value between 20 and 100%

  if (avgTemp < TEMP_MIN) {
    PWMDuty = 20.0;
  } 
  else if (avgTemp > TEMP_MAX) {     
    PWMDuty = 100.0;
  }
  else {
    PWMDuty = 20 + ((avgTemp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN) * 80);
  }
  PWMDuty_int = static_cast<int>(PWMDuty);

  setPWMDuty(PWMDuty);
  Serial.print("PWM Duty: ");
  Serial.println(PWMDuty_int);
}



void loop() {
  temperature();
  update_avg_temp();
  fanControl();
  Serial.println("");
  delay(1000);


}




