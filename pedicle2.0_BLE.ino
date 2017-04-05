#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "Adafruit_VL6180X.h"
#include "HX711.h"
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#define BNO055_SAMPLERATE_DELAY_MS (100)

#define torque_dout A0
#define torque_clk A1
#define torque_factor 29480

#define thrust_dout A2
#define thrust_clk A3
#define thrust_factor 45480

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_VL6180X vl = Adafruit_VL6180X();
Adafruit_BNO055 bno = Adafruit_BNO055(55);
HX711 torque(torque_dout, torque_clk);
HX711 thrust(thrust_dout, thrust_clk);  
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

String torqueVal, thrustVal, x, y, z, range;

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

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);

  // LED Activity command is only supported from 0.6.6
    if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
    {
      // Change Mode LED Activity
      Serial.println(F("******************************"));
      Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
      ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
      Serial.println(F("******************************"));
    }
  }
}

void loop(void)
{
  String t = String(micros());
  range = String(vl.readRange());
  
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  x = String(event.orientation.x);
  y = String(event.orientation.y);
  z = String(event.orientation.z);

  torqueVal = String(torque.get_units());
  thrustVal = String(thrust.get_units());

  //String data = x + ", " + y + ", " + z + ", " + torqueVal + ", " + thrustVal + ", " + range;
  String data = t + ";" + x + " " + y + " " + z + " " + torqueVal + " " + thrustVal + " " + range + " ";
  //String data = " x: " + x + " y: " + y + " z: " + z + " torque: " + torqueVal + " thrust: " + thrustVal + " range: " + range;

  ble.print("AT+BLEUARTTX=");
  ble.println(data);
  
  /* Wait the specified delay before requesting nex data */
  delay(BNO055_SAMPLERATE_DELAY_MS);
}

bool getUserInput(char buffer[], uint8_t maxSize)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!Serial.available()) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (Serial.available()) );

  return true;
}
