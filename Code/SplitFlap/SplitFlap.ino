#include <AccelStepper.h>

#define BUSY_LED                A9
#define STEPPER_COUNT           12
#define NUM_FLAPS               45
#define STEPS_PER_ROUND         4096
#define STEPS_PER_FLAP          (STEPS_PER_ROUND * 1.0 / NUM_FLAPS * 1.0)
#define ACCELERATION            10000.0
float   SPEED              =    600.0;

const uint8_t SENSOR_PINS[STEPPER_COUNT] = {  A0 ,  A2  ,  A4 ,  A6 ,  A8  , A10 , A12 , A14 , A1  , A3 ,  A5,  A7 };
const uint8_t ZERO_OFFSET[STEPPER_COUNT] = { 110 ,  100 ,  115 , 170 , 190 , 110 , 120 , 190 , 165 , 95 , 130, 100 };

AccelStepper steppers[STEPPER_COUNT] = {
  AccelStepper(AccelStepper::HALF4WIRE,   4,  3,  2,  5),
  AccelStepper(AccelStepper::HALF4WIRE,   8,  7,  6,  9),
  AccelStepper(AccelStepper::HALF4WIRE,  12, 11, 10, 13),
  AccelStepper(AccelStepper::HALF4WIRE,  20, 19, 18, 21),
  AccelStepper(AccelStepper::HALF4WIRE,  24, 23, 22, 25),
  AccelStepper(AccelStepper::HALF4WIRE,  28, 27, 26, 29),
  AccelStepper(AccelStepper::HALF4WIRE,  42, 41, 40, 43), 
  AccelStepper(AccelStepper::HALF4WIRE,  46, 45, 44, 47),
  AccelStepper(AccelStepper::HALF4WIRE,  38, 37, 36, 39),
  AccelStepper(AccelStepper::HALF4WIRE,  34, 33, 32, 35),
  AccelStepper(AccelStepper::HALF4WIRE,  14, 52, 15, 53),
  AccelStepper(AccelStepper::HALF4WIRE,  50, 49, 48, 51)
};

bool running[STEPPER_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

void setBusyLED(bool s) {
  digitalWrite(BUSY_LED, s);
}

void gotoZero(uint8_t moduleNum) {
  setBusyLED(true);

  Serial.print("gotoZero("); Serial.print(moduleNum);Serial.print(")");
  steppers[moduleNum].enableOutputs();
  steppers[moduleNum].setSpeed(SPEED);
  bool zeroPos = (digitalRead(SENSOR_PINS[moduleNum]) == 0);
  while (zeroPos) {
    zeroPos = (digitalRead(SENSOR_PINS[moduleNum]) == 0);
    steppers[moduleNum].runSpeed();
  }
  unsigned long zeroPosStartMillis = millis();
  while (!zeroPos && millis() - zeroPosStartMillis < 8000) {
    zeroPos = (digitalRead(SENSOR_PINS[moduleNum]) == 0);
    if (zeroPos) {
      Serial.println("  Zero Pos detected");
      steppers[moduleNum].setCurrentPosition(0);
      steppers[moduleNum].runToNewPosition(ZERO_OFFSET[moduleNum]);
    }
    steppers[moduleNum].runSpeed();
  }
  if (!zeroPos) Serial.println(" TIMEOUT!");
  steppers[moduleNum].setCurrentPosition(0);
  //Serial.print("setCurrentPosition(0) : ("); Serial.print(moduleNum); Serial.print(","), Serial.print(disable); Serial.println(")");
  steppers[moduleNum].disableOutputs();
  setBusyLED(false);
}

void gotoZeroAll() {
  for (uint8_t j = 0; j < STEPPER_COUNT; j++) {
    gotoZero(j);
  }
  Serial.println("gotoZeroAll -> Finish!");
}

void gotoLetter(char printletter, uint8_t moduleNum) {
  Serial.print("gotoLetter("); Serial.print(printletter); Serial.print(","); Serial.print(moduleNum); Serial.println(")");

  uint16_t stepsFromZero = 0;

  for (uint8_t i = 0; i < letters.length(); i++) {
    if (letters[i] == printletter) {
      break;
    }
    stepsFromZero += STEPS_PER_FLAP;
  }

  uint16_t currentPosition = steppers[moduleNum].currentPosition();

  uint16_t steps = (stepsFromZero > currentPosition) ? stepsFromZero - currentPosition : (STEPS_PER_ROUND - currentPosition) + stepsFromZero;

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
    return isBusy;
}

void setup() {
  Serial.begin(57600);
  Serial2.begin(57600);
  initHW();
}

void loop() {
  bool busy = processFlapRun();

  setBusyLED(busy);

  bool newChar = false;
  if (!busy && (Serial.available() > 0 || Serial2.available() > 0)) {
    char in = (Serial.available() > 0) ? Serial.read() : Serial2.read();
    if (in == '\n') {
      newChar = true;
    } else {
      serialInput += in;
    }
  }

  if (newChar) {
    newChar = false;

    if (serialInput[0] == '%') {
      switch (serialInput[1]) {
      case 'z':
        if (serialInput[2] == 'a') {
          gotoZeroAll();
        } else {
          int modnum = atoi(serialInput.substring(2).c_str());
          gotoZero(modnum);
        }
        break;
      case 's':
        SPEED = atoi(serialInput.substring(2).c_str()) * 1.0;
        Serial.print("Setting SPEED to ");Serial.println(SPEED, DEC);
        break;
      }
    }

    if (serialInput.length() == (STEPPER_COUNT + 1)) {
      serialInput.toUpperCase();
      for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
        char letter = serialInput[i];
        gotoLetter(letter, i);
      }
    }
    serialInput = "";
  }
}
