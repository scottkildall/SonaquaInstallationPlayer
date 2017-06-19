/*
 *  SonaquaInstallationPlayer
 *  by Scott Kildall
 *  
 *  EC Water Sensor to play sounds as an installation
 *  Designed for Arudino Mega, which should have 6 channels of playback available
 *  
 *  Adapted from this Hackaday project
 *  https://hackaday.io/project/7008-fly-wars-a-hackers-solution-to-world-hunger/log/24646-three-dollar-ec-ppm-meter-arduino
 *  
 *  Uses the Tone library to manage multiple speakers
 */
#include "Tone.h"
#include "MSTimer.h"
#include "Adafruit_LEDBackpack.h"

//-- conditional defines
#define LED_TEST          // will just loop LED on pins 52 (ground), 53 (+)
//#define EC_TEST             // will test EC on pins A0 (probe), 7 (EC Power)
//#define TONE_TEST         // go through each, test the tones (maybe add a button for this...)


#define RANDOM_CHANCE (10)    // out of 1000
#define NUM_ACTIVE_CHANNELS (5)

// You can declare the tones as an array
const int numChannels = 5;
Tone tonePlayer[numChannels];

const int startSeakerPin = 8;

 //#define ECPin (A0)    

boolean isOn[numChannels];

// these need to proceed numerically, e.g. A0, A1, A2...
const int startECPin (A0);

const int startPowerPin (30);      // also numerically


unsigned int rawEC[numChannels];

MSTimer toneTimer[numChannels];
const int defaultToneTime = 1000;


Adafruit_7segment matrix = Adafruit_7segment();

#ifdef LED_TEST
  #define LEDTestPin (53)
  #define LEDTestGroundPin (52)
#endif

#ifdef EC_TEST
  #define ECTestPowerPin (6)
  #define ECTestProbePin (A0)  

  #ifndef LED_TEST
    #define LEDTestGroundPin (52)
  #endif
#endif

void setup(void)
{
  Serial.begin(115200);
Serial.println("---------- Sonaqua Installation Player ----------" );
  Serial.println("---------- by Scott Kildall ----------" );

  
 for( int i = 0; i < numChannels; i++ ) {
  tonePlayer[i].begin(startSeakerPin + i);
  isOn[i] = false;
 }


   for( int i = 0; i < numChannels; i++ )
    pinMode(startECPin + i,INPUT);

   //Setting pins for sourcing current
  for( int i = 0; i < numChannels; i++ ) 
    pinMode(startPowerPin + i,OUTPUT);


  // Initialize times
  for( int i = 0; i < numChannels; i++ ) 
    toneTimer[i].setTimer(defaultToneTime);

    
    randomSeed(analogRead(A15));

    
  matrix.begin(0x70);
 
  matrix.print(9999, DEC);
  matrix.writeDisplay();


#ifdef LED_TEST
  pinMode(LEDTestPin, OUTPUT); 
  pinMode(LEDTestGroundPin, OUTPUT);
  digitalWrite(LEDTestGroundPin, LOW);
#endif

#ifdef EC_TEST
  pinMode(ECTestProbePin,INPUT);
  pinMode(ECTestPowerPin,OUTPUT);                // set pin for sourcing current
  digitalWrite(LEDTestGroundPin, LOW);
#endif
}

void loop(void)
{
#ifdef EC_TEST
  int rawEC = getEC(ECTestPowerPin,ECTestProbePin);
  
  matrix.print(rawEC, DEC);
  matrix.writeDisplay();
  delay(100);

  #ifndef LED_TEST
    return;
  #endif
#endif


#ifdef LED_TEST
  int delayTime = 500;
  
  #ifdef EC_TEST
    delayTime = 50;
  #endif
  
  digitalWrite(LEDTestPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delayTime*10);                       // wait for a second
  digitalWrite(LEDTestPin, LOW);    // turn the LED off by making the voltage LOW
  delay(delayTime);
  return;
#endif




      
  for( int i = 0; i < numChannels; i++ ) {
    //rawEC[i] = getEC(i); 
    /*Serial.print("EC [");
    
    Serial.print(i);
    Serial.print("] = " );
    Serial.println(rawEC[i]);
*/
    if( isOn[i] == false ) {
      if( random(1000) < RANDOM_CHANCE ) {
       isOn[i] = true;
        toneTimer[i].start();
        toneTimer[i].setTimer(random(500,4000));
       }
    }
  }
    

  int toneValue;
/*
  for( int i = 0; i < NUM_ACTIVE_CHANNELS; i++ ) {
    if( rawEC[i] > 1000 ) {
       tonePlayer[i].stop();
    }
    else {
     //  toneValue = (rawEC[i]*2 - 700);
     toneValue = (600*2 - 700);
     
        if( toneValue < 100 )
          toneValue = 100;

        toneValue += random(10);

        if( isOn[i] ) {
          if( toneTimer[i].isExpired() ) {
              tonePlayer[i].stop();
              isOn[i] = false;
          }
          else
            tonePlayer[i].play(toneValue);

             matrix.print(i+1, DEC);
            matrix.writeDisplay();
        }
        //
    }
  }
*/

  delay(150);
}


/*
unsigned int getEC(int channelNum){
  unsigned int raw;
 
  digitalWrite(startPowerPin + channelNum,HIGH);
  raw= analogRead(startECPin + channelNum);
  raw= analogRead(startECPin + channelNum);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(channelNum + channelNum,LOW);
 
 return raw;
}
*/

//-- Sample EC using Analog pins, returns 0-1023
//-- powerPin is a digital pin, probe pin is analog
unsigned int getEC(int powerPin, int probePin){
  unsigned int raw;
 
  digitalWrite(powerPin,HIGH);

  // This is not a mistake, First reading will be low beause of charged a capacitor
  raw= analogRead(probePin);
  raw= analogRead(probePin);   
  
  digitalWrite(powerPin,LOW);
 
 return raw;
}

