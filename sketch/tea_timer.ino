#include <Arduino.h>	         // Define normal Arduino Library
#include <Wire.h>              // Needed for I2C usage 
#include <LiquidCrystal_I2C.h> // I2c lcd
#include <Servo.h>             // servo 
#include <RotaryEncoder.h>     // Rotery Encoder
#include <Bounce2.h>	         // button debounce

// INSTANTIATE A Button OBJECT
Bounce2::Button button = Bounce2::Button();

// Define LCD I2C
#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

// Define Encoder
#define ENCODER_CLK 22
#define ENCODER_DT  21
#define ENCODER_BTN 26

// state identifiers:
// #define MENU 1
// #define INPROCESS 2
// #define DONE 3

//custom LCD characters:
byte leftArrow[8] = {0, 0, 4, 12, 28, 12, 4, 0};
byte rightArrow[8] = {0, 0, 4, 6, 7, 6, 4, 0};
byte clocksy[8] = {0, 14, 21, 23, 17, 14, 0, 0};
byte sandTimer[8] = {31, 17, 10, 4, 10, 31, 31, 0};
byte teaCup[8] = {10, 5, 0, 31, 25, 9, 15, 0};


const String welcomeMessage = ("Welcome!");
const String doneMessage = ("Done!");
const int buttonPin = 26;
const int servoPin = 28;
const int buzzerPin = 6;
const int servoHighPosition = 70;
const int servoLowPosition = 150;
const int servoSpeedDelay = 30; // decrease this value to increase the servo speed

unsigned long counter;
unsigned long startTime;
long timeLeft;
volatile int state;

Servo servo;

void setup() {
  Serial.begin(115200); // Any baud rate should work
  // Serial.println("Hello Arduino\n");
  button.attach( buttonPin, INPUT );
  button.interval(60);
  button.setPressedState(LOW);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, FALLING);
  pinMode(buzzerPin, OUTPUT);
  servo.attach(servoPin);
  state = 1;
  // Init
  lcd.init();
  lcd.backlight();
  // Turn on the blacklight and print a message.
  lcd.print(welcomeMessage);
  counter = 180; // default of 180 seconds of brewing time

  lcd.createChar(0, leftArrow);
  lcd.createChar(1, rightArrow);
  lcd.createChar(2, sandTimer);
  lcd.createChar(3, teaCup);
  lcd.createChar(4, clocksy);

}

void readEncoder() {
  int dtValue = digitalRead(ENCODER_DT);
  if (dtValue == HIGH) {
    counter = counter + 15; // Clockwise add 15 seconds to the time max of 10 minutes
    if (counter >= 600)
      counter = 600;
  }
  if (dtValue == LOW) {
    counter = counter - 15; // Counterclockwise remove 15 seconds fromt he time minimum of 15 seconds
    if (counter < 15 )
      counter = 15;
  }

}

//do I need this?
//int getCounter() {
//  int result;
//  noInterrupts();
//  result = counter;
//  interrupts();
//  return result;
//}

void loop() {

  switch (state) {

    case 1:   { //menu
        moveServoTo(servoHighPosition);
        lcd.clear();

        lcd.setCursor(11, 1);
        lcd.print("start");

        lcd.setCursor(8, 1);
        lcd.write(0);  //display selection arrows
        lcd.write(1);

        lcd.setCursor(1, 0);
        lcd.write(4);  //display clock symbol

        while (state == 1) {
          lcd.setCursor(3, 0);
          lcd.print(secondstotime(counter));
          delay(200);
          button.update();
          if ( button.pressed() ) {
            state = 2;
            break;
          }
        }
      }

      break;

    case 2:  moveServoTo(servoLowPosition); { //brewing
        startTime = millis();

        lcd.clear();

        lcd.setCursor(12, 1);
        lcd.print("stop");

        lcd.setCursor(1, 1);
        lcd.write(2);  //display sand timer symbol

        while (state == 2) {

          timeLeft = counter - (millis() / 1000 - startTime / 1000);
          if (timeLeft > 0) {

            lcd.setCursor(3, 1);
            lcd.print(secondstotime(timeLeft));
            button.update();
            if ( button.pressed() ) {
              state = 1;
              break;
            }
          }
          else state = 3;

          delay(500);
        }
      }
      break;

    case 3:       lcd.clear(); { //Tea is done
        lcd.setCursor(1, 0);
        lcd.print(doneMessage);
        lcd.setCursor(1, 1);
        lcd.print("Enjoy! ");
        lcd.write(3);  //display tea cup symbol

        moveServoTo(servoHighPosition);

        // doneBeep();

        lcd.setCursor(12, 1);
        lcd.print("menu");

        while (state == 3) {
          button.update();
          if ( button.pressed() ) {
            state = 1;
            delay(3);
            break;
          }
        }
      }
      break;
  }
}

void moveServoTo(int finalPosition) { //move the servo slowly to the desired position

  int currentPosition = servo.read();
  if (finalPosition > currentPosition) {
    servo.write(finalPosition);
    delay(servoSpeedDelay);

  }
  else if (finalPosition < currentPosition) {

    servo.write(finalPosition);
    delay(servoSpeedDelay);
  }
}
String secondstotime(long secondsin) { //convert seconds into minutes

  unsigned long minutes = (secondsin / 60);
  unsigned long seconds = secondsin % 60;
  String minutesString = String(minutes);
  String secondsString = String(seconds);

  if (minutes < 10) minutesString = "0" + minutesString;

  if (seconds < 10) secondsString = "0" + secondsString;

  return minutesString + ":" + secondsString;
}

void doneBeep() {
  tone(buzzerPin, 4000, 700);
}
