// Anthony Parra @ Parra Industries
// 02-01-2019
// Fingerprint Scanner Door Lock With LCD


#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>
#include <NewTone.h>

// pin #1 and #2 is for red and green led
// pin #2-5, and #11-13 is from LCD
// pin #6 is the programming button
// pin #8 is IN from sensor (GREEN wire)
// pin #9 is OUT from arduino  (WHITE wire)
// pin #10 is from solenoid
// pin #13 is for buzzer

int greenLed = 14;
int redLed =15;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int buttonPin = 6;
int buttonState = 0;

uint8_t id;
SoftwareSerial mySerial(8, 9);

int tipPin = 10;

int buzzerPin = 13;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  pinMode(buttonPin, INPUT);
  
  lcd.begin(16, 2);
  lcd.print("Place finger");
  lcd.setCursor(0, 1);
  lcd.print("please...");

  pinMode(buzzerPin, OUTPUT);
  pinMode(tipPin, OUTPUT);
  
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop()                     
{
  buttonState = digitalRead(buttonPin);

  int i = 0;
  
  if (buttonState == HIGH) {
    i = i + 1;
  }

  switch (i){
    case 1:
     Serial.println("Ready to enroll a fingerprint!");
     Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
     id = readnumber();
     if (id == 0) {// ID #0 not allowed, try again!
        return;
     }
     Serial.print("Enrolling ID #");
     Serial.println(id);
      
     while (!  getFingerprintEnroll() );
     i = i - 1;
      break;

    case 0:
      getFingerprintID();
      break;
    default:
      getFingerprintID();
      break;
      
  }
  
}

//checking fingerprint function

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, LOW);
      //code for lcd
      delay(3000);
      lcd.clear();
      lcd.begin(16, 2);
      lcd.print("Place finger");
      lcd.setCursor(0, 1);
      lcd.print("please...");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      return p;
    default:
      Serial.println("Unknown error");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      //code for lcd
      delay(3000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Try again please");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      //code for lcd
      delay(3000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Try again please");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      //code for lcd
      delay(3000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Try again please");
      return p;
    default:
      Serial.println("Unknown error");
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    //code for lcd
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("How bout' no.");
    NewTone(buzzerPin, 1000);
    delay(200);
    noNewTone(buzzerPin);
    delay(200);
    NewTone(buzzerPin, 1000);
    delay(500);
    noNewTone(buzzerPin);
    return p;
  } else {
    Serial.println("Unknown error");
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);

  //code for lcd
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome!");

  //code for solenoid
  analogWrite(tipPin, 255); 
  Serial.print("Lock Opened");
  NewTone(buzzerPin, 1000);
  delay(5000);
  noNewTone(buzzerPin);
  lcd.clear();
  lcd.begin(16, 2);
  lcd.print("Place finger");
  lcd.setCursor(0, 1);
  lcd.print("please...");
  analogWrite(tipPin, 0);
  
  return finger.fingerID;
}

//enrolling fingerprint function

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
} 

