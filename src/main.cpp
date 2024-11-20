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

float flop_ms_delay = 200;

int CURRENT_SENSE_3 = PIN_A2;
int CURRENT_SENSE_1 = PIN_A1;

int PIN_ENCODER_A = 3;
int PIN_ENCODER_B = 2;
int PIN_ENCODER_INDEX = 21;

bool motor_enabled = false;
bool old_motor_enabled = true;

BLDCMotor motor = BLDCMotor(PP, R, KV, L);

// Uses ACS712. Value docs use is 66.0 mVpa. Source:
// https://docs.simplefoc.com/inline_current_sense
// antun says scale value from 3.3v range to 5v range
InlineCurrentSense current_sense =
    InlineCurrentSense(66.0 * (3.3 / 5), CURRENT_SENSE_1, _NC, CURRENT_SENSE_3);

// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(PIN_A, PIN_B, PIN_C, PIN_ENABLE);
//  init encoder
Encoder encoder = Encoder(PIN_ENCODER_A, PIN_ENCODER_B, 512, PIN_ENCODER_INDEX);
// channel A and B callbacks
void doA() { encoder.handleA(); }
void doB() { encoder.handleB(); }
void doX() { encoder.handleIndex(); }

// angle set point variable
float target_angle = 0;
// commander interface
Commander command = Commander(Serial);
void onTargetAngleChange(char *cmd) { command.scalar(&target_angle, cmd); }

double degreesToRadians(double degrees) { return degrees * (PI / 180.0); }

void report_state() {
  Serial.println("Current state:");
  Serial.print("p");
  Serial.println(motor.PID_velocity.P, 7);
  Serial.print("i");
  Serial.println(motor.PID_velocity.I, 7);
  Serial.print("d");
  Serial.println(motor.PID_velocity.D, 7);
  Serial.print("r");
  Serial.println(motor.PID_velocity.output_ramp, 7);
  Serial.print("l");
  Serial.println(motor.PID_velocity.limit, 7);
  Serial.print("f");
  Serial.println(flop_ms_delay);
}

void onPChange(char *cmd) {
  command.scalar(&motor.PID_velocity.P, cmd);
  report_state();
}

void onIChange(char *cmd) {
  command.scalar(&motor.PID_velocity.I, cmd);
  report_state();
}

void onDChange(char *cmd) {
  command.scalar(&motor.PID_velocity.D, cmd);
  report_state();
}

void onRChange(char *cmd) {
  command.scalar(&motor.PID_velocity.output_ramp, cmd);
  report_state();
}

void onLChange(char *cmd) {
  command.scalar(&motor.PID_velocity.limit, cmd);
  report_state();
}
void onFChange(char *cmd) {
  command.scalar(&flop_ms_delay, cmd);
  report_state();
}

// void onPIDChange(char *cmd) { command.pid(&motor.PID_velocity, cmd); }
void onMotorEnableDisable(char *cmd) {
  float motor_enabled_float = 0.0;
  command.scalar(&motor_enabled_float, cmd);
  if (motor_enabled_float == 1.0) {
    motor_enabled = true;
    Serial.println("Enabled motor!");
  } else {
    motor_enabled = false;
    Serial.println("Disabled motor!");
  }
  report_state();
}

unsigned long time_since_last_flip = 0;
unsigned long current_time = 0;

void setup() {
  current_time = millis();
  time_since_last_flip = current_time;
  // monitoring port
  Serial.begin(115200);
  SimpleFOCDebug::enable();
  motor.useMonitoring(Serial);

  // initialize encoder hardware
  encoder.init();

  // hardware interrupt enable
  encoder.enableInterrupts(doA, doB, doX);
  // link the motor to the sensor
  motor.linkSensor(&encoder);
  // motor.disable();

  driver.init();

  // power supply voltage
  // default 12V
  driver.voltage_power_supply = 12;

  current_sense.linkDriver(&driver);

  // note: current_sense.init() must be AFTER driver.init()
  if (current_sense.init())
    Serial.println("Current sense init success!");
  else {
    Serial.println("Current sense init failed!");
    return;
  }

  motor.linkCurrentSense(&current_sense);

  motor.linkDriver(&driver);

  // set control loop to be used
  motor.controller = MotionControlType::angle;
  // controller configuration based on the control type
  // velocity PI controller parameters
  // default P=0.5 I = 10
  motor.PID_velocity.P = 0.7000;
  motor.PID_velocity.I = 0.0900;
  motor.PID_velocity.D = 0.0002;
  motor.PID_velocity.output_ramp = 2000.0000;
  motor.PID_velocity.limit = 12.0000;
  // motor.PID_velocity.P = 0.5;
  // motor.PID_velocity.I = 10;
  // motor.PID_velocity.D = 0.002;
  // motor.PID_velocity.D = 0.004;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  // motor.PID_velocity.output_ramp = 1000;

  // default voltage_power_supply
  motor.voltage_limit = 12;
  motor.current_limit = 12;

  // velocity low pass filtering
  // default 5ms - try different values to see what is the best.
  // the lower the less filtered
  // motor.LPF_velocity.Tf = 0.01;
  motor.LPF_velocity.Tf = 0.1;
  // motor.LPF_velocity.Tf = 0.1;
  // motor.LPF_velocity.Tf = 0.05;

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
  command.add('a', onTargetAngleChange, "change target angle");
  command.add('p', onPChange, "change P");
  command.add('i', onIChange, "change I");
  command.add('d', onDChange, "change D");
  command.add('r', onRChange, "change R");
  command.add('l', onLChange, "change L");
  command.add('f', onFChange, "change flop ms delay");
  command.add('e', onMotorEnableDisable, "change motor enabled state");

  Serial.println("Motor ready.");

  report_state();
}

bool flip_flop_state = false;

void loop() {
  command.run();
  current_time = millis();
  if ((current_time - time_since_last_flip) > flop_ms_delay) {
    time_since_last_flip = current_time;
    // Serial.println("one second elapsed");
    flip_flop_state = !flip_flop_state;
    Serial.print("Current draw (A): ");
    Serial.println(current_sense.getPhaseCurrents().a);
    Serial.print("Current draw (B): ");
    Serial.println(current_sense.getPhaseCurrents().b);
  }

  if (motor_enabled) {
    motor.monitor();
    // iterative FOC function
    motor.loopFOC();

    // if (flip_flop_state) {
    //   motor.move(degreesToRadians(30));
    // } else {

    //   motor.move(degreesToRadians(-30));
    // }

    motor.move(degreesToRadians(target_angle));
  }

  if (motor_enabled && (!old_motor_enabled)) {
    Serial.println("Motor is now enabled. Propagating state...");
    motor.enable();
  }

  if ((!motor_enabled) && old_motor_enabled) {
    Serial.println("Motor is now disabled. Propagating state...");
    motor.disable();
  }

  old_motor_enabled = motor_enabled;
}
