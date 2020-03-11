#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <AccelStepper.h>
#include <MCP3017AccelStepper.h>

#define FLAP_MODULE_BLOCKS      2
#define STEPPER_COUNT_PER_BLOCK 4
#define NUM_FLAPS               45
#define STEPS_PER_ROUND         4096
#define STEPS_PER_FLAP          (STEPS_PER_ROUND * 1.0 / NUM_FLAPS * 1.0)

const uint8_t SENSOR_PINS[FLAP_MODULE_BLOCKS][STEPPER_COUNT_PER_BLOCK] = {{ A0, 3, 4, 5 }};//, { 6, 7, 8, 9 }};
const uint8_t ZERO_OFFSET[FLAP_MODULE_BLOCKS][STEPPER_COUNT_PER_BLOCK] = {{ 90, 0, 0, 0 }};//, { 0, 0, 0, 0 }};

typedef struct {
  MCP3017AccelStepper steppers[STEPPER_COUNT_PER_BLOCK] = {
    MCP3017AccelStepper(AccelStepper::HALF4WIRE,  0,  2,  1,  3),
    MCP3017AccelStepper(AccelStepper::HALF4WIRE,  4,  6,  5,  7),
    MCP3017AccelStepper(AccelStepper::HALF4WIRE,  8, 10,  9, 11),
    MCP3017AccelStepper(AccelStepper::HALF4WIRE, 12, 14, 13, 15)
  };
  bool running[STEPPER_COUNT_PER_BLOCK] = {0, 0, 0, 0};
  Adafruit_MCP23017 mcp;
} FlapModuleBlockType;

FlapModuleBlockType FlapModuleBlock[FLAP_MODULE_BLOCKS];

//                ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ÄÖÜ-ß!?.
String letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[](-)!?. ";

unsigned long lastmillis = 0;
uint8_t       cnt = 0;

void initHW() {
  Serial.println("initHW()");

  for (uint8_t i = 0; i < FLAP_MODULE_BLOCKS; i++) {
    FlapModuleBlock[i].mcp.begin(0 + i);
    for (uint8_t j = 0; j < STEPPER_COUNT_PER_BLOCK; j++) {
      FlapModuleBlock[i].steppers[j].setMcp(FlapModuleBlock[i].mcp);
      FlapModuleBlock[i].steppers[j].setMaxSpeed(1000);
      FlapModuleBlock[i].steppers[j].setAcceleration(8000);
      FlapModuleBlock[i].steppers[j].setSpeed(1000);
      pinMode(SENSOR_PINS[i][j], INPUT_PULLUP);
    }
  }
}

void gotoZero(uint8_t blockNum, uint8_t moduleNum, bool disable) {
  TWBR = 2;  // 12 = 400 kHz; 2 = 800 kHz

  Serial.print("gotoZero("); Serial.print(blockNum); Serial.print(","), Serial.print(moduleNum); Serial.print(","), Serial.print(disable); Serial.println(")");
  FlapModuleBlock[blockNum].steppers[moduleNum].enableOutputs();
  bool zeroPos = false;
  while (!zeroPos) {
    zeroPos = (digitalRead(SENSOR_PINS[blockNum][moduleNum]) == 0);
    if (zeroPos) {
      Serial.println("Zero Pos");
      FlapModuleBlock[blockNum].steppers[moduleNum].setCurrentPosition(0);
      FlapModuleBlock[blockNum].steppers[moduleNum].runToNewPosition(ZERO_OFFSET[blockNum][moduleNum]);
    }
    FlapModuleBlock[blockNum].steppers[moduleNum].runSpeed();
  }
  FlapModuleBlock[blockNum].steppers[moduleNum].setCurrentPosition(0);
  Serial.print("setCurrentPosition(0) : ("); Serial.print(blockNum); Serial.print(","), Serial.print(moduleNum); Serial.print(","), Serial.print(disable); Serial.println(")");
  if (disable) FlapModuleBlock[blockNum].steppers[moduleNum].disableOutputs();
}

