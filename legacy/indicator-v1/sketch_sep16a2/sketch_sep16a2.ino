#define NUM_LEDS 220
#include "FastLED.h"
#define PIN 6
CRGB leds[NUM_LEDS];
byte counter;

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;

TwoWire I2Cax = TwoWire();
sensors_event_t a, g, temp;

int state = 0;

///////////////////////////////////
// HELPERS

float cur_acceleration(sensors_event_t a) {
  return sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z);
}

// HITS

// 0 or 1 currently, for hit state exactly
int hit_state = 0;

// 0 - not hit, before high hit threshold
// 1 - exactly hit moment, needed to switch hit state next frame
// 2 - hit aftershock, switched to 0 after acceleration is lower than low hit threshold
int hit_substate = 0;
const float hit_high_threshold = 5;
const float hit_low_threshold = 3;
const float default_g = 9.8;
void hit_reset() { hit_state = 0; }
int hit_check(sensors_event_t a) {
  float cur_accel = cur_acceleration(a);

  cur_accel -= default_g;
  cur_accel = abs(cur_accel);
  
  if (hit_substate == 0) {
    if (cur_accel > hit_high_threshold) {
      hit_state = 1;
      hit_substate = 1;
    }
  } else
  if (hit_substate == 1) {
    hit_substate = 2;
    hit_state = 0;
  }
  // no else for immediate switch 1->0 if possible
  if (hit_substate == 2) {
    if (cur_accel < hit_low_threshold) {
      hit_state = 0;
      hit_substate = 0; 
    }
  }
  
}

// HIT SEQUENCES

int hit_back_to_default = 0;
int hit_back_to_default_activated = 0;
long hit_back_to_default_last_hit = 0;
int hit_back_to_default_last_hit_threshold_1_min = 300;
int hit_back_to_default_last_hit_threshold_1_max = 600;
int hit_back_to_default_last_hit_threshold_2_min = 300;
int hit_back_to_default_last_hit_threshold_2_max = 600;

// 0 - no actions, hit 1 wait
// 2 - hit 2 wait
// 4 - hit 3 wait
// when hit 3, go to 0
int hit_back_to_default_substate = 0;
void check_hit_back_to_default() {
  if (hit_back_to_default_activated) {
    if (hit_back_to_default_substate == 0) {
      if (hit_state == 1) {
        hit_back_to_default_last_hit = millis();
        hit_back_to_default_substate = 1;
      }
    } else
    if (hit_back_to_default_substate == 1) {
      if (hit_state == 1) {
        if (millis() - hit_back_to_default_last_hit > hit_back_to_default_last_hit_threshold_1_min) {
          hit_back_to_default_last_hit = millis();
          hit_back_to_default_substate = 2;
        } else {
          hit_back_to_default_last_hit = 0;
          hit_back_to_default_substate = 0;
        }
      } else
      if (millis() - hit_back_to_default_last_hit > hit_back_to_default_last_hit_threshold_1_max) {
        hit_back_to_default_last_hit = 0;
        hit_back_to_default_substate = 0;
      }
    } else
    if (hit_back_to_default_substate == 2) {
      if (hit_state == 1) {
        if (millis() - hit_back_to_default_last_hit > hit_back_to_default_last_hit_threshold_2_min) {
          hit_back_to_default = 1;
          hit_back_to_default_substate = 0;
          return;
        } else {
          hit_back_to_default_last_hit = 0;
          hit_back_to_default_substate = 0;
        }
      } else
      if (millis() - hit_back_to_default_last_hit > hit_back_to_default_last_hit_threshold_2_max) {
        hit_back_to_default_last_hit = 0;
        hit_back_to_default_substate = 0;
      }
    }
  }
  
  hit_back_to_default = 0;
}

