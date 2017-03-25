#include <HX711.h>
#include "Arduino.h"

#if defined(ARDUINO_ARCH_SAMD) || defined(__SAM3X8E__)
  // use pin 18 with Due, pin 1 with Zero or M0 Pro 
  #define lcd Serial1 
#else
  #include <SoftwareSerial.h>
  // Create a software serial port!
  SoftwareSerial lcd = SoftwareSerial(0,2);
#endif

#define calibration_factor -9160.0 //Value found with SparkFun_HX711_Calibration sketch
#define DOUT  A1
#define CLK  A0

float max_force = 0;
int flag = 0;
int q = 0;

HX711 scale(DOUT, CLK);

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  scale.set_scale(calibration_factor);
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

  Serial.println("Readings:");
  
  /*Set up for LCD Display*/
  lcd.begin(9600);  
  
  // set the size of the display if it isn't 16x2 (you only have to do this once)
  lcd.write(0xFE);
  lcd.write(0xD1);
  lcd.write(16);  // 16 columns
  lcd.write(2);   // 2 rows
  delay(10);       
  // we suggest putting delays after each command to make sure the data 
  // is sent and the LCD is updated.

  // set the contrast, 200 is a good place to start, adjust as desired
  lcd.write(0xFE);
  lcd.write(0x50);
  lcd.write(200);
  delay(10);       
  
  // set the brightness - we'll max it (255 is max brightness)
  lcd.write(0xFE);
  lcd.write(0x99);
  lcd.write(255);
  delay(10);       
  
  // turn off cursors
  lcd.write(0xFE);
  lcd.write(0x4B);
  lcd.write(0xFE);
  lcd.write(0x54);
  
  // clear screen
  lcd.write(0xFE);
  lcd.write(0x58);
  delay(10);   // we suggest putting delays after each command 
  
  // go 'home'
  lcd.write(0xFE);
  lcd.write(0x48);
  delay(10);   // we suggest putting delays after each command 
  
  delay(1000);
}

//uint8_t red, green, blue;

void loop() {
  float force = scale.get_units();
  
  if (max_force < force) {
    max_force = force;
  }
  Serial.print("Current Force = ");
  Serial.println(force);
  Serial.print("Max Force = ");
  Serial.println(max_force);
  

  lcd.print(force);
  lcd.println();

  // hit any key to exit and print to LCD
  if (Serial.available() > 0) {
    q = Serial.read();
    while (q != 0) {
      lcd.print(max_force);
      lcd.println();
      delay(10000);
    }
  }

  delay(100); 
}
