// Magic Wheelchair - Guardians of the Galaxy Milano
//
// IMPORTANT - USE A NO-POWER USB CABLE IF THE TEENSY IS EXTERNALLY POWERED!
//
// reference: 
// IMPORTANT NOTE: 8.3 FILENAMES FOR WAV AUDIO FILES!
// IMPORTANT NOTE: WAV 44100 STEREO 16BIT
//

// DEBUGGING
// you can use the serial input with keys 1-9 to toggle
// key 0 will turn them all off so we don't use 0 here

// BUTTON FUNCTIONS FOR MILANO
// RED      ENGINE    ActionBoost   Turn off Idle sounds
// GREEN    WEAPON    Fire          ?
// YELLOW   SPEECH    Random        I am Groot
// BLUE     MUSIC     Next track    Stop
// note that speech & weapon share an audio channel


//TODO 
// create weapon animation
// Add more texture to startup? Maybe the sound then a specific voice clip?
// add an easter egg, maybe random I AM GROOT?
// Remove the "torpedo charge" concept - does not apply here 


bool debugOptions[10] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0};   //change default here, helpful for startup debugging

                                                        
const char *debugOptionsText[10] =  {"", "Input","Audio", "Action", "Peak Audio",
                                "Animation","Animation RGB Dump"};
                                
#define DEBUG_INPUT               1  //input functions 
#define DEBUG_AUDIO               2  //audio functions 
#define DEBUG_ACTION              3  //action functions 
#define DEBUG_PEAK                4  //Peak Audio functions 
#define DEBUG_ANIMATION           5  //Animation functions   //THIS MAY SLOW ANIMATION                            
#define DEBUG_ANIMATION_RGB       6  //Show Full Animation Frame RGB 
                                     //Does not require DEBUG_ANIMATION to be on
                                     //THIS MAY SLOW ANIMATION

 
 /*
 * Hardware Buttons
 */
 
#include "NButton.h"

#define NUM_BUTTONS 4
uint8_t buttonPins[NUM_BUTTONS] = { 35, 36, 38, 37 };
NButton buttons[NUM_BUTTONS] = { {0, buttonPins[0], true, true}, {1, buttonPins[1], true, true}, 
                                 {2, buttonPins[2], true, true}, {3, buttonPins[3], true, true} };
int buttonDebounce[NUM_BUTTONS] = {250, 100, 1000, 1000};
    
#define BUTTON_LIGHT_MUSIC    2
#define BUTTON_LIGHT_WEAPONA  5
#define BUTTON_LIGHT_SPEECH   29
#define BUTTON_LIGHT_ENGINE   30

uint8_t buttonLightPins[NUM_BUTTONS] = { BUTTON_LIGHT_MUSIC, BUTTON_LIGHT_WEAPONA,
                                         BUTTON_LIGHT_ENGINE, BUTTON_LIGHT_SPEECH  };

int keyHeld = 0;
unsigned long keyHeldDuration = 0;
#define KEY_HOLD_DURATION 1000


