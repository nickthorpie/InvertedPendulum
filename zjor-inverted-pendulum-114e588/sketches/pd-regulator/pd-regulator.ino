/**
 * This sketch demonstrates how to bring a cart to [x: 0; v: 0] from non-zero position using PD-regulator
 */
#include <float.h>
#include <limits.h>
 
#define STEP_PIN    9
#define DIR_PIN     10
#define PULSE_WIDTH 2

#define POSITION_LIMIT  1700

// upright zero angle measurement
#define ANGLE_ZERO  505.75

#define aKp  80.0
#define aKd  6.0

#define Kp  1.6
#define Kd  0.9

#define P0  0
#define V0  0.0
#define x0  0.0

volatile boolean adcReady = false;
volatile int adcValue;

boolean isStabilizing = false;

int rawAngle;

float a = .0;         // meters per second^2
float v = V0;         // meters per second
float f = 1000000.0;  // micros per second
float x = x0;         // calculated position

// prevoius values for improved Euler's method
float last_a = .0;
float last_v = .0;

int direction = HIGH;
long position = P0;

unsigned long stepDelay = 0;
unsigned long lastStepTime = 0;

unsigned long lastAngleUpdateTime = 0;

// angle update and integration period in microseconds
unsigned long angleUpdatePeriod = 5000;

float angle = .0;
float lastAngle = .0;
float omega = .0;
float lastOmega = .0;

void adc_init() {
    ADMUX  = _BV(REFS0)    // ref = AVCC
           | 0;         // input channel
    ADCSRB = 0;          // free running mode
    ADCSRA = _BV(ADEN)   // enable
           | _BV(ADSC)   // start conversion
           | _BV(ADATE)  // auto trigger enable
           | _BV(ADIF)   // clear interrupt flag
           | _BV(ADIE)   // interrupt enable
           | 7;          // prescaler = 128
}

void setup() {  
  pinMode(STEP_PIN, OUTPUT); 
  pinMode(DIR_PIN, OUTPUT);
  
  adc_init();
  Serial.begin(115200);
}

long i = 0;

void loop() {
  updateAngleAndDerivative();
  runMotor();

//  if (i % 500 == 0) {
//    Serial.print(rawAngle);
//    Serial.print("\t");    
//    Serial.print(angle, 6);
//    Serial.print("\t");
//    Serial.print(omega, 6);
//    Serial.print("\t");
//    Serial.print(x, 6);
//    Serial.print("\t");
//    Serial.print(v, 6);
//    Serial.print("\t");    
//    Serial.println(a, 6);    
//  }
//  i++;
  
}

float swingUpControl(float th, float dth) {
  return -0.2 * dth * cos(th);
}

float getControl(float th, float omega, float x, float v) {
  if (!isStabilizing) {
    return 0.0;
  }
  return - (Kp * x + Kd * v + aKp * th + aKd * omega);
}

void integrate(float dt) {
  a = getControl(angle, omega, x, v);
//  a = swingUpControl(angle, omega);
  v += (a + last_a) * dt / 2;
  x += (v + last_v) * dt / 2;
  stepDelay = getStepDelay(v);
  
  last_a = a;
  last_v = v;
}

inline boolean isControllable(int angle) {
  return abs(rawAngle - ANGLE_ZERO) < 3;
}

void updateAngleAndDerivative() {

  if (adcReady) {      
    rawAngle = adcValue;
    adcReady = false;
    isStabilizing = (!isStabilizing && isControllable(rawAngle)) || isStabilizing;
  }
  
  unsigned long now = micros();
  if (now - lastAngleUpdateTime >= angleUpdatePeriod) {    
    angle = normalizeAngle(rawAngle);

    float dt = 1.0 * (now - lastAngleUpdateTime) / f;
    omega = (angle - lastAngle) / dt;
//    omega = (omega + lastOmega) / 2;
    
    integrate(dt);
    lastAngle = angle;
    lastOmega = omega;
    
    lastAngleUpdateTime = now;
  }
}

float normalizeAngle(int value) {  
  return fmap(value - ANGLE_ZERO, 0, 1024, 0, 2.0 * PI);
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


unsigned long getStepDelay(float speed) {
  direction = (speed > 0.0) ? HIGH : LOW;  
  return max(400, (speed == 0) ? ULONG_MAX : (f / fabs(10000.0 * speed) - PULSE_WIDTH));
}

void runMotor() {
  unsigned long now = micros();
  if (now - lastStepTime >= stepDelay) {
    step();
    lastStepTime = now;  
  }
}

void step() {
  if (abs(position) > POSITION_LIMIT) {
    return; 
  }
  
  digitalWrite(DIR_PIN, direction);
  digitalWrite(STEP_PIN, HIGH); 
  delayMicroseconds(PULSE_WIDTH); 
  digitalWrite(STEP_PIN, LOW);
  position += (direction == HIGH) ? 1: -1;
}

ISR (ADC_vect) {
  adcValue = ADC;
  adcReady = true;  
}
