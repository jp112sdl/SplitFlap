#include <AccelStepper.h>

#define BUSY_LED                LED_BUILTIN
#define STEPPER_COUNT           8
#define NUM_FLAPS               45
#define STEPS_PER_ROUND         4096
#define STEPS_PER_FLAP          (STEPS_PER_ROUND * 1.0 / NUM_FLAPS * 1.0)
#define SPEED                   1000.0
#define ACCELERATION            10000.0

const uint8_t SENSOR_PINS[STEPPER_COUNT] = { A0 , 0, 0, 0, 0, 0, 0, 0 };
const uint8_t ZERO_OFFSET[STEPPER_COUNT] = { 100, 0, 0, 0, 0, 0, 0, 0 };

AccelStepper steppers[STEPPER_COUNT] = {
  AccelStepper(AccelStepper::HALF4WIRE,  4,  6,  5,  7),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0),
  AccelStepper(AccelStepper::HALF4WIRE,  0,  0,  0,  0)
};

bool running[STEPPER_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};

String serialInput = "";
//                                                     ÄÖÜ ß
String letters = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[](-)!?.";

unsigned long lastmillis = 0;
uint8_t       cnt = 0;

void initHW() {
  Serial.println("initHW()");
  for (uint8_t j = 0; j < STEPPER_COUNT; j++) {
    steppers[j].setMaxSpeed(SPEED);
    steppers[j].setAcceleration(ACCELERATION);
    steppers[j].setSpeed(SPEED);
    pinMode(SENSOR_PINS[j], INPUT_PULLUP);
  }
  pinMode(BUSY_LED, OUTPUT);
}

void gotoZero(uint8_t moduleNum, bool disable) {
  TWBR = 2;  // 12 = 400 kHz; 2 = 800 kHz

  Serial.print("gotoZero("); Serial.print(moduleNum); Serial.print(","), Serial.print(disable); Serial.println(")");
  steppers[moduleNum].enableOutputs();
  bool zeroPos =  (digitalRead(SENSOR_PINS[moduleNum]) == 0);
  while (!zeroPos) {
    zeroPos = (digitalRead(SENSOR_PINS[moduleNum]) == 0);
    if (zeroPos) {
      Serial.println("Zero Pos");
      steppers[moduleNum].setCurrentPosition(0);
      steppers[moduleNum].runToNewPosition(ZERO_OFFSET[moduleNum]);
    }
    steppers[moduleNum].runSpeed();
  }
  steppers[moduleNum].setCurrentPosition(0);
  Serial.print("setCurrentPosition(0) : ("); Serial.print(moduleNum); Serial.print(","), Serial.print(disable); Serial.println(")");
  if (disable) steppers[moduleNum].disableOutputs();
}

void gotoZeroAll() {
  for (uint8_t j = 0; j < STEPPER_COUNT; j++) {
    gotoZero(j, false);
  }
}

void gotoLetter(char printletter, uint8_t moduleNum) {
  Serial.print("gotoLetter("); Serial.print(printletter); Serial.print(","); Serial.print(moduleNum); Serial.println(")");
  TWBR = 2;  // 12 = 400 kHz; 2 = 800 kHz

  uint16_t stepsFromZero = 0;

  for (uint8_t i = 0; i < letters.length(); i++) {
    if (letters[i] == printletter) {
      break;
    }
    stepsFromZero += STEPS_PER_FLAP;
  }

  uint16_t currentPosition = steppers[moduleNum].currentPosition();

  uint16_t steps = (stepsFromZero > currentPosition) ? stepsFromZero - currentPosition : (STEPS_PER_ROUND - currentPosition) + stepsFromZero;

  //Serial.print("gotoLetter : currentPosition = "); Serial.println(currentPosition, DEC);
  //Serial.print("gotoLetter :   stepsFromZero = "); Serial.println(stepsFromZero, DEC);
  //Serial.print("gotoLetter :           steps = "); Serial.println(steps, DEC);

  if (steps != STEPS_PER_ROUND) steppers[moduleNum].move(steps);
}

bool processFlapRun() {
  bool isBusy = false;
  for (uint8_t j = 0; j < STEPPER_COUNT; j++) {
    if (steppers[j].distanceToGo() != 0 && !running[j]) {
      Serial.print("Enable  Steppers ("); Serial.print(j, DEC); Serial.println(")");
      steppers[j].enableOutputs();
      running[j] = true;
    }

    steppers[j].run();

    if (steppers[j].distanceToGo() == 0 && running[j]) {
      Serial.print("Disable Steppers ("); Serial.print(j, DEC); Serial.println(")");
      if (steppers[j].currentPosition() > STEPS_PER_ROUND) {
        steppers[j].setCurrentPosition(steppers[j].currentPosition() - STEPS_PER_ROUND);
      }
      steppers[j].disableOutputs();
      running[j] = false;
    }

    if (running[j]) lastmillis = millis();
    if (running[j]) isBusy = true;
  }
  digitalWrite(BUSY_LED, isBusy);
  return isBusy;
}

void setup() {
  Serial.begin(57600);

  initHW();

  gotoZero(0, true);
  //gotoZeroAll();

}

void loop() {
  bool busy = processFlapRun();

  bool newChar = false;
  if (!busy && Serial.available() > 0) {
    char in = Serial.read();
    serialInput += in;

    if (in == '\n') {
      newChar = true;
    }
  }

  if (newChar) {
    newChar = false;
    if (serialInput.length() == (STEPPER_COUNT + 1)) {
      for (uint8_t i = 0; i < STEPPER_COUNT; i++){
        char letter = serialInput[i];
        gotoLetter(letter, i);
      }
    }
    serialInput = "";
  }
}
