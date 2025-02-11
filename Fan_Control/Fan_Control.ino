#include <pwm.h>
#include "Arduino_LED_Matrix.h"

// Initialises the LED matrix used to display temperature and fan speed.
ArduinoLEDMatrix matrix;


unsigned long matrixFrame[] = { // 12 rows, 8 columns, 92 bits total. 3 nibbles per row
  0x00000000,
  0x00000000,
  0x00000000
};

unsigned short PWM_Target_LED = 0x0FFF, PWM_LED = 0x0FFF, Temp_LED = 0x0FFF;


// Temperature control variables
const int thermistorPin = 0; //Pin of Thermistor output
int rawTempVoltage; // Raw output from thermistor pin
const float r1 = 10000; // Fixed resistor value, 10,000 Ohms
float logR2, r2, temp, avgTemp;
const float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07; //Stein Heart Equation Values

const int RT_Len = 10; // Length of rolling temp list
// higher RT_Lens mean transient temperature changes are more smoothed out. 
float rollingTemp[RT_Len];
int tempIndex = 0;


// Fan control variables
const word PWM_FREQ_HZ = 25000;
PwmOut pwm(D9);

const int TEMP_MIN = 10;
const int TEMP_MAX = 40;
float PWMDuty;
float Target_PWMDuty;
int PWMDuty_int;
int Target_PWMDuty_int;

unsigned long curTime; //millis()
unsigned long prevTime;
const unsigned long Fan_Interval = 1000; // Time between loop iterations in milliseconds

const float PWMDuty_Max_Change = 10 / (Fan_Interval / 1000); //Maximum PWM_Duty change per second, in percent 

//Add Ramp Rate
//Time to ramp  - 8 seconds? and ramp increase per step? 10%
//Requires recording previous PWMDuty and adding 5%  or last few %



void setup() {
  Serial.begin(9600);

  pwm.begin(PWM_FREQ_HZ, 20.0f);
  PWMDuty = 20.0;

  std::fill(std::begin(rollingTemp), std::end(rollingTemp), 0);

  prevTime = millis();
  
  matrix.begin();

  //matrixFrame[0] = 0x0 | PWM_LED << 4 | PWM_LED << 16;
  //matrixFrame[1] = PWM_Target_LED | PWM_Target_LED << 12 | 0x00;
  //matrixFrame[2] = Temp_LED >> 4| Temp_LED << 8 | 0x00 << 24;

  matrixFrame[0] = PWM_LED << 8 | PWM_LED << 20;
  matrixFrame[1] = PWM_Target_LED << 4| PWM_Target_LED << 16;
  matrixFrame[2] = Temp_LED | Temp_LED << 12;

  matrix.loadFrame(matrixFrame);

  delay(3000);

  Serial.print("Program Start");
  Serial.println("");


}


unsigned short percentToMatrix(int percent){
  int scale = (int)(percent * 1.2 / 10);
  unsigned short output = 0;
  for (int i=0; i < scale; i++) output += pow(2, i);
  return output;
}

int temperatureToPercent(float inpTemp) {
  if (inpTemp >= TEMP_MAX) return 100;
  else if (inpTemp <= TEMP_MIN) return 0;
  else return ((inpTemp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN) * 100);
}

void matrixControl(){
  /* Updates the LED Matrix to display 3 bars which display the relative values of Temperature, PWMDuty and Target_PWMDuty
  */

  PWM_LED = percentToMatrix(PWMDuty_int);
  PWM_Target_LED = percentToMatrix(Target_PWMDuty_int);
  Temp_LED = percentToMatrix(temperatureToPercent(avgTemp));

  matrixFrame[0] = PWM_LED << 8 | PWM_LED << 20;  
  matrixFrame[1] = PWM_Target_LED << 4| PWM_Target_LED << 16;
  matrixFrame[2] = Temp_LED | Temp_LED << 12;
  matrix.loadFrame(matrixFrame);
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
  //Below TEMP_MIN C, the fan runs at minimum speed
  //Above TEMP_MAX C, the fan runs at full speed
  //In between, the fan runs at a relative value between 20 and 100%

  if (avgTemp < TEMP_MIN) {
    Target_PWMDuty = 20.0;
  } 
  else if (avgTemp > TEMP_MAX) {     
    Target_PWMDuty = 100.0;
  }
  else {
    Target_PWMDuty = 20 + ((avgTemp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN) * 80);
  }

  if (abs(Target_PWMDuty - PWMDuty) > PWMDuty_Max_Change) {

    if (Target_PWMDuty > PWMDuty) {
      PWMDuty += PWMDuty_Max_Change;
    } else {
      PWMDuty -= PWMDuty_Max_Change;
    }

  } else {
    PWMDuty = Target_PWMDuty;
  }
  setPWMDuty(PWMDuty);

  PWMDuty_int = static_cast<int>(PWMDuty);
  Target_PWMDuty_int = static_cast<int>(Target_PWMDuty);

  setPWMDuty(PWMDuty);
  Serial.print("PWM Duty: ");
  Serial.println(PWMDuty_int);
  Serial.print("Target PWM Duty: ");
  Serial.println(Target_PWMDuty_int);
}



void loop() {
  curTime = millis();
  if (curTime - prevTime > Fan_Interval) {
    prevTime = curTime;
    temperature();
    update_avg_temp();
    fanControl();
    matrixControl();
  }

  delay(1000);
  Serial.println("");


}




