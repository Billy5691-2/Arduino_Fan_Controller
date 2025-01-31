#include <pwm.h>

// Temperature control variables
const int thermistorPin = 0; //Pin of Thermistor output
int rawTempVoltage; // Raw output from thermistor pin
const float r1 = 10000; // Fixed resistor value, 10,000 Ohms
float logR2, r2, temp;
const float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07; //Stein Heart Equation Values


// Fan control variables
const word PWM_FREQ_HZ = 25000;
const byte GTIOC7B_PIN = 9;
//const word TCNT1_TOP = 16000000/(2*PWM_FREQ_HZ);

PwmOut pwm(D9);

//const byte OC1B_PIN = 10;
//const int SETTEMPINTERVAL = 30000;


const int Temp_Lower = 20;
const int Temp_Upper = 40;
float PWMDuty;
int PWMDuty_int;
//int PWMDuty_int = static_cast<int>(PWMDuty);



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Clear Timer Settings:
  //GTIOC7B = 0;
  //TCCR1B = 0;
  //TCNT1 = 0;

  //TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  //TCCR1B |= (1 << WGM13) | (1 << CS10);
  //ICR1 = TCNT1_TOP;
  
  //pinMode(GTIOC7B_PIN, OUTPUT);
  pwm.begin(PWM_FREQ_HZ, 0.0f);
  PWMDuty = 20.0;
  delay(3000);
  setPWMDuty(PWMDuty);
  delay(2000);

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

  //return temp;
}

void setPWMDuty(float duty){
  //OCR1A = (word) (duty*TCNT1_TOP)/100;
  //analogWrite(GTIOC7B_PIN, (int) (duty*TCNT1_TOP)/100);
  pwm.pulse_perc(duty);
}

void fanControl() {
  //Change the PWM duty cycle to control fan speed
  //Changes speed based on temperature
  //Below 20C, the fan runs at minimum speed
  //Above 40C, the fan runs at full speed
  //In between, the fan runs at a relative value between 20 and 100%

  if (temp < Temp_Lower) {
    PWMDuty = 20.0;
  } 
  else if (temp > Temp_Upper) {     
    PWMDuty = 100.0;
  }
  else {
    PWMDuty = 20 + ((temp - Temp_Lower) / (Temp_Upper - Temp_Lower) * 80);
  }
  PWMDuty_int = static_cast<int>(PWMDuty);

  setPWMDuty(PWMDuty);
  Serial.print("PWM Duty: ");
  Serial.println(PWMDuty_int);
}



void loop() {
  // put your main code here, to run repeatedly:

  temperature();
  fanControl();
  delay(500);


}




