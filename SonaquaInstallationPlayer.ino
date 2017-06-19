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
 */
 
#include "Tone.h"
#include "MSTimer.h"
#include "Adafruit_LEDBackpack.h"

//-- conditional defines
//#define LED_TEST          // will just loop LED on pins 52 (ground), 53 (+)
//#define EC_TEST             // will test EC on pins A0 (probe), 7 (EC Power)
//#define TONE_TEST         // go through each, test the tones (maybe add a button for this...)
//#define CHANNEL_TEST      // will test LEDS, speakers and EC for each channel, progressively
//#define NO_TONE
#define SERIAL_DEBUG

#define RANDOM_CHANCE (4)    // out of 1000, this will be running 4 Arduinos, so essentally try 3 instead 

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


boolean isPlaying;

int currentChannel;
int currentToneValue;
int currentLEDPin;

const long toneTimerMinValue = 500;
const long toneTimerMaxValue = 3000;


MSTimer toneTimer;
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
  isPlaying = false;
  currentChannel = 0;
  
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("---------- Sonaqua Installation Player ----------" );
  Serial.println("---------- by Scott Kildall ----------" );
#endif


  //-- initialize tone players
  for( int i = 0; i < numChannels; i++ ) {
    tonePlayer[i].begin(startSpeakerPositivePin + (i*2) );
  }

  lineOut.begin(lineOutPositivePin);


   for( int i = 0; i < numChannels; i++ )
    pinMode(startECPin + (i*2),INPUT);

   //Setting pins for sourcing current
  for( int i = 0; i < numChannels; i++ ) 
    pinMode(startPowerPin + (i*2),OUTPUT);


  // Initialize times
  toneTimer.setTimer(defaultToneTime);

    
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

  //-- check if timer is playing
  if( isPlaying ) {
    if( toneTimer.isExpired() ) {
       #ifdef SERIAL_DEBUG
        Serial.println("--------------------------");
        Serial.println("-> Stopping Sound <- " );
        Serial.print("Channel Num = " );
        Serial.println(currentChannel);
      #endif
    
      tonePlayer[currentChannel].stop();
      digitalWrite(currentLEDPin,LOW);
      isPlaying = false;
    }
    else {
      // 1 in 1000 chance, better done through a timer, but this creates an interestng sound   
      if( random(1000) < 1 ) {

        
//        #ifdef SERIAL_DEBUG
//          Serial.print("Randomizing: ");
//          Serial.println(currentToneValue + randValue);
//        #endif
      
        currentToneValue = makeToneFromEC(getEC( getPowerPin(currentChannel), getECPin(currentChannel) ));
        tonePlayer[currentChannel].play(currentToneValue);
        matrix.print(currentToneValue, DEC);
        matrix.writeDisplay();
      
       }
    }

    // exit loop()
    return;
  }

  //-- at this point, there is no tone playing
  if( random(1000) < RANDOM_CHANCE ) {

    currentChannel = random(0, numChannels );

    #ifdef SERIAL_DEBUG
      Serial.println("--------------------------");
      Serial.println("-> Playing Sound <- " );
      Serial.print("Channel Num = " );
      Serial.println(currentChannel);
    #endif
    
    int ecValue = getEC( getPowerPin(currentChannel), getECPin(currentChannel) );

    #ifdef SERIAL_DEBUG
      Serial.print("EC Value = " );
      Serial.println(ecValue);
    #endif

    int toneValue = makeToneFromEC(ecValue);

    #ifndef NO_TONE
      matrix.print(toneValue, DEC);
      matrix.writeDisplay();
    
      tonePlayer[currentChannel].play(toneValue);
    #endif
    
    toneTimer.setTimer(random(toneTimerMinValue,toneTimerMaxValue));
    toneTimer.start();

    isPlaying = true;

    currentLEDPin = startLEDPositivePin + (currentChannel*2);
    currentToneValue = toneValue;
    digitalWrite(currentLEDPin,HIGH);
  }

  // 50 ms delay loop
  delay(50);
}


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

