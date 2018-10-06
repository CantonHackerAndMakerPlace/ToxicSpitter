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

  Notes: 
    PWM and other blocking librays DO NOT GET ALONG.
    Servos twich due to delay commands in PWM circutry.
    This also HEAVILY applies to SERIAL output.
    If you want Debug info, you MUST set a high baud rate.
*/

#define DEBUG_PRINT(x) //Serial.println(x);

#include <Servo.h>
#include <Adafruit_NeoPixel.h>

//Pin CONFIG
const uint8_t LED_PIN = 5;
const uint8_t SERVO_1 = 6;
const uint8_t SERVO_2 = 7;
const uint8_t SENS_TRIG = 11;
const uint8_t SENS_ECHO = 13;

//NeoPixle CONFIG
const uint8_t NUM_LEDS = 5; //How many LED in the strip?
const uint8_t TWINKLE_RATE = 10; //How many miliseconds before lights change color
const uint8_t LIGHT_SENS_MAX = 10; //Threshold before activation
const uint8_t LIGHT_SENS_DISTANCE_MAX = 120; //Cm to trigger lights

//Head CONFIG
const uint16_t HEAD_BEGIN_ANGLE = 0; //Angle of head servo begin
const uint16_t HEAD_END_ANGLE = 90; //Angle of head servo end
const uint16_t HEAD_SENS_MAX = 100; //Threshold before activation
const uint16_t HEAD_SENS_DISTANCE_MAX = 40; //cm to trigger Head

//Spit CONFIG
const uint8_t SPIT_BEGIN_ANGLE = 0; //Begin angle of spit servo
const uint8_t SPIT_END_ANGLE = 120; //End angle of spit servo
const uint8_t SPIT_CHANCE = 30; //x/100

//Timer Config (In Mili)
const uint16_t LIGHT_DELAY_MAX = 7000; //How long lights stay on
const uint16_t COOLDOWN_TIMER_MAX = 10000; //How long before next scare.
const uint16_t DELAY_BEFORE_SPRAY = 2000; //Delay before spit
const uint16_t HOLD_SPRAY_MAX = 1000; //Hold down spit servo

/* ======================================
 * CONFIG END
 * Code beyond Here should not be edited.
 */
 
//Distance avarage(In CM)
const int DIS_ACC = 12; //Sample Avg.
uint16_t mesures[DIS_ACC];
int mesures_index = 0;
const uint16_t MAX_DISTANCE = 400;

//Action Variables
Adafruit_NeoPixel strip;
bool need_to_spit = false;
bool already_fireing = false;
uint16_t head_sens = 0;
uint16_t light_sens = 0;
uint16_t light_twinkle_countdown = 0;
bool lights_on = false;

//Timers
unsigned long light_timer;
unsigned long cooldown_timer;
unsigned long head_timer;
unsigned long hold_spray;
unsigned long spit_timer;
unsigned long begin_time = 0;
unsigned long end_time = 0;

//Servo Variables
Servo servo_head;
Servo servo_spit;
uint8_t servo_head_angle = HEAD_BEGIN_ANGLE;
uint8_t servo_spit_angle = SPIT_BEGIN_ANGLE;
 
void setup(){ //Get shit together.
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
  servo_head.attach(SERVO_1); 
  servo_spit.attach(SERVO_2); 

  //init avarage distance
  for(int i = 0; i < DIS_ACC; i++)
  {
    mesures[i] = MAX_DISTANCE;
  }
  DEBUG_PRINT("READY");
}

void startCooldown(){ //Set the cooldown and set everything back to a "Known" State.
  lightsOff();
  DEBUG_PRINT("DONE");
  already_fireing = false;
  servo_head_angle = HEAD_BEGIN_ANGLE;
  servo_spit_angle = SPIT_BEGIN_ANGLE;
  cooldown_timer = COOLDOWN_TIMER_MAX;
  light_timer = 0;
  head_timer = 0;
  spit_timer = 0;
  need_to_spit = false;
  lights_on = false;
  for(int i = 0; i < DIS_ACC; i++)
  {
    mesures[i] = MAX_DISTANCE;
  }
  
}

void lightsOff(){ //Turn the lights out.
  if(lights_on)
  {
    lights_on = false;
    DEBUG_PRINT("Lights Out");
    setAll(0,0,0);
    showStrip();
  }
}

void twinkle() { //Call each loop to twinkle Lights
  if(!lights_on){
    DEBUG_PRINT("Twinkle");
    lights_on = true;
  }
  if(light_twinkle_countdown++ > TWINKLE_RATE){
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
    }
    showStrip(); 
    light_twinkle_countdown = 0;
  }
}

void showStrip() { //Apply the light colors
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

unsigned int getAvarageDistance(){ //Calculate avarage of distance for linear smothing
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

unsigned int getDistanceOfSensor(){ //Get distance of sensor.
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

void recalculate(const unsigned long deltatime,unsigned long& mtime){ //Ajust timers based on "DELTA" Time
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

void handleHead(){ //Handle head motion
  if(head_timer == 0){
      if(already_fireing){
        startCooldown();
      }
    }
    else{
      if(!already_fireing)
      {
        DEBUG_PRINT("HEAD");
        if(random(0, 100) < SPIT_CHANCE)
        {
          spit_timer = DELAY_BEFORE_SPRAY;
        }
        servo_head_angle = HEAD_END_ANGLE;
        already_fireing = true;
      }
    }
}

void handleLight(){ //Handle Light action
  if(light_timer == 0){
      lightsOff();
    }
    else{
      twinkle();
    }
}

void handleSpit(){ //Handle Spitting
  if(spit_timer == 0){
      if(need_to_spit)
      {
        DEBUG_PRINT("SPITTING");
        servo_spit_angle = SPIT_END_ANGLE;
        need_to_spit = false;
      }
    }
    else{
      if(!need_to_spit)
      {
        need_to_spit = true;
      }
    }
}

void handleDistanceSensor(){ //Checks the distance sensor to see if actions need to take place.
  int distance = getAvarageDistance();
      if(distance < LIGHT_SENS_DISTANCE_MAX){
        if(light_sens >= LIGHT_SENS_MAX)
        {
          light_timer = LIGHT_DELAY_MAX;
        }
        else
        {
          light_sens++;
        }
      }
      else{
        if(light_sens > 0)
        {
          light_sens--;
        }
      }
      if(distance < HEAD_SENS_DISTANCE_MAX){
        if(head_sens >= HEAD_SENS_MAX)
        {
          head_timer = HOLD_SPRAY_MAX + DELAY_BEFORE_SPRAY;
        }
        else
        {
          head_sens++;
        }
      }
      else{
        if(head_sens > 0)
        {
          head_sens--;
        }
      } 
}

void loop(){ //No delays in Loop =D
  servo_head.write(servo_head_angle);
  servo_spit.write(servo_spit_angle);
  unsigned long deltatime = end_time - begin_time;
  begin_time = millis();
  //Reduce timers
  recalculate(deltatime,light_timer);
  recalculate(deltatime,head_timer);
  recalculate(deltatime,spit_timer);
  recalculate(deltatime,cooldown_timer);
  if(cooldown_timer == 0){
    //Check Timer Triggers
    handleLight();
    handleSpit();
    handleHead();

   if(!already_fireing){
      handleDistanceSensor();
   }
  }
  else
  {
    DEBUG_PRINT(cooldown_timer);
  }
  end_time = millis();
}




