#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_APDS9960.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// I/O Pins
#define LED 13
#define NEOPIXEL_PIN 8

// OLED Setup
#define OLED_DC     6
#define OLED_CS     10
#define OLED_RESET  4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Gesture sensore setup
#define APDS9960_INT    2 // Needs to be an interrupt pin
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;

//Neopixel ring setup
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


//PROTOTYPES
int angleToLED(int angle);
void setLEDs();
void setAngleLEDs();
void cleanLEDs();
void leds();
void showAngleInLedsAndScreen();



// GLOBAL VARIABLES
struct RGB {
  unsigned int R[NUMPIXELS];
  unsigned int G[NUMPIXELS];
  unsigned int B[NUMPIXELS];
} activeLEDs;

unsigned int ledR[NUMPIXELS] = {0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int ledG[NUMPIXELS] = {0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int ledB[NUMPIXELS] = {0,0,0,0,0,0,0,0,0,0,0,0};

int angle = 0;


// ===================================================


void neoPixlesInit(){

  pixels.begin();
  for(int i = 0; i < NUMPIXELS; i++){
    ledR[i] = 0;
    ledG[i] = 0;
    ledB[i] = 0;
    pixels.setPixelColor(i, pixels.Color(ledR[i],ledG[i],ledB[i]));
    pixels.show();
  }
}

void oledInit(){
  //OLED Init
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);

  display.display();
  display.clearDisplay();
  display.display();

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(2, 0);
  display.println("FARUSAC");
  display.display();

  display.setTextSize(1);
  display.setCursor(18, 25);
  display.println("Lab CC Aplicadas");
  display.display();

  delay(5000);

  display.clearDisplay();
  display.display();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Nivel Laser");
  display.display();

  display.setCursor(0, 14);
  display.println("Automatico");
  display.display();

  delay(5000);

  display.clearDisplay();
  display.display();

  display.setTextSize(2);
}

void gestureSensorInit(){
  //Gesture sensor Init
  // Set interrupt pin as input
  pinMode(APDS9960_INT, INPUT_PULLUP);
   // Initialize interrupt service routine
  attachInterrupt(0, gestureInt, FALLING);

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void gestureInt() {
  isr_flag = 1;
}

void handleGesture() {
    if ( apds.isGestureAvailable() ) {
      display.setCursor(0, 0);
      display.clearDisplay();
      display.display();
      switch ( apds.readGesture() ) {
        case DIR_UP:
          Serial.println("UP");
          //display.println("ARRIBA");
          angle = (angle - 30) >= 0 ? angle - 30 : 330;
          break;
        case DIR_DOWN:
          Serial.println("DOWN");
          //display.println("ABAJO");
          angle = (angle + 30)%360;
          break;
        case DIR_LEFT:
          Serial.println("LEFT");
          //display.println("IZQUIERDA");
          
          break;
        case DIR_RIGHT:
          Serial.println("RIGHT");
          //display.println("DERECHA");
          break;
        case DIR_NEAR:
          Serial.println("NEAR");
          //display.println("CERCA");
          break;
        case DIR_FAR:
          Serial.println("FAR");
          //display.println("LEJOS");
          break;
        default:
          Serial.println("NONE");
          //display.println("ERROR");
      }
      showAngleInLedsAndScreen();
  }
}


void showAngleInLedsAndScreen(){
  display.setCursor(0, 0);
  display.setTextSize(3);
  display.print("  ");
  display.print(angle);
  display.setTextSize(2);
  display.println("o");
  display.display();
  cleanLEDs();
  leds();
}

void leds(){
  int positions[12] = {1, 0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  int compPositions[12] = {7, 6, 5, 4, 3, 2, 1, 0, 11, 10, 9, 8};

  pixels.setPixelColor(positions[angle/30], pixels.Color(0, 100, 0));
  pixels.setPixelColor(compPositions[angle/30], pixels.Color(100, 0, 0));
  pixels.show();
  
}

void pollGestureIsr(){
  if( isr_flag == 1 ) {
    isr_flag = 0;
    digitalWrite(LED, 1);
    detachInterrupt(0);
    handleGesture();
    digitalWrite(LED, 0);
    attachInterrupt(0, gestureInt, FALLING);
  }  
}

void ioSetup(){
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
}

void blinky(unsigned int msec){
  digitalWrite(LED, 1);
  delay(msec);
  digitalWrite(LED, 0);
  delay(msec);
}

void setup() {
  static unsigned int i;
  Serial.begin(115200); 
  ioSetup();
  oledInit();
  digitalWrite(LED, 1);
  delay(1000);
  digitalWrite(LED, 0);
  neoPixlesInit();
  cleanLEDs();
  gestureSensorInit();
  showAngleInLedsAndScreen();

}

void loop() {
  pollGestureIsr();
/*  if(isr_flag == 1){
    setAngleLEDs();
  }
*/  
}

void cleanLEDs(){
  // Limpiar estados de LEDs
  for(unsigned int i = 0; i < NUMPIXELS; i++){
    ledR[i] = 0;
    ledG[i] = 0;
    ledB[i] = 0;
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
  
}

void setAngleLEDs(){

  unsigned int MAX_BRIGHT = 100;
/*
  for(int i = 0; i < 360; i += 30){

    cleanLEDs();
    
    ledG[angleToLED(i)] = MAX_BRIGHT;
    ledR[angleToLED((i+180)%360)] = MAX_BRIGHT;
    delay(2000);
  
    Serial.print("GREEN: ");
    for(int j = 0; j < NUMPIXELS; j++){
      Serial.print(ledG[j]);
      Serial.print(",");
    }
    Serial.println("");

    Serial.print("RED:   ");
    for(int j = 0; j < NUMPIXELS; j++){
      Serial.print(ledR[j]);
      Serial.print(",");
    }
    Serial.println("\n");
  }

*/


  cleanLEDs();
  ledG[angleToLED(90)] = MAX_BRIGHT; 
  ledR[angleToLED(270)] = MAX_BRIGHT; 
  
  setLEDs();
  
}

void setLEDs(){
  static unsigned int i;
  for(i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(ledR, ledG, ledB));
    pixels.show();
  }
}


int angleToLED(int angle){
  //Angle input intended to be within 0 and 90 degrees
  int DELTA = 360/NUMPIXELS;
  return ((NUMPIXELS - (int) ((-1.0/DELTA)*angle) - 2) ) % NUMPIXELS;
}

