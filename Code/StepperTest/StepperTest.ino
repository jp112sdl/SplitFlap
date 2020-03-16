#include <AccelStepper.h>

AccelStepper stepper = AccelStepper(AccelStepper::HALF4WIRE,12, 11, 10, 13);
#define SENSOR_PIN     A4
#define ZERO_OFFSET    90

#define SPEED 500
#define NUM_FLAPS               45
#define STEPS_PER_ROUND         4096
#define STEPS_PER_FLAP          (STEPS_PER_ROUND * 1.0 / NUM_FLAPS * 1.0)
  bool a = true;


unsigned long lastmillis = 0;

void gotoZero() {
  TWBR = 2;  // 12 = 400 kHz; 2 = 800 kHz

  stepper.enableOutputs();
  bool zeroPos =  (digitalRead(SENSOR_PIN) == 0);
  Serial.println("SENSOR PIN is " + String(zeroPos));

  while (zeroPos) {
    zeroPos = (digitalRead(SENSOR_PIN) == 0);
    stepper.runSpeed();
  }

  while (!zeroPos) {
    zeroPos = (digitalRead(SENSOR_PIN) == 0);
    if (zeroPos) {
      Serial.println("Zero Pos");
      stepper.setCurrentPosition(0);
      stepper.runToNewPosition(ZERO_OFFSET);
    }
    stepper.runSpeed();
  }
  stepper.setCurrentPosition(0);
  //stepper.disableOutputs();
}

void setup() {
  Serial.begin(57600);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  stepper.setMaxSpeed(SPEED);
  stepper.setAcceleration(10000);
  stepper.setSpeed(SPEED);


  if (!a) {
    gotoZero();
  delay(5000);
  }
  //stepper.runToNewPosition(4096);


}

void loop() {


  if (a) {
    stepper.runSpeed();
  } else {

    stepper.run();
    if (millis() - lastmillis > 2000) {
      lastmillis = millis();
      stepper.move(STEPS_PER_FLAP);

    }
  }

}
