// this is the sketch that triggers the warning when the device
// is being worn by someone
#include <NXPMotionSense.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Audio.h>
#include <SerialFlash.h>
#include <play_sd_mp3.h>
#define PROP_AMP_ENABLE    5
#define FLASH_CHIP_SELECT  6

// these make the mp3 play, do not alter
AudioPlaySdMp3           playMp31; //xy=154,422
AudioMixer4              mixer1;         //xy=327,432
AudioOutputAnalog        dac1;           //xy=502,412
AudioConnection          patchCord1(playMp31, 0, mixer1, 0);
AudioConnection          patchCord2(playMp31, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, dac1);

// these make the IMU work, do not alter
NXPMotionSense imu;
NXPSensorFusion filter;

// to alter performance:
//
// 1 - change the value that DELTA is equal to to tweak 
//     sensitivity of warning trigger,
//     the smaller DELTA is, the more sensitive the trigger
//
// 2 - change the text INSIDE THE QUOTES of the mp3_file_name[]
//     varaible to play a different mp3 file, text should read
//     exactly as file is named
//
// <<Warning!>> the above are the only things in the code that can
// easily be changed without impacting the overall function
// of the code, nothing else should need modification <<Warning!>>

float init_ang = 0;           // initial angle of body

float curr_ang = 0;           // current angle of body

float diff_ang = 0;           // difference in angle from initial 
                              // to current

const float DELTA = 30;       // desired limit of diff_ang

int init_flag = 0;            // allows init_ang to be set

int curr_flag = 300;          // triggers reset of curr_ang
                              // after a warning is triggered

char mp3_file_name[] = """monoWarning.mp3"""; //this is the file
                                              //that is played when
                                              //a warning is triggered



void setup() {
  AudioMemory(8); //4
  delay(2000);

  // Start SerialFlash
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    while (1)
      {
        Serial.println ("Cannot access SPI Flash chip");
        delay (1000);
      }
  }

  //this sets volume, note that this is maxed
  //volume of mp3 file should be tweaked to increase or decrease
  //sound level of warning
  mixer1.gain(0, 1);
  mixer1.gain(1, 1);

  //Start Amplifier
  pinMode(PROP_AMP_ENABLE , OUTPUT);
  digitalWrite(PROP_AMP_ENABLE , 1); 
  imu.begin();
  filter.begin(100);
}

//this function is used to play the mp3 file of warning
void playFile(const char *filename)
{
  SerialFlashFile ff = SerialFlash.open(filename);
  Serial.print("Playing file: ");
  Serial.println(filename);  

  uint32_t sz = ff.size();
  uint32_t pos = ff.getFlashAddress();
  
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playMp31.play(pos,sz);

  // Simply wait for the file to finish playing.
  while (playMp31.isPlaying()) {yield();}
  Serial.println("Finished...");
}

void loop() {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
  float roll, pitch, heading;

    if (imu.available()) {
      // Read the motion sensors
      imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);

      // Update the SensorFusion filter
      filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

      // this updates the variable roll that corresponds to the
      // angle of interest
      roll = filter.getRoll();

      // this sets the body's initial positioning, only happens
      // once when device is first turned on
      if (init_flag < 500) {
        init_ang = roll;
        curr_ang = roll;
        init_flag++;
      }

      // after a warning is triggered, this resets curr_ang
      if (curr_flag < 300) {
        curr_ang = roll;
        curr_flag++;
      }

      // this keeps curr_pos up to date with body's positioning
      curr_ang = roll;
      Serial.print("initial angle ="); Serial.println(init_ang);
      Serial.print("current angle ="); Serial.println(curr_ang);

      // once curr_ang is up to date with body's positioning
      // diff_ang is calculated
      if (curr_flag == 300) {
        diff_ang = curr_ang - init_ang;
        Serial.print("current difference in angle ="); Serial.println(diff_ang);
      }

      // once diff_ang becomes greater than or equal to DELTA
      // the audio warning is triggered, and curr_flag is set to
      // zero so that curr_ang resets
      if (diff_ang >= DELTA) {
        Serial.println("WARNING!!!");
        playFile(mp3_file_name);
        curr_flag = 0;
        diff_ang = 0;
      }
   }
}