int double_hit = 0;
int single_hit_not_double = 0;
int double_hit_activated = 0;
int double_hit_substate = 0;
long double_hit_last_hit = 0;
int double_hit_threshold_1_min = 200;
int double_hit_threshold_1_max = 600;
void check_double_hit() {
  if (double_hit_activated) {
    if (double_hit_substate == 0) {
      if (hit_state == 1) {
        double_hit_substate = 1;
        double_hit_last_hit = millis();
      }
    } else
    if (double_hit_substate == 1) {
      if (hit_state == 1) {
        if (millis() - double_hit_last_hit > double_hit_threshold_1_min) {
          double_hit = 1;
          double_hit_last_hit = 0;
          double_hit_substate = 0;
          return;
        } else {
          double_hit_last_hit = 0;
          double_hit_substate = 0;
        }
      } else if (millis() - double_hit_last_hit > double_hit_threshold_1_max){
        single_hit_not_double = 1;
        double_hit_last_hit = 0;
        double_hit_substate = 0;
        return;
      }
    }
  }
  double_hit = 0;
  single_hit_not_double = 0;
}


void check_hit_sequences() {
  check_hit_back_to_default();
  check_double_hit();
}

// ANGLES

float angle_x = 0, angle_y = 0;
float subangle_x = 0;
float subangle_y_minus = 0;
float subangle_y_plus = 0;

float angle_correction(float angle) {
  if (abs(angle) > 90) {
    float angle_sign = angle / abs(angle);
    float new_abs = abs(angle) - 180;
    float new_angle = new_abs * angle_sign;
    return new_angle;
  }
  return angle;
}

void angle_check(sensors_event_t a) {
  /*angle_x = cur_acceleration(a); // reuse for acceleration

  angle_x -= default_g;
  angle_x = abs(angle_x);
  if (angle_x > 1.5) {
    return;
  }*/
  
  angle_x = atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;
  angle_y = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  subangle_y_minus = atan2(-a.acceleration.y * sqrt(3) - a.acceleration.x, a.acceleration.z) * 180 / PI;
  subangle_y_plus = atan2(a.acceleration.y * sqrt(3) - a.acceleration.x, a.acceleration.z) * 180 / PI;

  // need to convert it from (-180;180) to (-90;90) with 0 points when original angle is 0 or near to -180\180 and with same signs when turned to same side

  angle_x = angle_correction(angle_x);
  angle_y = angle_correction(angle_y);

  // subangles are non-negative angles for 3 vectors 
  subangle_y_minus = max(angle_correction(subangle_y_minus), 0.0);
  subangle_y_plus = max(angle_correction(subangle_y_plus), 0.0);
  subangle_x = max(angle_x, 0.0);
}



// HELPERS
//////////////////////////////////


// let's do every subprogram here a state with it's own control method and substates but with universal exit to default state
// 
// States list:
// 0 - default
// 1 - color red blinking, for debug
// 2 - violet with flecks



// 0 - default state, done for switching between other subprograms, by default it uses one hits to switch between sleep mode and selection mode, selection mode uses tilt for choosing option with color indication

// 0 - start state to clear color
// 1 - state for choosing color
int default_state_substate = 0;

void start_default_state(){
  state = 0;
  default_state_substate = 0;
  hit_back_to_default_activated = 0;
  double_hit_activated = 1;
}

void loop_start_default_state() {
  if (default_state_substate == 0) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 255);
    }
    if (double_hit) {
      default_state_substate = 1;
    }
  } else
  if (default_state_substate == 1) {

    int choice_count = 5;
    int choice_number = (angle_y + 90) / (180 / choice_count);
    
    if (choice_number == 0) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255, 255, 255);
      }
      if (single_hit_not_double) {
        end_default_state();
        start_smooth_state();
      }
    } else
    if (choice_number == 4) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 255, 255);
      }
      if (single_hit_not_double) {
        end_default_state();
        start_movement_state();
      }
    } else 
    if (choice_number == 1) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255, 0, 0);
      }
      if (single_hit_not_double) {
        end_default_state();
        start_debug1_state();
      }
    } else 
    if (choice_number == 3) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(109, 0, 204);
      }
      if (single_hit_not_double) {
        end_default_state();
        start_debug2_state();
      }
    } else
    if (choice_number == 2) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV((int(millis() / 100) + i * 10) % 255, 255, 255);
      }
      if (single_hit_not_double) {
        end_default_state();
        start_color_choice_state();
      }
    }
  }
  return;
}

