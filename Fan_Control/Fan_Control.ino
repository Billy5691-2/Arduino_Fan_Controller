
// Temperature control variables
const int thermistorPin = 0; //Pin of Thermistor output
int rawTempVoltage; // Raw output from thermistor pin
const loat r1 = 10000; // Fixed resistor value, 10,000 Ohms
float logR2, r2, temp;
const float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07; //Stein Heart Equation Values


// Fan control variables
const int PWM_FREQ_HZ = 25000;
const int OC1B_PIN = 10;
const int temp_lower = 20;
const int temp_upper = 40;
float PWMDuty;
int PWMDuty_int = static_cast<int>(PWMDuty);



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(OC1B_PIN, OUTPUT);
  PWMDuty = 20.0;
  setPwmDuty(PWMDuty);
  delay(2000);

}

void temperature() {
  //Reads the voltage from an analog pin connected to a thermistor and converts it to a temperature in degrees C
  rawTempVoltage = analog.Read(thermistorPin);

  r2 = r1 * (1023.0 / (float)rawTempVoltage - 1);
  logR2 = log(r2);
  temp = (1.0 / (A + B*logR2 + C*logR2*logR2*logR2));
  temp = temp - 273.15;
  Serial.print("Temperature: ");
  Serial.print(T);
  Serial.println(" C");

  //return temp;
}

void fanControl() {
  //Change the PWM duty cycle to control fan speed
  //Changes speed based on temperature
  //Below 20C, the fan runs at minimum speed
  //Above 40C, the fan runs at full speed
  //In between, the fan runs at a relative value between 20 and 100%

  if (temp < temp_lower) {
    PWMDuty = 20.0;
  } 
  else if (temp > temp_upper) {
  }     
    PWMDuty = 100.0;
  else {
    PWMDuty = 20 + (temp / temp_upper * 80);
    
  }
  setPWMDuty(PWMDuty_int);
  Serial.print("PWM Duty: ");
  Serial.println(PWMDuty_int);
}



void loop() {
  // put your main code here, to run repeatedly:

  temperature();
  fanControl();


}