/*
 * Audio System Includes & Globals
 * Reference the Audio Design Tool - https://www.pjrc.com/teensy/gui/index.html
 * 
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav2;     //xy=163,192
AudioPlaySdWav           playSdWav1;     //xy=165,133
AudioPlaySdWav           playSdWav3;     //xy=165,251
AudioMixer4              mixer3;         //xy=347,337
AudioMixer4              mixer2;         //xy=348.16668701171875,238.3333282470703
AudioMixer4              mixer1;         //xy=350.0000305175781,146.1666717529297
AudioMixer4              mixer4;         //xy=351,422
AudioMixer4              mixer5;         //xy=351,503
AudioOutputI2S           i2s1;           //xy=578.1666564941406,152.6666717529297
AudioAnalyzePeak         peak1;          //xy=594,337
AudioAnalyzePeak         peak3;          //xy=595,502
AudioAnalyzePeak         peak2;          //xy=596,421
AudioConnection          patchCord1(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord2(playSdWav2, 0, mixer4, 0);
AudioConnection          patchCord3(playSdWav2, 1, mixer2, 1);
AudioConnection          patchCord4(playSdWav2, 1, mixer4, 1);
AudioConnection          patchCord5(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord6(playSdWav1, 0, mixer3, 0);
AudioConnection          patchCord7(playSdWav1, 1, mixer2, 0);
AudioConnection          patchCord8(playSdWav1, 1, mixer3, 1);
AudioConnection          patchCord9(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord10(playSdWav3, 0, mixer5, 0);
AudioConnection          patchCord11(playSdWav3, 1, mixer2, 2);
AudioConnection          patchCord12(playSdWav3, 1, mixer5, 1);
AudioConnection          patchCord13(mixer3, peak1);
AudioConnection          patchCord14(mixer2, 0, i2s1, 1);
AudioConnection          patchCord15(mixer1, 0, i2s1, 0);
AudioConnection          patchCord16(mixer4, peak2);
AudioConnection          patchCord17(mixer5, peak3);
AudioControlSGTL5000     sgtl5000_1;     //xy=106,55
// GUItool: end automatically generated code


#define NUM_CHANNELS       3     //SD Card playback is best to not exceed 3 simultaneous WAVs
                                 //There is an SD card test in the Audio examples

#define CHANNEL_MUSIC      0     //Only one item can play per channel
#define CHANNEL_ENGINE     1     //Use these to map sound types to a channel
#define CHANNEL_SPEECH     2     
#define CHANNEL_WEAPON     2

#define LEVEL_CHANNEL0    .1    //change these for relative channel levels
#define LEVEL_CHANNEL1    .3
#define LEVEL_CHANNEL2    .8  

#define STARTING_VOLUME       .4 //change this to reduce clipping in the main amp
#define MAX_VOLUME            1.0 
#define MIN_VOLUME            .2 
#define VOLUME_INCREMENT      .1 //how much the volume changes when using buttons

//I use this syntax so that I can leave the declarations above which come from the Audio Design tool
AudioPlaySdWav *channels[NUM_CHANNELS] = { &playSdWav1, &playSdWav2, &playSdWav3 };
String playQueue[NUM_CHANNELS];



AudioAnalyzePeak  *peakAnalyzers[NUM_CHANNELS] = { &peak1, &peak2, &peak3 };

#define NUM_BGM_WAVS          12    //format BCKGND#.WAV
#define NUM_WEAPONA_WAVS      3     //format WPNA#.WAV
#define NUM_WEAPONB_WAVS      3     //format WPNB#.WAV
#define NUM_ENGINE_WAVS       16    //format ENGINE#.WAV
#define NUM_IDLE_WAVS         2     //format IDLE#.WAV
#define NUM_VOICE_WAVS        4     //format VOICE#.WAV
#define NUM_STARTUP_WAVS      2     //format START#.WAV
#define NUM_GROOT_WAVS        16    //format GROOT#.WAV




//LED ALL THE THINGS!
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

#define LASER_NUM_LEDS 24          
#define LASER_DATA_PIN 32
CRGB laserLEDS[LASER_NUM_LEDS];
int laserFrame = 9999;
int torpedoFrame = 9999;

#define LASER_ANIMATION_HEIGHT 60
#define LASER_ANIMATION_WIDTH  16
byte laserAnimation[LASER_ANIMATION_HEIGHT * LASER_ANIMATION_WIDTH * 3];

#define TORPEDO_ANIMATION_HEIGHT 72
#define TORPEDO_ANIMATION_WIDTH  16
byte torpedoAnimation[TORPEDO_ANIMATION_HEIGHT * TORPEDO_ANIMATION_WIDTH * 3];

#define ENGINE_NUM_LEDS 64 
#define ENGINE_DATA_PIN 8
CRGB engineLEDS[ENGINE_NUM_LEDS]; 

#define SPEECH_NUM_LEDS 24 
#define SPEECH_DATA_PIN 26
CRGB speechLEDS[SPEECH_NUM_LEDS]; 

//WARNING - ADJUSTING THIS SETTING COULD LEAD TO 
//EXCESS CURRENT DRAW AND POSSIBLE SYSTEM DAMAGE
#define DEFAULT_BRIGHTNESS 64 //WARNING!!!!!!!!!
//DON'T DO IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//Show & Mode Globals

#include <Metro.h> //Include Metro library 

Metro bgmMetro = Metro(100);
Metro playQueueMetro = Metro(50);
Metro animationMetro = Metro(17); //approx 60 frames per second


bool bgmStatus = 0;           //0 = BGM off; 1 = BGM on
bool idleStatus = 1;        //0 = Engine off; 1 = Engine on
int   bgmTrack = 0;             //Starting track (requires N-1) for background music 
float mainVolume = STARTING_VOLUME;

//for USB host functions
#include "USBHost_t36.h"

USBHost myusb;
USBHub hub1(myusb);
USBHIDParser hid1(myusb);
KeyboardController keyboard1(myusb);


// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

#define SOURCE_KEY                    0
#define SOURCE_BUTTON                 1
#define SOURCE_BTN_DBLCLICK           2
#define SOURCE_BTN_LONGPRESS_START    3
#define SOURCE_BTN_LONGPRESS_DURING   4
#define SOURCE_BTN_LONGPRESS_STOP     5

#define ACTION_DO_NOT_USE             0 //just putting this here as a reminder to not use it :)

#define ACTION_WEAPONB               1
#define ACTION_WEAPONA               2
#define ACTION_SPEECH                3
#define ACTION_ENGINE                4
#define ACTION_BGM_TOGGLE            5          //BGM = BACKGROUND MUSIC
#define ACTION_BGM_NEXT              6
#define ACTION_IDLE_TOGGLE           7
#define ACTION_TORPEDO_CHARGE_START  8
#define ACTION_TORPEDO_CHARGE_DURING 9
#define ACTION_TORPEDO_CHARGE_STOP   10
#define ACTION_GROOT                 11
#define ACTION_FLASH_BUTTON          12



#define ACTION_PLAY_WAV               20
#define ACTION_PLAY_WAV_RND           21
#define ACTION_VOLUME_UP                 22
#define ACTION_VOLUME_DOWN               23

#define MAX_ACTION_ID                23 //change this if you add actions!
unsigned long lastActionTime[MAX_ACTION_ID]; 


/*
 * The ActionMap allows us to have multiple types of input events
 * that get mapped to an action. We can have a button press, or an HID keyboard
 * key, etc. that all go to the same action
 * 
 */

