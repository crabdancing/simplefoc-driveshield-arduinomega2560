#include <Arduino.h>
#include <SimpleFOC.h>

// pole pairs (14)
float PP = 7;
// phase resistance (ohms)
float R = .1 * 1.5;
float KV = 400;
float L = 0.00004;
int PIN_A = 5;
int PIN_B = 9;
int PIN_C = 6;
int PIN_ENABLE = 7;

int CURRENT_SENSE_3 = PIN_A2;
int CURRENT_SENSE_1 = PIN_A1;

int PIN_HALL_A = 3;
int PIN_HALL_B = 2;
int PIN_HALL_INDEX = 11;

BLDCMotor motor = BLDCMotor(PP, R, KV, L);

// Uses ACS712. Value docs use is 66.0 mVpa. Source: https://docs.simplefoc.com/inline_current_sense
InlineCurrentSense current_sense = InlineCurrentSense(66.0, CURRENT_SENSE_1, _NC, CURRENT_SENSE_3);


// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(PIN_A, PIN_B, PIN_C, PIN_ENABLE);
//  init encoder
Encoder encoder = Encoder(PIN_HALL_A, PIN_HALL_B, 2048);
// channel A and B callbacks
void doA(){encoder.handleA();}
void doB(){encoder.handleB();}

// angle set point variable
float target_angle = 0;
// commander interface
Commander command = Commander(Serial);
void onTarget(char* cmd){ command.scalar(&target_angle, cmd); }

void setup() {
  // monitoring port
  Serial.begin(115200);
  SimpleFOCDebug::enable();
  motor.useMonitoring(Serial);


  // initialize encoder hardware
  encoder.init();

  // hardware interrupt enable
  encoder.enableInterrupts(doA, doB);
  // link the motor to the sensor
  motor.linkSensor(&encoder);

  driver.init();

  // power supply voltage
  // default 12V
  driver.voltage_power_supply = 20;

  // note: current_sense.init() must be AFTER driver.init()
  if (current_sense.init())  Serial.println("Current sense init success!");
  else {
    Serial.println("Current sense init failed!");
    return;
  }

  current_sense.linkDriver(&driver);
  
  motor.linkDriver(&driver);

  // set control loop to be used
  motor.controller = MotionControlType::angle;
  // controller configuration based on the control type 
  // velocity PI controller parameters
  // default P=0.5 I = 10
  // motor.PID_velocity.P = 0.2;
  // motor.PID_velocity.I = 20;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  // motor.PID_velocity.output_ramp = 1000;
  
  //default voltage_power_supply
  motor.voltage_limit = 2;

  // velocity low pass filtering
  // default 5ms - try different values to see what is the best. 
  // the lower the less filtered
  // motor.LPF_velocity.Tf = 0.01;
  motor.LPF_velocity.Tf = 0.1;

  // angle P controller 
  // default P=20
  motor.P_angle.P = 20;
  //  maximal velocity of the position control
  // default 20
  // motor.velocity_limit = 4;
  motor.velocity_limit = 20;
  
  // initialize motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  // add target command T
  command.add('t', onTarget, "target angle");

  Serial.println("Motor ready.");
  Serial.println("Set the target angle using serial terminal:");
  _delay(1000);
}

void loop() {
  motor.monitor();
  // iterative FOC function
  motor.loopFOC();

  // function calculating the outer position loop and setting the target position 
  motor.move(target_angle);

  // user communication
  command.run();
}

