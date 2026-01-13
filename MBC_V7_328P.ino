/*
PROJECT : Malkani Bike Computer V7 Software
PURUPOSE: ISP.long, Road to <6:40 2k
COURSE  : TEJ3M
AUTHOR  : Zach Malkani '27
DATE    : Nov 30
MCU     : 328P
STATUS  : Working, not final
HARDWARE: MBC V2
NOTES   :
*/

//LCD Library inclusion
#include <LiquidCrystal.h>

// define input pins
#define INPUT_PIN A4
#define INPUT_PIN2 A2
#define INPUT_PIN3 A1
#define INPUT_PAUSE A0

// LCD Contrast PWM pin
#define V0_PIN 6

uint16_t rpm1 = 0;  // RPM for sensor 1
uint16_t rpm2 = 0;  // RPM for sensor 2

float meters = 0;  // meter count

const uint32_t debounceDelay = 20;  // debounce - input cooldown
const uint32_t timeOut = 2500;      // timout after no inputs

uint32_t lastLCDUpdate = 0;        // Tracks the last screen refresh
const uint16_t lcdInterval = 250;  // Update screen every 250ms (4 times per second)

// stopwatch variables
bool stopwatchRunning = false;  // not running on startup
bool stopwatchPaused = false;   // not paused on startup
float stopwatchStart = 0;       // set to 0millis on startup
float stopwatchElapsed = 0;

bool lastButtonState = false;       // pulled low on startup
bool lastPauseButtonState = false;  // pulled low on startup

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);  //parameters: LCD pins - rs, e, d4, d5, d6, d7

bool lastState = false;  //state during previous loop - 0V/low/false on startup
bool lastState2 = false;

float lastInputTime1 = 0;  //time since last input - 0 on startup
float lastInputTime2 = 0;

uint16_t inputsPerMinute1 = 0;
uint16_t inputsPerMinute2 = 0;
uint16_t V0val = 125;