#define ACTION_MAP_SIZE 20

int ActionMap[][3] = {
  //src, key, action
  {SOURCE_KEY, 214, ACTION_WEAPONB},            //remote right
  {SOURCE_KEY, 211, ACTION_WEAPONA},              //remote left
  {SOURCE_KEY,  27, ACTION_SPEECH},               //remote up
  {SOURCE_KEY,  98, ACTION_ENGINE},             //remote down
  {SOURCE_KEY, 198, ACTION_BGM_TOGGLE},         //remote play
  
  {SOURCE_BUTTON, 0, ACTION_BGM_NEXT},              //blue button
  {SOURCE_BTN_DBLCLICK, 0, ACTION_BGM_TOGGLE},              
  {SOURCE_BTN_LONGPRESS_DURING, 0, ACTION_FLASH_BUTTON},            
  {SOURCE_BTN_LONGPRESS_STOP, 0, ACTION_BGM_TOGGLE},
  
  {SOURCE_BUTTON, 1, ACTION_WEAPONA},              //green button
  //{SOURCE_BTN_DBLCLICK, 1, ACTION_WEAPONB},              
  //{SOURCE_BTN_LONGPRESS_DURING, 1, ACTION_FLASH_BUTTON},            
  //{SOURCE_BTN_LONGPRESS_STOP, 1, ACTION_WEAPONB},
  
  {SOURCE_BUTTON, 3, ACTION_SPEECH},               //yellow button
  {SOURCE_BTN_DBLCLICK, 3, ACTION_SPEECH},               
  {SOURCE_BTN_LONGPRESS_DURING, 3, ACTION_FLASH_BUTTON},            
  {SOURCE_BTN_LONGPRESS_STOP, 3, ACTION_GROOT},            
  
  {SOURCE_BUTTON, 2, ACTION_ENGINE},             //red button
  {SOURCE_BTN_DBLCLICK, 2, ACTION_ENGINE},                        
  {SOURCE_BTN_LONGPRESS_DURING, 2, ACTION_FLASH_BUTTON}, 
  {SOURCE_BTN_LONGPRESS_STOP, 2, ACTION_IDLE_TOGGLE},            
  
  {SOURCE_KEY, 233, ACTION_VOLUME_UP},            //remote vol+
  {SOURCE_KEY, 234, ACTION_VOLUME_DOWN},          //remote vol-
}; //if you change this, don't forget to update the ACTION_MAP_SIZE

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Setup Started.");
  

  //setup audio system
  AudioMemory(128);
  sgtl5000_1.enable();
  sgtl5000_1.volume(mainVolume);

  //set relative volumes by channel
  mixer1.gain(0, LEVEL_CHANNEL0);
  mixer2.gain(0, LEVEL_CHANNEL0);
  mixer1.gain(1, LEVEL_CHANNEL1);
  mixer2.gain(1, LEVEL_CHANNEL1);
  mixer1.gain(2, LEVEL_CHANNEL2);
  mixer2.gain(2, LEVEL_CHANNEL2);

  //these mixers feed the peak analysis
  mixer3.gain(0, .8); //peak1 (bgm)
  mixer3.gain(1, .8);
  mixer4.gain(1, .8); //peak2 (engine)
  mixer4.gain(1, .8);
  mixer5.gain(1,  1); //peak3
  mixer5.gain(1,  1);

  //setup SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  //load laser animation BMP
  loadAnimationBMP("LASER.BMP", laserAnimation, LASER_ANIMATION_HEIGHT, LASER_ANIMATION_WIDTH);
  if (debugOptions[DEBUG_ANIMATION]) {
    printAnimation(laserAnimation, LASER_ANIMATION_HEIGHT, LASER_ANIMATION_WIDTH);
  }
  //load torpedo animation BMP
  loadAnimationBMP("TORPEDO.BMP", torpedoAnimation, TORPEDO_ANIMATION_HEIGHT, TORPEDO_ANIMATION_WIDTH);
  if (debugOptions[DEBUG_ANIMATION]) { 
    printAnimation(torpedoAnimation, TORPEDO_ANIMATION_HEIGHT, TORPEDO_ANIMATION_WIDTH);
  }

  //seed random function
  randomSeed(analogRead(0));

   //setup buttons & button light
  for (int btn = 0; btn< NUM_BUTTONS; btn++) {
    //pinMode(buttonPins[btn], INPUT_PULLUP);
    pinMode(buttonLightPins[btn], OUTPUT);  
 
    buttons[btn].attachClick(buttonClick);
    buttons[btn].attachDoubleClick(buttonDoubleClick);
    buttons[btn].attachLongPressStart(buttonLongPressStart);
    buttons[btn].attachLongPressStop(buttonLongPressStop);
    buttons[btn].attachDuringLongPress(buttonLongPress);
  }


  Serial.println("Starting up!");

  //play random STARTUP#.WAV
  String fn = "START";
  fn = fn + random (1, NUM_STARTUP_WAVS + 1) + ".WAV";
  playWAV( CHANNEL_MUSIC, fn);

