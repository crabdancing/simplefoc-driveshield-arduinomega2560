#include <Arduino.h>
#include <SimpleFOC.h>

// pole pairs (14)
float PP = 7;
// phase resistance (ohms)
float R = .1 * 1.5;
float KV = 400;
float L = 0.00004;
int PIN_A = 5;
int PIN_B = 0;
int PIN_C = 6;
int PIN_ENABLE = 8;

int PIN_HALL_A = 12;
int PIN_HALL_B = 3;
int PIN_HALL_INDEX = 11;

BLDCMotor motor = BLDCMotor(PP, R, KV, L);

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


  // power supply voltage
  // default 12V
  driver.voltage_power_supply = 12;
  driver.init();
  // link the motor to the driver
  motor.linkDriver(&driver);

  // set control loop to be used
  motor.controller = MotionControlType::angle_openloop;
  
  // controller configuration based on the control type 
  // velocity PI controller parameters
  // default P=0.5 I = 10
  // motor.PID_velocity.P = 0.2;
  // motor.PID_velocity.I = 20;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  // motor.PID_velocity.output_ramp = 1000;
  
  //default voltage_power_supply
  motor.voltage_limit = 12;

  // velocity low pass filtering
  // default 5ms - try different values to see what is the best. 
  // the lower the less filtered
  // motor.LPF_velocity.Tf = 0.01;
  motor.LPF_velocity.Tf = 0.01;

  // angle P controller 
  // default P=20
  motor.P_angle.P = 20;
  //  maximal velocity of the position control
  // default 20
  motor.velocity_limit = 4;
  // motor.velocity_limit = 20;
  
  // initialize motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  // add target command T
  command.add('T', onTarget, "target angle");

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