// setup
void setup() {
  Serial.begin(9600);
  //setup pinModes at register level
  DDRC &= ~(1 << PC4);  // pinMode(A4, INPUT)
  DDRC &= ~(1 << PC2);  // pinMode(A2, INPUT)
  DDRC &= ~(1 << PC1);  // pinMode(A1, INPUT)
  DDRC &= ~(1 << PC0);  // pinMode(A0, INPUT)
  DDRD |= (1 << PD6);   // pinMode(6, OUTPUT)
  DDRD &= ~(1 << PD3);  // pinMode(3, INPUT)

  lcd.begin(16, 2);  //init LCD w/ x,y dimensions

  analogWrite(V0_PIN, V0val); //set lcd contrast pwm

  //load/intro screen
  lcd.setCursor(5, 0);  //(x, y) (rows,colums)
  lcd.print("MALKANI");
  lcd.setCursor(2, 1);
  lcd.print("BIKE COMPUTER");
  delay(2500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V7 Software");
  lcd.setCursor(0, 1);
  lcd.print("V3 Hardware");

  delay(2000);
  lcd.clear();
}

// loop
void loop() {
  // read all inputs at register level
  bool currentState = (PINC & (1 << PC4)) >> PC4;      // digitalRead(A4)
  bool currentState2 = (PINC & (1 << PC2)) >> PC2;     // digitalRead(A2)
  bool buttonState = (PINC & (1 << PC1)) >> PC1;       // digitalRead(A1)
  bool pauseButtonState = (PINC & (1 << PC0)) >> PC0;  // digitalRead(A0)

  //Button logic for stopwatch START/STOP/RESET
  if (buttonState == HIGH && lastButtonState == LOW) {  //if previously high on this loop and low before
    if (!stopwatchRunning) {                            // if stopwatch isnt already running - intended start from user
      stopwatchRunning = true;                          //start stopwatch
      stopwatchPaused = false;                          //ensure not paused
      stopwatchStart = millis() - stopwatchElapsed;     //stopwatch start time = current time - stopwatch elapsed time
      //on first start - stopwatchStart becomes current millis as elapsed = 0
      //when resuming from a pause (e.g. for 2s or 2000ms) - lets program know the time that has passed while paused

      //Serial.println("START"); test point
    } else if (stopwatchRunning) {  // if stopwatch is already running - intended reset from user
      stopwatchRunning = false;     // set to not running
      stopwatchPaused = false;      // set to unpaused
      stopwatchElapsed = 0;         // set elapsed back to 0
      lcd.setCursor(4, 1);
      lcd.print("    ");
      //Serial.println("RESET"); test point
      meters = 0;
    }
    delay(200);  // button debounce
  }
  lastButtonState = buttonState;  // set lastButton state to the state from the loop before, every loop

  // stop/reset/pause logic for stopwatch  ---
  if (stopwatchRunning && pauseButtonState == HIGH && lastPauseButtonState == LOW) {  // if stopwatch running and then paused, and previously not in such a state
    if (!stopwatchPaused) {                                                           // if not already paused -- PAUSE
      stopwatchPaused = true;                                                         // pause
      stopwatchElapsed = millis() - stopwatchStart;
    } else if (stopwatchPaused) {  // if already paused -- UNPAUSE
      stopwatchPaused = false;     // unpause
      stopwatchStart = millis() - stopwatchElapsed;
    }
    delay(200);  // button debounce
  }
  lastPauseButtonState = pauseButtonState;  // set laststate on next loop to state on current loop

  // update stopwatch elapsed time
  if (stopwatchRunning && !stopwatchPaused) {      // if running and not paused
    stopwatchElapsed = millis() - stopwatchStart;  //set elapsed time to current time - start time = time running
  }



  // sensor 1 reading cadence ---------------------
  if (!stopwatchPaused) {                                 // if not paused
    uint32_t currentT = millis();                         //set time at start of loop to current millis time
    if (currentState == HIGH && lastState == LOW) {       //if sensor reading and previously not
      if (currentT - lastInputTime1 > debounceDelay) {    // Only calculate if the time since the last valid pulse is greater than debounce delay
        uint32_t deltaTime = currentT - lastInputTime1;   // deltaTime = change in time since previous input
        if (lastInputTime1 != 0) {                        // if not on first input
          inputsPerMinute1 = 60000.0 / (float)deltaTime;  //inputs per minute = 60,000ms/change in time since last input, a.k.a. 60 seconds/change in time = ipm
        }
        lastInputTime1 = currentT;  // set last input time to current time as input has been logged
      }
    }

    // Reset to zero if pulse hasn't been seen in (timeOut) seconds
    if (currentT - lastInputTime1 > timeOut) inputsPerMinute1 = 0;
    lastState = currentState;
  }

  // Sensor 2 Reading (Speed) using same method as sensor 1 reading
  if (!stopwatchPaused) {
    uint32_t currentT = millis();
    if (currentState2 == HIGH && lastState2 == LOW) {
      // Debounce check
      if (currentT - lastInputTime2 > debounceDelay) {
        uint32_t deltaTime = currentT - lastInputTime2;
        if (lastInputTime2 != 0) {
          inputsPerMinute2 = 60000.0 / (float)deltaTime;
          meters += 2.136;  // add 1 wheel circumference to distance travelled
        }
        lastInputTime2 = currentT;
      }
    }

    // timeOut logic
    if (currentT - lastInputTime2 > timeOut) inputsPerMinute2 = 0;
    lastState2 = currentState2;
  }

  //set both RPM values to their IPM values now that theyre finished being calculated and round up using +.5 for decimal -> integer calcs
  rpm1 = (uint16_t)(inputsPerMinute1 + 0.5);
  rpm2 = (uint16_t)(inputsPerMinute2 + 0.5);

  //if (rpm1 > 9999) rpm1 = 9999;
  //if (rpm2 > 9999) rpm2 = 9999; ------------------------------------------------------------------------Delete this line and line above if all is well without them

  // speed calculations-----------
  float speed = (rpm2 * 2.136 * 60) / 1000;
  /*
  logic: # of revolutions/minute * wheel circumference * 60 minutes = distance, in KM travelled every hour, a.k.a KM/H

  formula: speed = (rpm *circumference* 60mins)/1000

  lastly, divide by 1000 to convert from millis to seconds
  */

  // Print to LCD every 250ms
  if (millis() - lastLCDUpdate >= lcdInterval) {  // if current time - last time is greater than or equal to the interval specified above
    lastLCDUpdate = millis();                     //reset print cooldown

    lcd.setCursor(0, 0);  // x,y
    lcd.print(rpm1);
    lcd.print("rpm   ");

    lcd.setCursor(0, 1);
    lcd.print("KMH:");
    lcd.setCursor(4, 1);
    lcd.print(speed, 1);
    lcd.print(" ");

    lcd.setCursor(9, 0);
    if (meters < 1000) {       //if meter count is sub-1km, print as meters
      lcd.print((int)meters);  // int meters so it doesnt show decimals from float calculation
      lcd.print("m ");         //units
    } else {
      lcd.print(meters / 1000.0, 2);  //if meter count is over 1km
      lcd.print("km");                //units
    }


    // stopwatch LCD reading
    uint32_t seconds = stopwatchElapsed / 1000;  // convert milliseconds to seconds
    uint32_t minutes = seconds / 60;             //grab minutes
    seconds = seconds % 60;                      //max 60

    lcd.setCursor(10, 1);
    if (minutes < 10) lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");
    if (seconds < 10) lcd.print("0");
    lcd.print(seconds);

    if (stopwatchPaused) {
      lcd.setCursor(15, 1);
      lcd.print(">");
    } else {
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
    //update LCD contrast pin, only needed to every 250ms
    OCR0A = V0val;  // analogWrite(6, V0val) access OCR0A (PWM pin 6) and set to contrast value
  }

  delay(10);  //extra debounce
}