/* 
 *  
  //startup button light animation
  int btnDelay = 200;
  int absBtn = 0;
  //int numCycles = 10;
  //for (int cycle = 0; cycle < numCycles; cycle++) { 
  
  do {
    for (int btn = 0; btn< NUM_BUTTONS; btn++) { 
      btnDelay = btnDelay - absBtn;
      if (btnDelay <0) btnDelay=0;
      digitalWrite(buttonLightPins[btn], HIGH);
      delay(btnDelay);  
      digitalWrite(buttonLightPins[btn], LOW);
      delay(btnDelay); 
      absBtn++;
    } 
  } while (channels[CHANNEL_MUSIC]->isPlaying());

*/
  //Setup LEDS
  
  //Validate the LED color order!
  LEDS.addLeds<WS2812SERIAL,  LASER_DATA_PIN,   BRG>(laserLEDS,   LASER_NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL,  ENGINE_DATA_PIN,  BRG>(engineLEDS,  ENGINE_NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL,  SPEECH_DATA_PIN,  BRG>(speechLEDS,  SPEECH_NUM_LEDS);


//FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB, DATA_RATE_MHZ(12)>(leds, NUM_LEDS);
 
    
  LEDS.setBrightness(DEFAULT_BRIGHTNESS);

  //set first LED to white at startup to show proper operation
  FastLED.clear();
  laserLEDS[0] = CRGB(255,255,255);
  engineLEDS[0] = CRGB(255,255,255);
  speechLEDS[0] = CRGB(255,255,255);
  
  FastLED.show();

  //Setup USB Host
  Serial.println("USB Host Setup");
  myusb.begin();
  keyboard1.attachPress(OnPress);
  keyboard1.attachRelease(OnRelease);
  keyboard1.attachExtrasPress(OnHIDExtrasPress);
  keyboard1.attachExtrasRelease(OnHIDExtrasRelease);
  
  delay(1000);
  Serial.println("Setup Complete.");
  printDebugOptions();
 
}

void loop() {
  //update all buttons
  for (int btn = 0; btn< NUM_BUTTONS; btn++) {
    buttons[btn].tick();
   }
    
  myusb.Task();

   if (bgmMetro.check() == 1) { // check if the metro has passed its interval 
      
      //set background music low while voice is playing
       if (channels[CHANNEL_SPEECH]->isPlaying()) { 
          mixer1.gain(CHANNEL_MUSIC, 0);
          mixer2.gain(CHANNEL_MUSIC, 0);
          //if (debugOptions[DEBUG_AUDIO]) Serial.println("Decreasing Background Music level from bgmMetro");
        }
        else { 
          mixer1.gain(CHANNEL_MUSIC, LEVEL_CHANNEL0);
          mixer2.gain(CHANNEL_MUSIC, LEVEL_CHANNEL0);
          //if (debugOptions[DEBUG_AUDIO]) Serial.println("Increasing Background Music level from bgmMetro");
        }
    
      //check on background music
      if (bgmStatus && !channels[CHANNEL_MUSIC]->isPlaying()) { 

        bgmTrack++;
        if (bgmTrack > NUM_BGM_WAVS) bgmTrack = 1;
        
        String fn = "BCKGND";
        fn = fn + bgmTrack + ".WAV";
        if (debugOptions[DEBUG_AUDIO]){
          Serial.println("Starting Background Music from bgmMetro");
        }
        playWAV( CHANNEL_MUSIC, fn);

      }
      
      //todo: move this to its own metro
      //check on idle
      if (!channels[CHANNEL_ENGINE]->isPlaying()) {
        
      if (debugOptions[DEBUG_AUDIO]) {
        Serial.println("Starting Background Idle Sound from bgmMetro");
      }
        String fn = "IDLE";
        fn = fn + random (1, NUM_IDLE_WAVS + 1) + ".WAV";
        playWAV(CHANNEL_ENGINE, fn);
      }
      
      
   } //end bgmMetro check

  if (playQueueMetro.check() == 1) { // check if theb metro has passed its interval
    for (int q = 0; q < NUM_CHANNELS; q++) {
      String fn = playQueue[q];
      if (fn.length() >0) {
        playQueue[q] = "";
        playWAV(q,fn); 
      }
    }
  } //end playQueueMetro check

  if (animationMetro.check() == 1) { // check if the metro has passed its interval
    //engine animation

    if (peakAnalyzers[CHANNEL_ENGINE]->available()) {
      float peak = peakAnalyzers[CHANNEL_ENGINE]->read();
      if (debugOptions[DEBUG_PEAK]) Serial.printf("Engine Peak: %f \n", peak);
      int peakbrt = map(peak,0,1,0,255);
      fill_solid(engineLEDS, ENGINE_NUM_LEDS, CRGB(peakbrt,0,0));
      analogWrite(BUTTON_LIGHT_ENGINE, peakbrt);     
    }
    else fill_solid(engineLEDS, ENGINE_NUM_LEDS, CRGB(random(16,24),0,0));

    if (peakAnalyzers[CHANNEL_MUSIC]->available()) {
      float peak = peakAnalyzers[CHANNEL_MUSIC]->read();
      if (debugOptions[DEBUG_PEAK]) Serial.printf("Music Peak: %f \n", peak);
      int peakbrt = map(peak,0,1,0,255);
      analogWrite(BUTTON_LIGHT_MUSIC, peakbrt);     
    }
    else analogWrite(BUTTON_LIGHT_MUSIC, 0);  
     
    //if weapons sounds are playing, then peak the appropriate button
    if (channels[CHANNEL_WEAPON]->isPlaying() ) {
      if (peakAnalyzers[CHANNEL_WEAPON]->available()) {
        float peak = peakAnalyzers[CHANNEL_WEAPON]->read();
        if (debugOptions[DEBUG_PEAK]) Serial.printf("Weapon Peak: %f \n", peak);
        int peakbrt = map(peak,0,1,0,255);
        unsigned long k = max(lastActionTime[ACTION_SPEECH], lastActionTime[ACTION_GROOT]);
        unsigned long l = lastActionTime[ACTION_WEAPONA];
        unsigned long t = max(lastActionTime[ACTION_WEAPONB], lastActionTime[ACTION_TORPEDO_CHARGE_START]);
                      t = max(t, lastActionTime[ACTION_TORPEDO_CHARGE_DURING]);
                      t = max(t, lastActionTime[ACTION_TORPEDO_CHARGE_STOP]);
                      

        if ( k > l && k > t) {
          //SPEECH was most recent
          analogWrite(BUTTON_LIGHT_SPEECH, peakbrt);
          analogWrite(BUTTON_LIGHT_WEAPONA, LOW); 
          fill_solid(speechLEDS, SPEECH_NUM_LEDS, CRGB(peakbrt,peakbrt*.8,0));
          
          //analogWrite(BUTTON_LIGHT_TORPEDO, LOW);
        }
        else if ( l > k && l > t) {
          analogWrite(BUTTON_LIGHT_WEAPONA, peakbrt);
          fill_solid(speechLEDS, SPEECH_NUM_LEDS, CRGB(0,0,0));
          //analogWrite(BUTTON_LIGHT_TORPEDO, LOW);   
        }
        else if ( t > k && t > l) {
          //analogWrite(BUTTON_LIGHT_TORPEDO, peakbrt);
          analogWrite(BUTTON_LIGHT_WEAPONA, LOW);
          fill_solid(speechLEDS, SPEECH_NUM_LEDS, CRGB(0,0,0));
        }
        
      }   //end if peak available()
      else fill_solid(speechLEDS, SPEECH_NUM_LEDS, CRGB(0,0,0));
    } //end if isPlaying
    else {
      analogWrite(BUTTON_LIGHT_WEAPONA, LOW); 
      //analogWrite(BUTTON_LIGHT_TORPEDO, LOW); 
      analogWrite(BUTTON_LIGHT_SPEECH, LOW);
      fill_solid(speechLEDS, SPEECH_NUM_LEDS, CRGB(0,0,0));
    }
    
  bool rLaser = animate(&laserFrame, LASER_ANIMATION_HEIGHT, laserAnimation, LASER_NUM_LEDS, laserLEDS);
  bool rTorpedo = animate(&torpedoFrame, TORPEDO_ANIMATION_HEIGHT, torpedoAnimation, LASER_NUM_LEDS, laserLEDS);
  if (!rLaser && !rTorpedo) { 
    fill_solid(laserLEDS, LASER_NUM_LEDS, CRGB(0,0,255)); 
  }
  
    FastLED.show();
  } //end animationMetro Check

  debugOptionsCheck();   
}

void processAction (int action, int src, int key, int data) {
  
  switch (action) {
   
      case ACTION_WEAPONB:                actionWeaponB();            break;
      case ACTION_WEAPONA:                actionWeaponA();            break;                                                                         
      case ACTION_SPEECH:                 actionSpeech();             break;                                        
      case ACTION_ENGINE:                 actionEngine();             break;
      case ACTION_IDLE_TOGGLE:            actionIdleToggle();         break;
      case ACTION_BGM_TOGGLE:             actionBGMToggle();          break;
      case ACTION_BGM_NEXT:               actionBGMNext();            break;
      case ACTION_GROOT:                  actionGroot();              break;
      //case ACTION_TORPEDO_CHARGE_START:   actionTorpedoCharge(0);   break;
      //case ACTION_TORPEDO_CHARGE_DURING:  actionTorpedoCharge(1);   break;  
      //case ACTION_TORPEDO_CHARGE_STOP:    actionTorpedoCharge(2);   break; 
      case ACTION_FLASH_BUTTON:           actionFlashButton(key);     break; 
      case ACTION_VOLUME_UP:              actionVolumeUp();           break;
      case ACTION_VOLUME_DOWN :           actionVolumeDown();         break;
   
  }
  
}

void actionFlashButton(int btn) {
  
  //bool state = digitalRead(buttonLightPins[btn]);
  //if (debugOptions[DEBUG_ACTION]) Serial.printf("Flash Button: %d, \n", btn);
  if (millis() % 10 == 0){
    //state = !state;
    digitalWrite(buttonLightPins[btn], HIGH);
  }
  else if (millis() % 100 == 0){
    //state = !state;
    digitalWrite(buttonLightPins[btn], LOW);
  }
}


void actionGroot() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("actionGroot()");

  //play random GROOT#.WAV
  String fn = "GROOT";
  fn = fn + random (1, NUM_GROOT_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_SPEECH, fn);
}


