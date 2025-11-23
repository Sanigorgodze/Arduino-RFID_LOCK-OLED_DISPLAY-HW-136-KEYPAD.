#include<SPI.h>
#include<MFRC522.h>
#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define SS_PIN 10
#define RST_PIN 9
#define LED_PIN 13

int Relay = 4;
int Red_led = 2;
int Green_led =3;

int sclPin = 7;
int sdoPin =8;

byte Key;
int input[4];
int password[4] = {1,2,3,4};
int index = 0;


MFRC522 rfid(SS_PIN, RST_PIN);
byte authUID[] = {0x29, 0xCF, 0x18, 0x06};


void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  
  // keypad
  pinMode(sclPin, OUTPUT);
  pinMode(sdoPin, INPUT);
  digitalWrite(sclPin, HIGH);

  // relay, 2 leds
  pinMode(Red_led, OUTPUT);
  pinMode(Green_led, OUTPUT);
  pinMode(Relay, OUTPUT);
  digitalWrite(Relay,HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  Serial.println("Scan RFID tag...");
  menu();

}

void loop() {
  ReadTag();
  ReadKeyPad();
  
}



void ReadTag() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  bool auth = true;

  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Check UID length must be exactly 4
  if (rfid.uid.size != 4) {
    auth = false;
  } else {
    // Compare each byte
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != authUID[i]) {
        auth = false;
        break;
      }
    }
  }

  Serial.println(auth ? "Access Granted" : "Access Denied");

  if (auth) {
    digitalWrite(Green_led, HIGH);
    digitalWrite(Relay, LOW);
    displayGranted();
  } else {
    displayDenied();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(2000);
  digitalWrite(Green_led, LOW);
  digitalWrite(Relay, HIGH);
  menu();
}



void ReadKeyPad(){
  
  Key = Read_Key();

  if (Key){
    delay(50);
    byte confirmKey = Read_Key();
    if (confirmKey != Key) return;
  }

  if (Key){
    Serial.print("pressed: ");
    Serial.print(Key);

    input[index]=Key;
    index++;
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);

    int X_shift = 0;
    for (int i = 0; i < index; i++){
      display.setCursor(X_shift, 32);
      display.print(input[i]);
      X_shift += 32;
    }

    display.display();
    

    if (index == 4){
      if (checkPassword()){
        Serial.println("acsess granted");
        digitalWrite(Green_led,HIGH);
        digitalWrite(Relay, LOW);
        displayGranted();

        delay(2000);

        digitalWrite(Green_led,LOW);
        digitalWrite(Relay,HIGH);
        menu();

        

      }else{
        Serial.print("acsess denied");
        displayDenied();
        delay(2000);
        menu();

      }
      index = 0;
    }
    delay(500);
    //digitalWrite(Green_led,LOW);
    //digitalWrite(Relay,HIGH);
    
  }
}

// Reading pushed Key

byte Read_Key(void){
  byte count;
  byte Key_State = 0;

  for (count = 1; count<= 16; count++){
    digitalWrite(sclPin, LOW);
    delayMicroseconds(5);

    if (!digitalRead(sdoPin))
      Key_State = count;
    digitalWrite(sclPin, HIGH);
    delayMicroseconds(5);
  }
  return Key_State;
}


// Checking for password

bool checkPassword(){
  for (int i = 0; i < 4; i++){
    if (input[i] != password[i]) return false;
  }
  return true;
}



void menu(){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(26,30);
  display.println("TAG OR KEYPAD ");
  display.display();

}

void displayDenied(){
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(25, 30);
    display.print("Access Denied!");
    display.display();
}

void displayGranted(){
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(20,30);
        display.print("Accsess Granted");
        display.display();

}