//
// This sketch pretends to be a keyboard and prints a stored password by pressing a button. Furthermore you can login/logout by registered fingerprints. 
// There are 4 commands available to administer the user and password.
// 
// Author: Ming-Che Lee <ming-che.lee@gmx.ch
//
// Licence: You may use, alternate and re-distribute it as you wish - it is complete free without any warranty. Use at own risk!
// 

#include <SPI.h>
#include <Keyboard.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Fingerprint.h>
#include "usToDE.h"

#define OLED 5
#define NUMPIXELS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, OLED, NEO_GRB + NEO_KHZ800);
int getFingerprintIDez(), fingerID = 0;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);


String entryTAG, serialString = "", serialString2 = "", newPWD = "", cmpPWD = "", replySave = "", PW, myUSER;
byte LOCK = 1, i;

byte randomValue;
char randPW[25], inChar;
byte randNumBytes = 20;
int charsRead;

const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 1023;
const int BUFSIZE = 25;
char buf[BUFSIZE];

const byte buttonPin = 4;
byte buttonState = 0; 
byte lastButtonState = 0;
int startPressed = 0;
int endPressed = 0;
int timeHold = 0;    
int timeReleased = 0;

boolean eeprom_is_addr_ok(int addr) 
{
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) 
{
  int i;
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) 
  {
    return false;
  }
  for (i = 0; i < numBytes; i++) 
  {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

boolean eeprom_write_string(int addr, const char* string) 
{
  int numBytes;
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

boolean eeprom_read_string(int addr, char* buffer, int bufSize) 
{
  byte ch;
  int bytesRead;
  if (!eeprom_is_addr_ok(addr)) 
  { 
    return false;
  }

  if (bufSize == 0) 
  { 
    return false;
  }
  
  if (bufSize == 1) 
  {
    buffer[0] = 0;
    return true;
  }
  
  bytesRead = 0;
  ch = EEPROM.read(addr + bytesRead);
  buffer[bytesRead] = ch;
  bytesRead++;
  
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) 
  {
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch;
    bytesRead++;
  }
  
  if ((ch != 0x00) && (bytesRead >= 1)) 
  {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}

char* saveStrgEEP(String myString, byte addr)
{
  char myStringChar[BUFSIZE];

  myString.toCharArray(myStringChar, BUFSIZE);
  strcpy(buf, myStringChar);
  eeprom_write_string(addr, buf); 

  eeprom_read_string(addr, buf, BUFSIZE);
  return buf;
}

String readSerialStrg ()
{
  byte waitPWD = 0;
  String tmpStrg = "";

  i = 0;
  while (waitPWD == 0)
  {
    while (Serial.available() > 0)
    {
      inChar = Serial.read();
      if ( inChar != '\r' && inChar != '\n' && i < BUFSIZE)
      {
        tmpStrg += inChar;
      }
      waitPWD = 1;
      i++;
    }
  }
  return tmpStrg;
}

boolean chgPW ()
{
  newPWD = "123456789012345618901";
  while ( newPWD.length() > 20 )
  {
    Serial.println("Pleae enter new password: ");
    newPWD = readSerialStrg();
    if ( newPWD.length() > 20 )
    {
      Serial.println("Password too long - max length are 20 chars, please retry\n");
      newPWD = "123456789012345618901";
    }
  }
    
  Serial.println("Pleae enter new password again: ");
  cmpPWD = readSerialStrg();

  if (newPWD == cmpPWD)
  {
    PW = "";
    PW = saveStrgEEP(newPWD,0);
    Serial.print("New password ");
    Serial.print(newPWD);
    Serial.println(" is saved - please test before use\n");
    return true;
  }
  else
  {
    Serial.println("Passwords do not match - please start over\n");
    return false;
  }
}  

void prtKbdStrg(String tmpStrg)
{
  byte strgLen = tmpStrg.length();
  for (i = 0; i < strgLen; i++)
  {
    inChar = tmpStrg[i];
    unsigned char key = (usToDE[inChar]);
    Keyboard.write(key);
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  return finger.fingerID; 
}

void setup()
{
  SPI.begin();
  pixels.begin();
  Keyboard.begin();
  Serial.begin(9600);
  finger.begin(57600);

  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  
  randomSeed(analogRead(0));
    
  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
    Serial.println();
  } 
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    Serial.println();
    while (1);
  }
  
  eeprom_read_string(0, buf, BUFSIZE);
  PW = buf;
  eeprom_read_string(50, buf, BUFSIZE);
  myUSER = buf;
}

void loop()
{
  
  buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) 
  {
      if (buttonState == LOW) 
      {
          startPressed = millis();
          timeReleased = startPressed - endPressed;
      } 
      else 
      {
        endPressed = millis();
        timeHold = endPressed - startPressed;

        if (timeHold >= 50 && timeHold < 500 && LOCK == 0) 
        {
          prtKbdStrg(PW);
          Keyboard.write(KEY_RETURN);
          delay(500);
        }

        if (timeHold >= 500 && LOCK == 0) 
        {
          prtKbdStrg(myUSER);
          delay(500);

          Keyboard.press(KEY_TAB);
          Keyboard.release(KEY_TAB);
          delay(100);

          prtKbdStrg(PW);
          Keyboard.write(KEY_RETURN);
          delay(500);
        }
        digitalWrite(buttonPin, HIGH);
      }
  }
  lastButtonState = buttonState;
  
  while (Serial.available() > 0) 
  {
    // read incoming serial data:
    inChar = Serial.read();
    serialString += inChar;
  }

  if ( serialString == "passwd\r\n" && LOCK == 0)
  {
    Serial.println("Changing existing password");
    Serial.println("==========================");
    chgPW();
  }

  if ( serialString == "pwgen\r\n" && LOCK == 0)
  {
    randomValue = 0;
    memset(randPW, 0, sizeof(randPW));
    for(i = 0; i < randNumBytes; i++) 
    {
      randomValue = random(0, 35);
      if (randomValue >= 0 && randomValue <= 12)
      {
        randPW[i] = randomValue + 'a';
      }
      if (randomValue >= 13 && randomValue <= 25)
      {
        randPW[i] = randomValue + 'A';
      }
        
      if (randomValue >= 26) 
      {
        randPW[i] = (35 - randomValue) + '0';
      }
    }

    Serial.println("Generate random password");
    Serial.println("==========================");
    Serial.print("Generated password: ");
    Serial.println(randPW);
    Serial.println("Replace old password and save it? <y/n>");
    replySave = readSerialStrg();
    
    if (replySave == "y")
    {
      PW = "";
      PW = saveStrgEEP(randPW,0);
      Serial.println("Old password replaced by generated password\n");
    }
    else
    {
      Serial.println("Nothing saved - current password not replaced\n");
    }
  }

  if ( serialString == "passwin\r\n" && LOCK == 0)
  {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press(KEY_DELETE);
      delay(100);
      Keyboard.releaseAll();
      delay(500);
      Keyboard.press(KEY_DOWN_ARROW);
      Keyboard.release(KEY_DOWN_ARROW);
      delay(100);
      Keyboard.press(KEY_DOWN_ARROW);
      Keyboard.release(KEY_DOWN_ARROW);
      delay(100);
      Keyboard.press(KEY_DOWN_ARROW);
      Keyboard.release(KEY_DOWN_ARROW);
      delay(100);
      Keyboard.write(KEY_RETURN);
  }

  if ( serialString == "reset\r\n")
  {
    Serial.println("Reset user and password");
    Serial.println("===============================");
    Serial.println("Please enter password: ");
    replySave = readSerialStrg();

    if ( replySave == PW )
    {
      myUSER = "FFFFFFFFFFFFFFFFFFFF";
      Serial.println();
    }
    else
    {
      Serial.println("Wrong password - reset not started, please try again\n");
    }
  }

  if ( serialString == "help\r\n" )
  {
    Serial.println("help        List of commands");
    Serial.println("passwd      Change password");
    Serial.println("pwgen       Generate random password");
    Serial.println("passwin     Change windows password");
    Serial.println("reset       Reset user, password and tag");
    Serial.println();
    delay(100);
    //Serial.println("\nThe commands are only available in authenticated mode - except help, reset and upload!");
    //Serial.println("\nSaved passwords will be printed in English keyboard layout -");
    //Serial.println("so test the saved password by pressing the 'print-password' button!\n");
  }

  serialString = "";
  replySave = "";

  if ( myUSER == "FFFFFFFFFFFFFFFFFFFF" && PW == "FFFFFFFFFFFFFFFFFFFF" )
  {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    
    while (serialString2 != "go\r\n")
    {
      delay (1000);
      Serial.println("Please enter \"go\" to init user and passowrd");
      while (Serial.available() > 0) 
      {
        // read incoming serial data:
        inChar = Serial.read();
        serialString2 += inChar;
      }
    } 
    serialString2 = "";
    Serial.println();
  }

  if ( myUSER == "FFFFFFFFFFFFFFFFFFFF" )
  {
    delay(2000);
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    
    Serial.println("Init sequence started");
    Serial.println("=====================");
    Serial.println("Please enter new user: ");
    replySave = readSerialStrg();
    myUSER = "";
    myUSER = saveStrgEEP(replySave,50);
    Serial.print("New User ");
    Serial.print(myUSER);
    Serial.println(" is set\n");

    if ( chgPW() == true )
    {
      LOCK = 0;
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      
      Serial.println("Init seq. done - your user and passwd are set.\n");
      Serial.println("Type 'help' for admin commands\n");
      delay(1300);
    }
    else
    {
      return;
    }
  }

  fingerID = getFingerprintIDez();
  
  if ( fingerID > 0 )
  {
    if ( LOCK == 1) 
    { 
      LOCK = 0; 
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      
      // CTRL-ALT-DEL:
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press(KEY_DELETE);
      delay(100);
      Keyboard.releaseAll();
      delay(1000);
      Keyboard.print(PW);
      Keyboard.write(KEY_RETURN);
    } 
    else 
    { 
      LOCK = 1; 
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show();
      
      // CTRL-ALT-DEL:
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press(KEY_DELETE);
      delay(200);
      Keyboard.releaseAll();
      delay(1500);
      Keyboard.write(KEY_RETURN);
    }
  delay(50);
  }
}