void actionWeaponB() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("actionWeaponB()");

  //play random WPNB#.WAV
  String fn = "WPNB";
  fn = fn + random (1, NUM_WEAPONB_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);

  //torpedo LED animation
  laserFrame=9999;
  torpedoFrame=0;
}

/*
void actionTorpedoCharge(int state) {
 if (debugOptions[DEBUG_ACTION]) {
  Serial.printf("Torpedo charge: %d\n", state); 
 }
 if (state == 0) queueWAV( CHANNEL_WEAPON, "TORPEDO0.WAV");
 //if (state == 1) actionFlashButton(0);
 if (state == 2) actionTorpedo();
 
}
*/
void actionWeaponA() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("actionWeaponA()");
  //weapon sound
  //weapon LED animation

  //play random WPNA#.WAV
  String fn = "WPNA";
  fn = fn + random (1, NUM_WEAPONA_WAVS + 1) + ".WAV";
  queueWAV( CHANNEL_WEAPON, fn);
  torpedoFrame=9999;
  laserFrame=0; 
}

void actionSpeech() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("actionSpeech()");

    //play random SPEECH#.WAV
    String fn = "VOICE";
    fn = fn + random (1, NUM_VOICE_WAVS + 1) + ".WAV";
    queueWAV( CHANNEL_SPEECH, fn);
}