void end_default_state() {
  double_hit_activated = 0;
  return;
}




// 1 - color red blinking, for debug
int red_delay = 500;
CRGB new_color;
void start_debug1_state(){
  state = 1;
  hit_back_to_default_activated = 1;
  double_hit_activated = 1;
  new_color = CRGB(random(255), random(255), random(255));
}

void loop_start_debug1_state() {
  if (hit_back_to_default) {
    end_debug1_state();
    start_default_state();
    return;
  }
  for (int i = 0; i < NUM_LEDS; i++ ) {
     leds[i] = new_color;
  }
  if (single_hit_not_double) {
    new_color = CRGB(random(255), random(255), random(255));
  }
  return;
}

void end_debug1_state() {
  double_hit_activated = 0;
  return;
}


// 2 - violet with flecks
#define FLECK_CNT 20
int fleck_state[FLECK_CNT];
void start_debug2_state(){
  state = 2;
  hit_back_to_default_activated = 1;
  for (int i = 0; i < FLECK_CNT; i++) {
    fleck_state[i] = random(NUM_LEDS / 2);
  }
}

void loop_start_debug2_state() {
  if (hit_back_to_default) {
    end_debug1_state();
    start_default_state();
    return;
  }
  for (int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB(109, 0, 204);
  }
  for (int i = 0; i < FLECK_CNT; i++) {
    leds[fleck_state[i]] = CRGB(0, 255, 0);
    leds[fleck_state[i] + NUM_LEDS / 2] = CRGB(0, 255, 0);
  }
  return;
}

void end_debug2_state() {
  return;
}


// 3 - color choice with angles
int color_r = 0;
int color_g = 0;
int color_b = 0;
int angle_threshold = 20;

// 0 for color choice, 1 to leave it fixed
int color_choice_state = 0;

void start_color_choice_state(){
  state = 3;
  hit_back_to_default_activated = 1;
  double_hit_activated = 1;
}

void loop_color_choice_state() {
  if (hit_back_to_default) {
    end_debug1_state();
    start_default_state();
    return;
  }
  for (int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB(color_r, color_g, color_b);
  }
  if (color_choice_state == 0) {
    if (subangle_x > angle_threshold) {
      if (color_r == 254){
        color_g = max(0, color_g-1);
        color_b = max(0, color_b-1);
      } else {
        color_r = min(255, color_r+2);
      }
    }
    if (subangle_y_plus > angle_threshold) {
      if (color_g == 254){
        color_r = max(0, color_r-1);
        color_b = max(0, color_b-1);
      } else {
        color_g = min(255, color_g+2);
      }
    }
    if (subangle_y_minus > angle_threshold) {
      if (color_b == 254){
        color_g = max(0, color_g-1);
        color_r = max(0, color_r-1);
      } else {
        color_b = min(255, color_b+2);
      }
    }
    if (double_hit) {
      color_choice_state = 1;
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[i] = CRGB(color_r, color_g, color_b);
      }
      FastLED.show();
      delay(300);
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[i] = CRGB(0, 0, 0);
      }
      FastLED.show();
      delay(500);
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[i] = CRGB(color_r, color_g, color_b);
      }
      FastLED.show();
      delay(300);
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[i] = CRGB(0, 0, 0);
      }
      FastLED.show();
      delay(500);
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[i] = CRGB(color_r, color_g, color_b);
      }
      FastLED.show();
    }
  }
}

void end_color_choice_state() {
  double_hit_activated = 0;
  return;
}


// 4 - smooth color change
long color_change_delay = 3500;
long last_color_change;
CRGB new_smooth_color;
CRGB new_smooth_color2;
void start_smooth_state(){
  state = 4;
  hit_back_to_default_activated = 1;
  new_smooth_color = CRGB(random(255), random(255), random(255));
  new_smooth_color2 = CRGB(random(255), random(255), random(255));
  last_color_change = millis();
}

