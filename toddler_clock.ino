#include <Wire.h>
#include <RTClib.h>
#include <Time.h>
#include <TimeAlarms.h>

#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>

#define yellowPin 8
#define greenPin 9

#define wakeUpHour 6
#define wakeUpMinute 40
#define sleepHour 9
#define sleepMinute 0

LiquidCrystal lcd(0);
RTC_DS1307 rtc;

void setup(void) {
  Serial.begin(115200);
  lcd.begin(16, 2);
  Wire.begin();
  rtc.begin();

  DateTime now = rtc.now();
  setTime(now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

  Alarm.alarmRepeat(wakeUpHour, wakeUpMinute, 0, wakeUp);
  Alarm.alarmRepeat(sleepHour, sleepMinute, 0, goToSleep);

  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  setColor(yellowPin);

  lcd.setBacklight(LOW);
}

void loop(void) {
  displayCurrentTime();

  Alarm.delay(1000);
}

void wakeUp() {
  setColor(greenPin);
}

void goToSleep() {
  setColor(yellowPin);
}

void setColor(const int onPin) {
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(onPin, HIGH);
}

void displayCurrentTime() {
  lcd.setCursor(0, 1);
  DateTime now = rtc.now();
  int hour = now.hour();
  if (hour > 12) {
    hour -= 12;
  }
  if (hour < 10) {
    lcd.print(F(" "));
  }
  lcd.print(hour, DEC);
  lcd.print(F(":"));

  printZeroPadded(now.minute());
  lcd.print(F(":"));

  printZeroPadded(now.second());
}

void printZeroPadded(const int number) {
  if (number < 10) {
    lcd.print(0);
  }
  lcd.print(number, DEC);
}