void actionEngine() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("There is no sound in space, but let's make some anyway!");
  //turn off the digital write from flashing the LED
  digitalWrite(BUTTON_LIGHT_ENGINE, 0);
   
  mixer1.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
  mixer2.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);

  //play random ENGINE#.WAV
    String fn = "ENGINE";
    fn = fn + random (1, NUM_ENGINE_WAVS + 1) + ".WAV";
    queueWAV( CHANNEL_ENGINE, fn);
  
//  queueWAV(CHANNEL_ENGINE, "PASSBY10.WAV");

}

/*
 * actionidleToggle() is used to toggle the status of idle constant sound
 * and will stop if the status is now off
 * Note that there is a metro that watches for idle sound to finish and restarts, 
 * so we will let that metro start the sound once we change the status
 * 
 */
void actionIdleToggle () {
    idleStatus = !idleStatus;

  if (debugOptions[DEBUG_ACTION]) Serial.printf("Background Idle status = %s\n", idleStatus?"HIGH":"LOW");

  if (idleStatus) {
      mixer1.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
      mixer2.gain(CHANNEL_ENGINE, LEVEL_CHANNEL1);
  }
  else {
      mixer1.gain(CHANNEL_ENGINE, .1);
      mixer2.gain(CHANNEL_ENGINE, .1);
  }

  //turn off the digital write from flashing the LED
  digitalWrite(BUTTON_LIGHT_ENGINE, 0);
}


/*
 * actionBGMToggle() is used to toggle the status of Background Music
 * and will stop if the status is now off
 * Note that there is a metro that watches for BGM to finish and restarts, 
 * so we will let that metro start the music once we change the status
 * 
 */
void actionBGMToggle() {
  bgmStatus = !bgmStatus;
  if (debugOptions[DEBUG_ACTION]) Serial.printf("Background music status = %s\n", bgmStatus?"ON":"OFF");
  if (!bgmStatus) channels[CHANNEL_MUSIC]->stop(); 
}

/*
 * actionBGMNext() is used to queue the next Background Music track
 * 
 */
void actionBGMNext() {
  if (debugOptions[DEBUG_ACTION]) Serial.println("actionBGMNext()");
  bgmStatus = 1;
  channels[CHANNEL_MUSIC]->stop(); //the metro should see it is stopped, but enabled and queue the next track..  
}

/*
 * HELPER FUNCTIONS
 * There should not be any code specific to this build below
 * 
 */

/*
 * Audio Playback 
 * queueWAV() - this is needed because we can't start audio files
 *              during the USB interrupt that generates key messages
 *              We add the file to a channel queue (currently only one deep)
 *              and there is a queue metro that will check for it and play it
 * 
 */

void queueWAV (int channel, String fn) {
  if (channel < NUM_CHANNELS) {
    playQueue[channel] = fn;
  if (debugOptions[DEBUG_AUDIO]) Serial.printf ("queueWAV(%i, %s)\n", channel, fn.c_str());
  
  }
}

/*
 * Audio Playback
 * playWAV() - this plays a specific wav file from SD card
 *             DO NOT CALL THIS FROM WITHIN A USB EVENT HANDLER, 
 *             USE queueWAV instead!
 */

void playWAV (int channel, String fn) {

  if (debugOptions[DEBUG_AUDIO]) Serial.printf("playWAV(%i, %s)\n", channel, fn.c_str());

  channels[channel]->play(fn.c_str());
  //while this delay is recommended, we are operating properly without it.
  //likely due to debounce and play queuing
  //delay(10);

} //end playWAV


