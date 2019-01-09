#include <math.h>
#include <Servo.h>
float therm = 0;
float R25 = 100000;
float T25 = 298.15;
float Vref = 5;
float Vmeas = 0;
float Rmeas = 0;
float Tmeas = 0;
int B = 4092;
float T = 0;

Servo pitch;
Servo yaw;

int potP = 1;
int potY = 2;

int valP = 0;
int valY = 0;

void setup() {
 Serial.begin(9600);
 pitch.attach(9);
 yaw.attach(10);

}

void loop() {
  Vmeas = analogRead(therm);
  valP = analogRead(potP);
  valY = analogRead(potY);

  valP = map(valP, 0, 1023, 0, 18+-0);
  valY = map(valY, 0, 1023, 0, 180);

  Vmeas = 5 * (Vmeas/1024);
  Rmeas = R25 *((Vref/Vmeas)-1);

  Tmeas = 1/((log(Rmeas/R25)/B) + (1/T25));
  T = Tmeas - 273;

  pitch.write(valP);
  delay(15);
  yaw.write(valY);
  delay(15);
  
  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.println(" C"); 
}
