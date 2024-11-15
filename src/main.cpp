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

int PIN_ENCODER_A = 3;
int PIN_ENCODER_B = 2;
int PIN_ENCODER_INDEX = 11;

BLDCMotor motor = BLDCMotor(PP, R, KV, L);
float motor_enabled = 0.0;

// Uses ACS712. Value docs use is 66.0 mVpa. Source:
// https://docs.simplefoc.com/inline_current_sense
// antun says scale value from 3.3v range to 5v range
InlineCurrentSense current_sense =
    InlineCurrentSense(66.0 * (3.3 / 5), CURRENT_SENSE_1, _NC, CURRENT_SENSE_3);

// init driver
BLDCDriver3PWM driver = BLDCDriver3PWM(PIN_A, PIN_B, PIN_C, PIN_ENABLE);
//  init encoder
Encoder encoder = Encoder(PIN_ENCODER_A, PIN_ENCODER_B, 2048);
// channel A and B callbacks
void doA() { encoder.handleA(); }
void doB() { encoder.handleB(); }

// angle set point variable
float target_angle = 0;
// commander interface
Commander command = Commander(Serial);
void onTargetChange(char *cmd) { command.scalar(&target_angle, cmd); }

void report_pid() {
  Serial.print("P value set to: ");
  Serial.println(motor.PID_velocity.P, 4);
  Serial.print("I value set to: ");
  Serial.println(motor.PID_velocity.I, 4);
  Serial.print("D value set to: ");
  Serial.println(motor.PID_velocity.D, 4);
  Serial.print("R value set to: ");
  Serial.println(motor.PID_velocity.output_ramp, 4);
  Serial.print("L value set to: ");
  Serial.println(motor.PID_velocity.limit, 4);
}

void onPChange(char *cmd) {
  command.scalar(&motor.PID_velocity.P, cmd);
  report_pid();
}

void onIChange(char *cmd) {
  command.scalar(&motor.PID_velocity.I, cmd);
  report_pid();
}

void onDChange(char *cmd) {
  command.scalar(&motor.PID_velocity.D, cmd);
  report_pid();
}

void onRChange(char *cmd) {
  command.scalar(&motor.PID_velocity.output_ramp, cmd);
  report_pid();
}

void onLChange(char *cmd) {
  command.scalar(&motor.PID_velocity.limit, cmd);
  report_pid();
}

// void onPIDChange(char *cmd) { command.pid(&motor.PID_velocity, cmd); }
void onMotorEnableDisable(char *cmd) {
  command.scalar(&motor_enabled, cmd);
  if (motor_enabled == 1.0) {
    Serial.println("Enabled motor!");
  } else {
    Serial.println("Disabled motor!");
  }
}

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
  motor.PID_velocity.D = 0.02;
  motor.PID_velocity.P = 0.0;
  motor.PID_velocity.I = 0.0;
  // motor.PID_velocity.P = 0.5;
  // motor.PID_velocity.I = 10;
  // motor.PID_velocity.D = 0.002;
  // motor.PID_velocity.D = 0.004;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  // motor.PID_velocity.output_ramp = 1000;

  // default voltage_power_supply
  motor.voltage_limit = 12;
  motor.current_limit = 5;

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
  command.add('t', onTargetChange, "change target angle");
  command.add('p', onPChange, "change P");
  command.add('i', onIChange, "change I");
  command.add('d', onDChange, "change D");
  command.add('r', onRChange, "change r");
  command.add('l', onLChange, "change l");
  command.add('e', onMotorEnableDisable, "change motor enabled state");
  // command.add(, )
  // command.target(, , )
  // command.pid(, 'p');

  Serial.println("Motor ready.");
  Serial.println("Set the target angle using serial terminal:");
  _delay(1000);
}

void loop() {

  // function calculating the outer position loop and setting the target
  // position
  if (motor_enabled == 1.0) {
    motor.enable();
    motor.monitor();
    // iterative FOC function
    motor.loopFOC();
    motor.move(target_angle);
  } else {
    motor.disable();
  }

  // user communication
  command.run();
}
