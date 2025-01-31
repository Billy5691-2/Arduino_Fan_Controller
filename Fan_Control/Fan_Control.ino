
// Temperature control variables
const int thermistorPin = 0; //Pin of Thermistor output
int rawTempVoltage; // Raw output from thermistor pin
const loat r1 = 10000; // Fixed resistor value, 10,000 Ohms
float logR2, r2, temp;
const float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07; //Stein Heart Equation Values


// Fan control variables
const int PWM_FREQ_HZ = 25000;
const int OC1B_PIN = 10;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(OC1B_PIN, OUTPUT);
  setPwmDuty(20);
  delay(2000);

}

float temperature() {
  //Reads the voltage from an analog pin connected to a thermistor and converts it to a temperature in degrees C
  rawTempVoltage = analog.Read(thermistorPin);

  r2 = r1 * (1023.0 / (float)rawTempVoltage - 1);
  logR2 = log(r2);
  temp = (1.0 / (A + B*logR2 + C*logR2*logR2*logR2));
  temp = temp - 273.15;
  Serial.print("Temperature: ");
  Serial.print(T);
  Serial.println(" C");

  return temp;
}

void fanControl(float temp) {
  
}



void loop() {
  // put your main code here, to run repeatedly:

}
