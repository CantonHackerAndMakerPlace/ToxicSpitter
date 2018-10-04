  /* Ardumoto Example Sketch
  by: Jim Lindblom
  date: November 8, 2013
  license: Public domain. Please use, reuse, and modify this 
  sketch!

  Three useful functions are defined:
    setupArdumoto() -- Setup the Ardumoto Shield pins
    driveArdumoto([motor], [direction], [speed]) -- Drive [motor] 
      (0 for A, 1 for B) in [direction] (0 or 1) at a [speed]
      between 0 and 255. It will spin until told to stop.
    stopArdumoto([motor]) -- Stop driving [motor] (0 or 1).

  setupArdumoto() is called in the setup().
  The loop() demonstrates use of the motor driving functions.
*/
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define PIN 5
#define NUM_LEDS 5
// ========================================================
//              This is BRG coloring!
// ========================================================
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);



Servo myServo;  // create a servo object
// Clockwise and counter-clockwise definitions.
// Depending on how you wired your motors, you may need to swap.
#define CW  0
#define CCW 1

// Motor definitions to make life easier:
#define MOTOR_A 0

int trigPin = 11;    // Trigger
int echoPin = 13;    // Echo
long duration, cm, inches;
 
// Pin Assignments //
// Don't change these! These pins are statically defined by shield layout
const byte PWMA = 3;  // PWM control (speed) for motor A
const byte DIRA = 12; // Direction control for motor A

void setup()
{
  setupArdumoto(); // Set all pins as outputs
    //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myServo.attach(9); // attaches the servo on pin 9 to the servo object
  Serial.begin(9600); // open a serial connection to your computer
  
  // Starting LED strip.
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  Twinkle(500);
  
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 

  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
 if (inches < 7)
 {
     // Drive motor A (and only motor A) at various speeds, then stop.
      driveArdumoto(MOTOR_A, CCW, 200);  // Set motor A to CW at half
      delay(500);  // Motor A will keep trucking for 1 second 
     stopArdumoto(MOTOR_A);  // STOP motor A 
       delay(2000);  //
       myServo.write(90);
       delay(2000);  //
       myServo.write(10);
     delay(5000);  //

  }
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  
  delay(250);

 }

// driveArdumoto drives 'motor' in 'dir' direction at 'spd' speed
void driveArdumoto(byte motor, byte dir, byte spd)
{
  if (motor == MOTOR_A)
  {
    digitalWrite(DIRA, dir);
    analogWrite(PWMA, spd);
  }
  
}

// stopArdumoto makes a motor stop
void stopArdumoto(byte motor)
{
  driveArdumoto(motor, 0, 0);
}

// setupArdumoto initialize all pins
void setupArdumoto()
{
  // All pins should be setup as outputs:
  pinMode(PWMA, OUTPUT);
  pinMode(DIRA, OUTPUT);

  // Initialize all pins as low:
  digitalWrite(PWMA, LOW);
  digitalWrite(DIRA, LOW);
}
void Twinkle(int SpeedDelay) {
  setAll(0,0,0);
  
  for (int i=0; i<NUM_LEDS; i++) {
    switch(random(3)) {
      case 0:
        // Purple
        setPixel(random(NUM_LEDS), 0xff, 0xff, 0x00);
        break;
      case 1:
        // Green
        setPixel(random(NUM_LEDS),0x00, 0x00, 0xFF);
        break;
      case 2:
        // Whit-ish
        setPixel(random(NUM_LEDS),0xE0, 0xE0, 0xE0);
        break;

    }
    showStrip(); 
  }
  
  delay(SpeedDelay);
}

void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}



