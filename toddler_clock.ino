#include <Wire.h>
#include "RTClib.h"

#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "ssid"        // cannot be longer than 32 characters!
#define WLAN_PASS       "password"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

LiquidCrystal lcd(0);
RTC_DS1307 rtc;

void setup(void) {
  Serial.begin(115200);
  lcd.begin(16, 2);
  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println(F("Hello, CC3000!\n"));

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    status(F("WiFi Error"));
    while(1);
  }
}

void loop(void) {
  connectToWifi();
  checkDHCP();

  displayCurrentTime();

  delay(1000);
}

void connectToWifi() {
  if (!cc3000.checkConnected()) {
    Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
      status(F("WiFi conn failed"));
    }

    Serial.println(F("\nConnected!"));
  }
}

void disconnectWifi() {
  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time your try to connect ... */
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
}

void checkDHCP() {
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  int maxWait = 20000;
  int timeout = 0;
  while (!cc3000.checkDHCP() && timeout < maxWait) {
    Serial.println(F("DHCP Request Failed"));
    status(F("DHCP Failed"));
    timeout += 500;
    delay(500);
  }

  if (timeout < maxWait) {
    /* Display the IP address DNS, Gateway, etc. */
    timeout = 0;
    while (!displayConnectionDetails() && timeout < maxWait) {
      Serial.println(F("No connection details"));
      status(F("No conn details"));
      timeout += 1000;
      delay(1000);
    }
  }
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  } else {
    status(F(" "));
    lcd.setCursor(0, 0);
    lcd.print((uint8_t)(ipAddress >> 24));
    lcd.print(F("."));
    lcd.print((uint8_t)(ipAddress >> 16));
    lcd.print(F("."));
    lcd.print((uint8_t)(ipAddress >> 8));
    lcd.print(F("."));
    lcd.print((uint8_t)ipAddress);
    /* Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress); */
    /* Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask); */
    /* Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway); */
    /* Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv); */
    /* Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv); */
    /* Serial.println(); */
    return true;
  }
}

void status(const char* msg) {
  lcd.setCursor(0, 0);
  lcd.print(msg);
  int printed = strlen(msg);
  while (printed++ <= 16) {
    lcd.print(F(" "));
  }
}

void status(const __FlashStringHelper* message) {
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void displayCurrentTime() {
  lcd.setCursor(0, 1);
  DateTime now = rtc.now();
  int hour = now.hour();
  if (hour > 13) {
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