void gotoZeroAll() {
  for (uint8_t i = 0; i < FLAP_MODULE_BLOCKS; i++) {
    for (uint8_t j = 0; j < STEPPER_COUNT_PER_BLOCK; j++) {
      gotoZero(i, j, false);
    }
  }
}

void gotoLetter(char printletter, uint8_t blockNum, uint8_t moduleNum) {
  Serial.print("gotoLetter("); Serial.print(printletter); Serial.print(","); Serial.print(blockNum); Serial.print(","), Serial.print(moduleNum); Serial.println(")");
  TWBR = 2;  // 12 = 400 kHz; 2 = 800 kHz

  uint16_t stepsFromZero = 0;

  for (uint8_t i = 0; i < letters.length(); i++) {
    if (letters[i] == printletter) {
      break;
    }
    stepsFromZero += STEPS_PER_FLAP;
  }

  Serial.print("gotoLetter : stepsFromZero = "); Serial.println(stepsFromZero, DEC);

  uint16_t currentPosition = FlapModuleBlock[blockNum].steppers[moduleNum].currentPosition();

  uint16_t  steps = (stepsFromZero > currentPosition) ? stepsFromZero - currentPosition : (STEPS_PER_ROUND - currentPosition) + stepsFromZero;

  FlapModuleBlock[blockNum].steppers[moduleNum].move(steps);
}

void processFlapRun() {
  for (uint8_t i = 0; i < FLAP_MODULE_BLOCKS; i++) {
    for (uint8_t j = 0; j < STEPPER_COUNT_PER_BLOCK; j++) {

      if (FlapModuleBlock[i].steppers[j].distanceToGo() != 0 && !FlapModuleBlock[i].running[j]) {
        Serial.print("Enable  Steppers ("); Serial.print(i, DEC); Serial.print(","); Serial.print(j, DEC); Serial.println(")");
        FlapModuleBlock[i].steppers[j].enableOutputs();
        FlapModuleBlock[i].running[j] = true;
      }

      FlapModuleBlock[i].steppers[j].run();

      if (FlapModuleBlock[i].steppers[j].distanceToGo() == 0 && FlapModuleBlock[i].running[j]) {
        Serial.print("Disable Steppers ("); Serial.print(i, DEC); Serial.print(","); Serial.print(j, DEC); Serial.println(")");
        if (FlapModuleBlock[i].steppers[j].currentPosition() > STEPS_PER_ROUND) {
          FlapModuleBlock[i].steppers[j].setCurrentPosition(FlapModuleBlock[i].steppers[j].currentPosition() - STEPS_PER_ROUND);
        }
        FlapModuleBlock[i].steppers[j].disableOutputs();
        FlapModuleBlock[i].running[j] = false;
      }
    }
  }
}

void setup() {
  Serial.begin(57600);

  initHW();

  gotoZero(0, 0, false);
  //gotoZeroAll();

}

void loop() {
  processFlapRun();

  if (millis() - lastmillis > 8000) {
    lastmillis = millis();

    if (cnt == 0) {
      Serial.println("Goto J");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('J', 0, 0);
    }

    if (cnt == 1) {
      Serial.println("Goto E");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('E', 0, 0);
    }

    if (cnt == 2) {
      Serial.println("Goto R");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('R', 0, 0);
    }

    if (cnt == 3) {
      Serial.println("Goto O");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('O', 0, 0);
    }

    if (cnt == 4) {
      Serial.println("Goto M");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('M', 0, 0);
    }

    if (cnt == 5) {
      Serial.println("Goto E");
      Serial.print("currentPosition: "); Serial.print(FlapModuleBlock[0].steppers[0].currentPosition(), DEC); Serial.println("");
      gotoLetter('E', 0, 0);
    }

    cnt++;
    if (cnt == 7) cnt = 0;
  }



}
