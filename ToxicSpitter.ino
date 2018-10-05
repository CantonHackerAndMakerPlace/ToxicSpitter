  /* Toxic spitter
  by: Steven venham, Brian Bartman
  date: October 4th 2018
  license: Meh

  Detect at 5ft:
    Turn on lights for 30 seconds
  Detect at 2ft:
    Turn on Motor
    if(30% Random)
      Fire Servo
*/
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

//Pin Information
const uint8_t LED_PIN = 5;
const uint8_t SERVO_1 = 6;
const uint8_t SERVO_2 = 7;
const uint8_t SENS_TRIG = 11;
const uint8_t SENS_ECHO = 13;

//NeoPixle Configuration
const uint8_t NUM_LEDS = 5;
Adafruit_NeoPixel strip;
const uint8_t TWINKLE_RATE = 10;

//Servo Configuration
Servo servo_head;
Servo servo_spit;

//Head
bool already_fireing = false;

//Timer Config
const int LIGHT_DELAY_MAX = 7000; //How long lights stay on
const int COOLDOWN_TIMER_MAX = 30000; //How long before next scare.
const int DELAY_BEFORE_SPRAY = 2000; //Delay before spit
const int HOLD_SPRAY_MAX = 1000; //Hold down spit servo
const int HEAD_UP_MAX = 2000; //How long head stays up
unsigned long light_timer;
unsigned long cooldown_timer;
unsigned long head_timer;
unsigned long spit_timer;
unsigned long begin_time = 0;
unsigned long end_time = 0;

//Distance avarage
const int DIS_ACC = 30;
unsigned int mesures[DIS_ACC];
int mesures_index = 0;
const unsigned int MAX_DISTANCE = 400;
 
void setup(){
  //Setup Serial Information
  Serial.begin (9600);
  
  //Define inputs and outputs
  pinMode(LED_PIN, OUTPUT);
  pinMode(SERVO_1, OUTPUT);
  pinMode(SERVO_2, OUTPUT);
  pinMode(SENS_TRIG, OUTPUT);
  pinMode(SENS_ECHO, INPUT);

  //Init the neopixle information
  strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //init servos
  servo_head.attach(SERVO_1); // attaches the servo on pin 9 to the servo object
  servo_spit.attach(SERVO_2); // attaches the servo on pin 9 to the servo object

  //init avarage distance
  for(int i = 0; i < DIS_ACC; i++)
  {
    mesures[i] = MAX_DISTANCE;
  }
}

unsigned int getAvarageDistance(){
  unsigned int dist = getDistanceOfSensor();
  if(dist > MAX_DISTANCE)
  {
     mesures[mesures_index] = MAX_DISTANCE;
  }
  else
  {
    mesures[mesures_index] = dist;
  }

  if(++mesures_index == DIS_ACC)
  {
    mesures_index = 0;
  }
  
  long total = 0;
  for(int i = 0; i < DIS_ACC; i++)
  {
    total += mesures[i];
  }

  return total / DIS_ACC;
}

unsigned int getDistanceOfSensor(){
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(SENS_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SENS_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SENS_TRIG, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(SENS_ECHO, INPUT);
  int duration = pulseIn(SENS_ECHO, HIGH);

  int distance = (duration / 2) * 0.0343;
 
  // Convert the time into a distance
  //cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  //inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  return distance;
}

void recalculate(const unsigned long deltatime,unsigned long& mtime){
  if(mtime > 0)
  {
    if(mtime < deltatime)
    {
      mtime = 0;
    }
    else
    {
      mtime -= deltatime;
    }
  }
}

void fireHead(){
  if(already_fireing)
  {
    
  }
  else
  {
    Serial.println("FIRE");
    //Do calculations to see if it sprays
    servo_head.write(170);
    already_fireing = true;
  }
}
//No delays in Loop =D
void loop(){
  unsigned long deltatime = end_time - begin_time;
  begin_time = millis();
  //Apply time delta
  recalculate(deltatime,light_timer);
  recalculate(deltatime,head_timer);
  recalculate(deltatime,spit_timer);
  //CHeck triggers
  if(head_timer == 0){
    if(already_fireing)
    {
      Serial.println("DONE");
      already_fireing = false;
      servo_head.write(0);
    }
  }
  else{
    fireHead();
  }
  if(light_timer == 0){
    setAll(0,0,0);
    showStrip();
  }
  else{
    twinkle();
  }
  if(spit_timer == 0){
    servo_spit.write(0);
  }
  else{
    
  }
  
  int distance = getAvarageDistance();
  if(distance < 120){
    light_timer = LIGHT_DELAY_MAX;
  }
  if(distance < 40){
    head_timer = HEAD_UP_MAX;
  }
  end_time = millis();
}


void twinkle() {
  //setAll(0,0,0);
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




