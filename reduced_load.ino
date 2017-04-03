#include "HX711.h"

// HX711.DOUT	- pin A0-torque, A2-thrust
// HX711.PD_SCK	- pin A1-torque, A3-thrust
// torque factor: 29480
// thrust factor: 45480

#define torque_dout A0
#define torque_clk A1
#define torque_factor 29480

#define thrust_dout A2
#define thrust_clk A3
#define thrust_factor 45480
 
HX711 torque(torque_dout, torque_clk);
HX711 thrust(thrust_dout, thrust_clk);	

float torqueVal, thrustVal;

void setup() {
  Serial.begin(38400);
  
  torque.set_scale(torque_factor);
  torque.tare();	
  thrust.set_scale(thrust_factor);
  thrust.tare();			        
}

void loop() {
  torqueVal = torque.get_units();
  thrustVal = thrust.get_units();
  Serial.print("Torque: ");
  Serial.println(torqueVal);
  Serial.print("Thrust: ");
  Serial.println(thrustVal);

  delay(10);
}
