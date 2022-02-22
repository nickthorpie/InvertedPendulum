/**
 * == Stabilizing DC motor with state control ==
 * 
 * Cart is being controlled by state controller (x, v) based on params found by fitting the model
 * 
 * == Hardware specification ==
 * OMRON E6B2-CWZ6C pinout
 * - Brown - Vcc
 * - Black - Phase A
 * - White - Phase B
 * - Orange - Phaze Z
 * - Blue - GND
 *
 * LPD3806-600BM-G5-24C pinout
 * - Green - Phase A
 * - White - Phase B
 * - Red - Vcc
 * - Black - GND
 */

#include <Arduino.h>

// motor encoder pins
#define OUTPUT_A  3 // PE5
#define OUTPUT_B  2 // PE4

// pulses per revolution
#define PPR  2400
#define SHAFT_R 0.00573

#define PWM_PIN 10
#define DIR_PIN 8

#define MAX_STALL_U 45.0
#define POSITION_LIMIT  0.2
#define X0  0.065

#define A 35.98
#define B 2.22
#define C -2.79

#define Kx  285.4
#define Kv  37.1

volatile long encoderValue = 0L;
volatile long lastEncoded = 0L;

unsigned long now = 0L;
unsigned long lastTimeMicros = 0L;

float x, last_x, v, dt;

unsigned long log_prescaler = 0;

void encoderHandler();

void setup() {

  // setting PWD frequency on pin 10 to 31kHz
  TCCR2B = (TCCR2B & 0b11111000) | 0x01;

  pinMode(OUTPUT_A, INPUT_PULLUP);
  pinMode(OUTPUT_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(OUTPUT_A), encoderHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(OUTPUT_B), encoderHandler, CHANGE);

  pinMode(PWM_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, LOW);

  Serial.begin(9600);
  lastTimeMicros = 0L;
}

float sign(float v) {
  if (fabs(v) < 1e-8) {
    return 0.0;
  } else if (v > 0) {
    return 1.0;
  } else {
    return -1.0;
  }
}

float avoidStall(float u) {
  if (fabs(u) < MAX_STALL_U) {
    return u > 0 ? u + MAX_STALL_U : u - MAX_STALL_U;
  }
  return u;
}

float saturate(float v, float maxValue) {
  if (fabs(v) > maxValue) {
    return (v > 0) ? maxValue : -maxValue;
  } else {
    return v;
  }
}

float getCartDistance(long pulses, long ppr) {
  return 2.0 * PI * pulses / PPR * SHAFT_R;
}

void driveMotor(float u) {
  digitalWrite(DIR_PIN, u > 0.0 ? LOW : HIGH);
  analogWrite(PWM_PIN, fabs(u));
}

void log_state(float control, float u) {
  if (log_prescaler % 50 == 0) {
    Serial.print(x, 4);Serial.print("\t");
    Serial.print(v, 4);Serial.print("\t");
    Serial.print(control, 4);Serial.print("\t");
    Serial.println(u, 4);
  }
  log_prescaler++;
}

void loop() {
  now = micros();
  dt = 1.0 * (now - lastTimeMicros) / 1000000;
  x = getCartDistance(encoderValue, PPR);
  v = (x - last_x) / dt;

  float control = - (Kx * (x - X0) + Kv * v);
  float u = (control + A * v - C * sign(v)) / B;
  u = 255.0 * u / 12.0;

  last_x = x;
  lastTimeMicros = now;

  driveMotor(saturate(u, 250));
  
  log_state(control, u);
  
  delay(5);
}

void encoderHandler() {
  int MSB = (PINE & (1 << PE5)) >> PE5; //MSB = most significant bit
  int LSB = (PINE & (1 << PE4)) >> PE4; //LSB = least significant bit
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderValue++; //CW
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue--; //CCW
  }

  lastEncoded = encoded; //store this value for next time  
}