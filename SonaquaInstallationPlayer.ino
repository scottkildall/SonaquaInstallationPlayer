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
 *  
 *  Adafruit Backpack for visual debugging and feedback, the pins used are:
 *  5V out (red wire), Ground (black wire), SDA (green) and SCL (yellow)
 *
 *  MSTimer code is my own, feel free to use/copy/modify
 *
 *
 *
 */
 
#include "Tone.h"
#include "MSTimer.h"
#include "Adafruit_LEDBackpack.h"

//-- conditional defines
//#define LED_TEST          // will just loop LED on pins 52 (ground), 53 (+)
//#define EC_TEST             // will test EC on pins A0 (probe), 7 (EC Power)
//#define TONE_TEST         // go through each, test the tones (maybe add a button for this...)
#define CHANNEL_TEST      // will test LEDS, speakers and EC for each channel, progressively
//#define NO_TONE

#define RANDOM_CHANCE (10)    // out of 1000

// You can declare the tones as an array
const int numChannels = 4;
Tone tonePlayer[numChannels];
Tone lineOut;


const int startLEDPositivePin = 31;       // these will be numbered 31, 33, 35, 37, 39
const int startLEDGroundPin = 30;         // these will be numbered 30, 32, 34, 36, 38

const int startSpeakerPositivePin = 43;   // these will be numbered 43, 45, 47, 49, 51
const int startSpeakerGroundPin = 42;     // these will be numbered 42, 44, 46, 48, 50

const int lineOutPositivePin = 9;
const int lineOutGroundPin = 8;

const int startPowerPin = 6;            // these will be numbered 6, 5, 4, 3, 2, descending order due to bridge
const int startECPin = A0;              // these will be numbered A0, A1, A3

const int ecValueNoPlayThreshhold = 950;    // value under which we will not play a tone
 //#define ECPin (A0)    

boolean isOn[numChannels];



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
  tonePlayer[i].begin(startSpeakerPositivePin + (i*2) );
  isOn[i] = false;
 }

  lineOut.begin(lineOutPositivePin);


   for( int i = 0; i < numChannels; i++ )
    pinMode(startECPin + (i*2),INPUT);

   //Setting pins for sourcing current
  for( int i = 0; i < numChannels; i++ ) 
    pinMode(startPowerPin + (i*2),OUTPUT);


  // Initialize times
  for( int i = 0; i < numChannels; i++ ) 
    toneTimer[i].setTimer(defaultToneTime);

    
    randomSeed(analogRead(A15));

    
  matrix.begin(0x70);
 
  matrix.print(7777, DEC);
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

  // LED outputs
  for( int i = 0; i < numChannels; i++) {
    pinMode(startLEDPositivePin + (i*2), OUTPUT);
    digitalWrite(startLEDPositivePin, LOW);

    pinMode(startSpeakerPositivePin + (i*2), OUTPUT);
    pinMode(lineOutPositivePin, OUTPUT);
  }

  // All ground pins
  for( int i = 0; i < numChannels; i++ ){
    pinMode(startLEDGroundPin + (i*2), OUTPUT);
    digitalWrite(startLEDGroundPin, LOW);

    pinMode(startSpeakerGroundPin + (i*2), OUTPUT);
    digitalWrite(startSpeakerGroundPin, LOW);

    pinMode(lineOutGroundPin, OUTPUT);
    digitalWrite(lineOutGroundPin, LOW);
  }

  // EC pins
  for( int i = 0; i < numChannels; i++ )
    pinMode( getPowerPin(i), OUTPUT );

   for( int i = 0; i < numChannels; i++ )
    pinMode(getECPin(i),INPUT);
}

void loop(void)
{
#ifdef CHANNEL_TEST
  channelTest();
  return;
#endif

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

//-- go through each channel, show LEDs, display on backpack
void channelTest() {
   int delayTime = 1000;
   
  // LED outputs
  for( int i = 0; i < numChannels; i++ ) {
    matrix.print(i, DEC);
    matrix.writeDisplay();
    delay(500);

    int ecValue = getEC( getPowerPin(i), getECPin(i) );
    matrix.print(ecValue, DEC);
    matrix.writeDisplay();
    
    digitalWrite(startLEDPositivePin + (i*2), HIGH);
    delay(delayTime);

    int toneValue = makeToneFromEC(ecValue);
    
    #ifndef NO_TONE
        //-- zero tone means that we over threshhold
       if( toneValue != 0 ) {
        tonePlayer[i].play(toneValue);
        lineOut.play(toneValue);
       }
    #endif
     
    digitalWrite(startLEDPositivePin + (i*2), LOW);
    delay(delayTime);

    tonePlayer[i].stop();
    lineOut.stop();
  }
}

//-- power are digital pins, start at 6, goes down from there
int getPowerPin(int channelNum) {
  return (startPowerPin - channelNum);
}

int getECPin(int channelNum) {
  if( channelNum == 0 )
    return A0;
  else if( channelNum == 1 )
    return A1;
  return (startECPin + channelNum);
}

int makeToneFromEC(int ecValue) {
  if( ecValue > ecValueNoPlayThreshhold )
      return 0;
      
  int toneValue = ecValue - 400;
  
  if( toneValue < 25 )
    toneValue = 25;
    
  return toneValue;
}