void loop_smooth_state() {
  if (hit_back_to_default) {
    end_smooth_state();
    start_default_state();
    return;
  }
  long scale = color_change_delay - min(color_change_delay, millis() - last_color_change);
  for (int i = 0; i < NUM_LEDS; i++ ) {
     leds[i] = CRGB(
      long(new_smooth_color.r) * scale / color_change_delay + long(new_smooth_color2.r) * (color_change_delay - scale) / color_change_delay,
      long(new_smooth_color.g) * scale / color_change_delay + long(new_smooth_color2.g) * (color_change_delay - scale) / color_change_delay,
      long(new_smooth_color.b) * scale / color_change_delay + long(new_smooth_color2.b) * (color_change_delay - scale) / color_change_delay
     );
  }
  if (millis() - last_color_change > color_change_delay) {
    new_smooth_color = new_smooth_color2;
    new_smooth_color2 = CRGB(random(255), random(255), random(255));
    last_color_change = millis();
  }
  return;
}
void end_smooth_state() {
  return;
}


// 5 - movement to color
float scale = 100;
void start_movement_state(){
  state = 5;
  hit_back_to_default_activated = 1;
  new_smooth_color = CRGB(0, 255, 255);
  new_smooth_color2 = CRGB(255, 0, 0);
}

void loop_movement_state() {
  if (hit_back_to_default) {
    end_movement_state();
    start_default_state();
    return;
  }
  
  float cur_accel = cur_acceleration(a);

  cur_accel -= default_g;
  cur_accel = abs(cur_accel);
  
  scale = min(100.0, 100.0 * 100.0 / (100.0 + cur_accel * cur_accel * 1000.0)) * 0.1 + scale * 0.9;
  for (int i = 0; i < NUM_LEDS; i++ ) {
     leds[i] = CRGB(
      long(new_smooth_color.r) * scale / 100 + long(new_smooth_color2.r) * (100 - scale) / 100,
      long(new_smooth_color.g) * scale / 100 + long(new_smooth_color2.g) * (100 - scale) / 100,
      long(new_smooth_color.b) * scale / 100 + long(new_smooth_color2.b) * (100 - scale) / 100
     );
  }
  return;
}
void end_movement_state() {
  return;
}


void setup() {
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(50);
  pinMode(13, OUTPUT);


  Serial.begin(115200);
  while (!Serial)
    delay(10); // wait for serial port

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  start_default_state();
}

long tmp_time = 0;

void loop() {
  if (state == 0) {
    loop_start_default_state();
  } else
  if (state == 1) {
    loop_start_debug1_state();
  } else
  if (state == 2) {
    loop_start_debug2_state();
  }else
  if (state == 3) {
    loop_color_choice_state();
  }else
  if (state == 4) {
    loop_smooth_state();
  }else
  if (state == 5) {
    loop_movement_state();
  }
  delay(5);

  
  mpu.getEvent(&a, &g, &temp);

  /*(Serial.print("Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(" m/s^2, Y: "); Serial.print(a.acceleration.y);
  Serial.print(" m/s^2, Z: "); Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Gyro X: "); Serial.print(g.gyro.x);
  Serial.print(" rad/s, Y: "); Serial.print(g.gyro.y);
  Serial.print(" rad/s, Z: "); Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temp: "); Serial.print(temp.temperature);
  Serial.println(" degC");*/

  hit_check(a);
  angle_check(a);
  check_hit_sequences();

  delay(25);
  
  FastLED.show();

  
  /*for (int i = 0; i < NUM_LEDS; i++ ) {         // от 0 до первой трети
    leds[i] = CHSV(counter + i * 2, 255, 255);  // HSV. Увеличивать HUE (цвет)
    // умножение i уменьшает шаг радуги
  }
  counter++;        // counter меняется от 0 до 255 (тип данных byte)
  FastLED.show();
  delay(5);   */
}
