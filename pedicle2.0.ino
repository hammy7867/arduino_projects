#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "Adafruit_VL6180X.h"
#include "HX711.h"

#define BNO055_SAMPLERATE_DELAY_MS (100)

#define torque_dout A0
#define torque_clk A1
#define torque_factor 29480

#define thrust_dout A2
#define thrust_clk A3
#define thrust_factor 45480

Adafruit_VL6180X vl = Adafruit_VL6180X();
Adafruit_BNO055 bno = Adafruit_BNO055(55);
HX711 torque(torque_dout, torque_clk);
HX711 thrust(thrust_dout, thrust_clk);  

float torqueVal, thrustVal;

void setup(void)
{
  Serial.begin(115200);
  
  while (!Serial) {
    delay(1);
  }
  bno.begin();
  vl.begin();

  delay(1000);
  bno.setExtCrystalUse(true);

  torque.set_scale(torque_factor);
  torque.tare();  
  thrust.set_scale(thrust_factor);
  thrust.tare();
}

void loop(void)
{
  uint8_t range = vl.readRange();
  //uint8_t status = vl.readRangeStatus();

  Serial.print("Range: "); Serial.println(range);
  
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(event.orientation.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");
  Serial.println(event.orientation.z, 4);

  torqueVal = torque.get_units();
  thrustVal = thrust.get_units();
  Serial.print("Torque: ");
  Serial.println(torqueVal);
  Serial.print("Thrust: ");
  Serial.println(thrustVal);

  /* Wait the specified delay before requesting nex data */
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
