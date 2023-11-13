#include <FastLED.h>

#define LED_PIN_1     13
#define LED_PIN_2     14
#define BUTTON_PIN    5
#define NUM_STRIPS    2
#define NUM_LEDS    256
#define BRIGHTNESS  20
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_STRIPS][NUM_LEDS];

int updatesPerSecond = 10;
boolean isGameStarted = false;

unsigned long timer;

CRGB colour;

CRGB *matrix[32][16];

int startIndex = 0;
int direction = 1;
int layer = 0;
int lastState = HIGH;
int reading;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;

int segmentLength = 8;
int buttonState;
int currentPosition[8];
int previousPosition[8];
bool found = false;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

uint8_t colourIndex;

bool playAnimation = true;

bool resetGame = true;

void setup() {
    Serial.begin(115200);
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN_1>(leds[0], NUM_LEDS);
    FastLED.addLeds<LED_TYPE, LED_PIN_2>(leds[1], NUM_LEDS);

    FastLED.setBrightness(  BRIGHTNESS );
    
    colour = CRGB(random(0,255), random(0,255), random(0,255));

    for (int i = 0; i<32; i++){
      for (int j=0; j<16; j++){
        mapLed(i, j);
      }
    }
    pinMode(BUTTON_PIN, INPUT_PULLUP);


    //palette
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}


void mapLed(int row, int column){
  int ledIndex;
  if (row % 2 == 0){
    ledIndex = 8*row + column;
      if (column<8){
        matrix[row][column] = &leds[1][ledIndex];
      } else {
        matrix[row][column] = &leds[0][ledIndex-8];
      }
  }
  if ( row % 2 == 1){
    ledIndex = 8*(row+1) - column;
    if (column<8){
        matrix[row][column] = &leds[1][ledIndex-1];
      } else {
        matrix[row][column] = &leds[0][ledIndex+7];
      }
  }

  for (int i = 0; i < 8; i++){
    if (i<segmentLength){
      currentPosition[i] = startIndex + i;
    } else {
      currentPosition[i]= 99;
    } 
  }       
}

void loop() {
    colourIndex = 0;
    reading = digitalRead(BUTTON_PIN);

    if (reading != lastState) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {         
          if (!isGameStarted){
            if (!resetGame){
              for (int i = 0;i <32;i++){
                for (int j=0; j<16; j++){
                  *matrix[i][j]=CRGB(0,0,0);                
                }
              }            
              isGameStarted = true;
            }
            resetGame = !resetGame;
          } else {
            if (layer>0){
              for (int i = 0; i<8;i++){                
                for ( int j = 0; j <8; j++){
                  if (currentPosition[i]==previousPosition[j]){
                    found = true;
                  }
                }
                if (!found){
                  *matrix[layer][currentPosition[i]]=CRGB(0,0,0);
                  currentPosition[i] = 99;
                  segmentLength--;
                }
                found = false;                
              }
              for (int i = 0; i < 8; i++){
                previousPosition[i] = currentPosition[i];
              }  
            } else {
              for (int i = 0; i < 8; i++){
                if (i<segmentLength){
                  previousPosition[i]=currentPosition[i];
                } else {
                  previousPosition[i]=99;
                }
              }            
            }           
            layer++;
            colour = CRGB(random(0,255), random(0,255), random(0,255));
            updatesPerSecond+=2;          
          }
        }
      }
    }
    lastState = reading;
   
    if (!isGameStarted){
      if (resetGame){  
        uint8_t brightness = 120;
      
        for( int i = 0; i < NUM_LEDS; ++i) {
          leds[0][i] = ColorFromPalette( currentPalette, colourIndex, brightness, currentBlending);
          leds[1][i] = ColorFromPalette( currentPalette, colourIndex, brightness, currentBlending);
          colourIndex += 3;
        }
      }
    } else {
      if (direction > 0){
        if (startIndex > 0){
          *matrix[layer][startIndex-1] = CRGB(0,0,0);
        }
        for (int i = 0; i<segmentLength; i++){
          *matrix[layer][i+startIndex] = colour;
          currentPosition[i] = startIndex+i;
        }
        for (int i = segmentLength; i < 8; i++){
          currentPosition[i] = 99;
        }
        startIndex++;
        if (startIndex == 16-segmentLength){
          direction = -1;
          *matrix[layer][startIndex-1] = CRGB(0,0,0);
        }
        
      } else {
        if (startIndex < 16-segmentLength){
          *matrix[layer][startIndex+segmentLength] = CRGB(0,0,0);          
        }        
        for (int i = 0; i<segmentLength; i++){
          *matrix[layer][startIndex+i] = colour;
          currentPosition[i] = startIndex+i;
        }
        for (int i = segmentLength; i < 8; i++){
          currentPosition[i] = 99;
        }
        startIndex--;
        if (startIndex == 0){
          direction = 1;
          *matrix[layer][startIndex+segmentLength] = CRGB(0,0,0);
        }
      }
      if (segmentLength == 0){
        isGameStarted = false;
        layer=0;
        segmentLength = 8;
        startIndex = 0;
        updatesPerSecond = 10;
        direction = 1;
      }
    }
    
    FastLED.show();
    FastLED.delay(1000 / updatesPerSecond);
}
