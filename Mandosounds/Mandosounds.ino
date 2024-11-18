#include "mp3tf16p.h"
#include <FastLED.h>

#define NUM_LEDS 72 // must be 22 for mando helmet
#define DATA_PIN A0
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS 60
#define LED_INTRO_TIMEOUT     2000
#define LED_SOUNDBITE_TIMEOUT 5000
#define LED_ONELINER_TIMEOUT  3000

#define BUTTON_PIN 2

#define LED_BUILTIN 13

// fixed folders with specific mp3 files
#define FOLDER_INTROS     1
#define FOLDER_SOUNDBITES 2
#define FOLDER_ONELINERS  3

#define VOLUME_DEFAULT 28

// Global variables
CRGB leds[NUM_LEDS];
uint8_t paletteIndex = 0;
int ledTimeout[4] = {0, LED_INTRO_TIMEOUT, LED_SOUNDBITE_TIMEOUT, LED_ONELINER_TIMEOUT};

bool once = true;
byte lastButtonState = LOW;
unsigned long debounceDuration = 50; // millis
unsigned long lastTimeButtonStateChanged = 0;

MP3Player myMP3(10, 11); // our mp3 player, RX, TX
int folderCnt[4] = {-1, 5, 8, 91}; // hard coded for now
String folderName[4] = {"root", "intros", "soundbites", "oneliners"};
int oneLinerCycle = 5; // after this number of one-liners we play a soundbite
int lastTypePlaying = 0; // FOLDER_INTRO | FOLDER_SOUNDBITES | FOLDER_ONELINERS
int lastTimePlayStart = 0; // time when last sound was started - for LED duration

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.
DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};
  
CRGBPalette16 myPal = Sunset_Real_gp;

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  randomSeed(analogRead(A1));

  myMP3.initialize();

  // Init LED strip
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS); // set master brightness control
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  
  Serial.print("Setting volume to ");
  Serial.println(VOLUME_DEFAULT);
  myMP3.player.volume(VOLUME_DEFAULT);

//  Serial.println((String)myMP3.player.readFileCountsInFolder(2)+" tracks found in folder 2");
}

void playFromFolder(int folder) {
  int playTrack = random(folderCnt[folder])+1;
  lastTypePlaying = folder;
  lastTimePlayStart = millis();
  Serial.println((String)"Playing track "+playTrack+" from folder "+folder+" ("+folderName[folder]+")");
  myMP3.player.playFolder(folder, playTrack);
}
  
void playNextSound() {
  static int oneLinersPlayed = 0;
  
//  for (int i=1; i<=3; i++) {
//    Serial.println((String)myMP3.player.readFileCountsInFolder(i)+" tracks found in folder "+i+" ("+folderName[i]+")");
//    delay(50);
//  }

  if (oneLinersPlayed == oneLinerCycle) {
    playFromFolder(FOLDER_SOUNDBITES);
    oneLinersPlayed = 0;
  } else {
    playFromFolder(FOLDER_ONELINERS);
    oneLinersPlayed++;
  }
}

void loop() {
  // play random track at startup
  if (!once) {
    Serial.println("Playing startup sound");
    playFromFolder(FOLDER_SOUNDBITES);
    Serial.println("HELLO WORLD!");
    once = false;
  }
  
  // BUTTON TRIGGER
  if (millis() - lastTimeButtonStateChanged > debounceDuration) {
    byte buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      if (buttonState == LOW) {
        // Button is pushed down. We play another random sound
        Serial.println("Button pushed");
        playNextSound();
      }
    }
  }

//  if (millis() - lastTimePlayStart < ledTimeout[lastTypePlaying]) { // estimate that sound is playing
//    // MOVING PALETTE EFFECT
//    //fill_palette(led array, nLEDS, startIndex, indexDelta, palette,
//    //             brightness, blendType:LINEARBLEND|NOBLEND)
//    fill_palette(leds, NUM_LEDS, paletteIndex, 1 /*255/NUM_LEDS*/, myPal, BRIGHTNESS, LINEARBLEND);
//  
////    EVERY_N_MILLISECONDS(20) {
////      Serial.println("Playing... palette update");
//      paletteIndex++;
////    }
//  }
//  else {
////    Serial.println("All quiet...");
//    FastLED.clear();
//  }

  FastLED.show();
}