/*
 * Animation
 * animate() - this function places an animation frame into an LED array
 *           note that we pass a pointer to the frame variable
 *           so that we can increment it inside the function if
 *           we are still within the animation
 *  
 */

bool animate(int *frame, int lastFrame, byte ani[], int numLEDS, CRGB leds[]) {
  if (*frame < lastFrame) {

  if (debugOptions[DEBUG_ANIMATION_RGB]) {
  Serial.print("Animation Row:");
  Serial.print(*frame);
  Serial.print("-");  
  Serial.print(millis());
  }
     
    for (int col=0; col < numLEDS; col++) {
      int base = (*frame*numLEDS*3) + (col*3);
      int b = ani[base];
      int g = ani[base + 1 ];
      int r = ani[base + 2 ];

      if (debugOptions[DEBUG_ANIMATION_RGB]) {
        Serial.printf("{%i:%i:%i,%i,%i}", col, base, r, g, b);
      }
  
      if (debugOptions[DEBUG_ANIMATION]) {
        Serial.printf("%s ", (r || g || b)?"*":" ");
      }  

      leds[col] = CRGB(r,g,b);    
    }
    
    if (debugOptions[DEBUG_ANIMATION_RGB] || debugOptions[DEBUG_ANIMATION]) {
        Serial.println("");
    }
    (*frame)++;
    return true;     //we updated the LED array
  } 
  else return false; //we did not update the LED array 
}

/*
 * Animation
 * loadAnimationBMP() - this function loads a BMP animation file from SD Card 
 *                      into an animation array 
 * 
 */
void loadAnimationBMP (const char *fn, byte ani[], int aniHeight, int aniWidth) { 
    Serial.printf("Loading BMP: %s \n", fn);
    
    // Open
    File bmpImage = SD.open(fn, FILE_READ);
    int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);

    // Change their types to int32_t (4byte)
    int32_t width = readNbytesInt(&bmpImage, 0x12, 4);
    int32_t height = readNbytesInt(&bmpImage, 0x16, 4);
    Serial.printf("Animation width: %i, height: %i \n", width, height); 

    if (width > aniWidth) Serial.println ("WARNING: BMP is wider than Array!");
    if (height > aniHeight) Serial.println ("WARNING: BMP is taller than Array!");
    if (width < aniWidth) Serial.println ("WARNING: BMP is narrower than Array!");
    if (height < aniHeight) Serial.println ("WARNING: BMP is shorter than Array!");
    
    int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

    if (pixelsize != 24)
    {
        Serial.println("Image is not 24 bpp");
        while (1);
    }

    bmpImage.seek(dataStartingOffset);//skip bitmap header

    // 24bpp means you have three bytes per pixel, usually B G R

    byte R, G, B;

    for(int32_t i = 0; i < height; i ++) {
        for (int32_t j = 0; j < width; j ++) {
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();

            int base = (aniHeight-1-i) * aniWidth * 3 + (j*3); //invert
            
            if ((i < aniHeight) && (j < aniWidth)) {
              ani[base] = B;
              ani[base+1] = G;
              ani[base+2] = R;
            }
            
            if (debugOptions[DEBUG_ANIMATION]) {
              if ( R || G || B ) Serial.print("*");
                else Serial.print(" ");
            }
        }
        if (debugOptions[DEBUG_ANIMATION]) Serial.print("\n");
    }

    bmpImage.close();

    Serial.printf("Finished loading BMP: %s \n", fn);     
}

/*
 * Animation
 * readNbytesInt - this function is used by loadAnimationBMP
 * 
 */
int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
    if (nBytes > 4)
        return 0;

    p_file->seek(position);

    int32_t weight = 1;
    int32_t result = 0;
    for (; nBytes; nBytes--)
    {
        result += weight * p_file->read();
        weight <<= 8;
    }
    return result;
}

/*
 * Animation
 * printAnimation() - this function prints a representation of an animation
 *                    array to the serial output
 *                    
 */

void printAnimation (byte ani[], int aniHeight, int aniWidth) {
    for(int32_t i = 0; i < aniHeight; i ++) {
        for (int32_t j = 0; j < aniWidth; j ++) {
            int base = (i * aniWidth * 3) + (j*3);
            
            int B = ani[base];
            int G = ani[base + 1];
            int R = ani[base + 2];

            if ( R || G || B ) Serial.print("*");
              else Serial.print(" ");
 
            }
        Serial.print("\n");
    }
}

/*
 * NButtons callback functions
 * 
 */
void buttonClick(int id) {
  if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonClick: %d\n", id);
  mapAction(SOURCE_BUTTON, id, 0);
}
void buttonDoubleClick(int id) {
  if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonDoubleClick: %d\n", id);
  mapAction(SOURCE_BTN_DBLCLICK, id, 0);
}
void buttonLongPressStart(int id) {
  if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonLongPressStart: %d\n", id);
  mapAction(SOURCE_BTN_LONGPRESS_START, id, 0);
}
void buttonLongPressStop(int id) {
  if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonLongPressStop: %d\n", id);
  mapAction(SOURCE_BTN_LONGPRESS_STOP, id, 0);
}
void buttonLongPress(int id) {
  if (debugOptions[DEBUG_INPUT]) Serial.printf("buttonLongPressDuring: %d\n", id);
  mapAction(SOURCE_BTN_LONGPRESS_DURING, id, 0);
}    
 

/*
 * Input
 * OnPress() - this function is called by the USB Host HID event when a key is pressed
 * 
 */
void OnPress(int key)
{
  if (debugOptions[DEBUG_INPUT]) Serial.printf("Pressed key: %i \n", key);

  //mapAction(SOURCE_KEY, key, 0);
  keyHeld = key;
  keyHeldDuration = millis();   
}

/*
 * Input
 * OnRelease() - this function is called by the USB Host HID event when a key is released
 * 
 */
void OnRelease(int key)
{
  if (debugOptions[DEBUG_INPUT]) Serial.printf("Released key: %i \n", key);

  unsigned long duration = 0;
  
  if (keyHeld == key) {
    duration = millis() - keyHeldDuration;
  }
  
  if (debugOptions[DEBUG_INPUT]) Serial.printf("KeyUp: %i; Duration = %i \n", key, duration); 

  mapAction(SOURCE_KEY, key, duration);   
}


/*
 * Input
 * OnHIDExtrasPress() - this function is called by the USB Host HID event when a MULTIMEDIA key is pressed
 * 
 */
void OnHIDExtrasPress(uint32_t top, uint16_t key) 
{
   OnPress(key);
}

/*
 * Input
 * OnHIDExtrasReleased() - this function is called by the USB Host HID event when a MULTIMEDIA key is released
 * 
 */
void OnHIDExtrasRelease(uint32_t top, uint16_t key) 
{
  OnRelease(key);
}

/*
 * action
 * actionVolumeUp() - increase system volume
 * 
 */
void actionVolumeUp() 
{
  mainVolume += VOLUME_INCREMENT;
  updateVolume();
}

/*
 * action
 * actionVolumeDown() - decrease system volume
 * 
 */
void actionVolumeDown() 
{
  mainVolume -= VOLUME_INCREMENT;
  updateVolume();
}

/*
 * audio
 * updateVolume() -  checks min / max vol levels and updates the audio system volume
 *  
 */
void updateVolume() {

  if (mainVolume < MIN_VOLUME) mainVolume = MIN_VOLUME;
  else if (mainVolume > MAX_VOLUME) mainVolume = MAX_VOLUME;
 
  sgtl5000_1.volume(mainVolume);
    if (debugOptions[DEBUG_AUDIO]) {
    Serial.print("System volume now: ");
    Serial.println(mainVolume);
  }
  
}


/* 
 *  Action
 *  mapAction() - this function maps a key, note, button, etc to an action
 *                and then calls the function for that action
 *  
 */

void mapAction(int src, int key, int data) {
  for (int s = 0; s< ACTION_MAP_SIZE; s++) {
    if (ActionMap[s][0] == src && ActionMap[s][1] == key) {
      int action = ActionMap[s][2];
      lastActionTime[action] = millis();
      processAction(action, src, key, data);     
    } //end if
  } //end for
}

/*
 * debugOptionsCheck() - this function checks the Serial input and
 *                       changes debug options as well as emulates some 
 *                       button presses
 *                       
 */
void debugOptionsCheck() {
  int incomingByte = 0;
  
  if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      int option;
      
      switch (incomingByte) {
          case '0':                         //turn all options off
            for (int o = 1; o<10; o++) {    //we dont use zero
              debugOptions[o] = 0;
            }
            Serial.println ("All debug options turned OFF");
            break;
            
          case '1' ... '9': 
            option = incomingByte - '0';
            debugOptions[option] = !debugOptions[option]; 
            //Serial.printf("Debug option %d is now %s\n", option, debugOptions[option]?"ON":"OFF");
            break;

          case 'q': mapAction(SOURCE_BUTTON, 0, 0); break;
          case 'w': mapAction(SOURCE_BUTTON, 1, 0); break;
          case 'e': mapAction(SOURCE_BUTTON, 2, 0); break;
          case 'r': mapAction(SOURCE_BUTTON, 3, 0); break;
          }
         
         printDebugOptions();
          
      }
       
}

/*
 * printDebugOptions() - this function outputs a debug option list with
 *                       with current status, and provides some input 
 *                       instructions
 *  
 */
void printDebugOptions() {
  Serial.println("\nDebug Options Status");
  Serial.println("Use serial input keys 1 through 9 to change debug options");
  Serial.println("Use serial input keys 0 to turn off all debug options at once");
  Serial.println("Use serial input keys QWER to emulate buttons 0 through 3");
  
  for (int o=1; o<10; o++) {    //we don't use zero
    {
      if (debugOptionsText[o]) //don't print undefined options
        Serial.printf("   Option %d = %s %s\n", o, debugOptions[o]?"ON: ":"OFF:", debugOptionsText[o]);
    }
  }
  Serial.println("\n");       //a little extra padding in the output
}


 
