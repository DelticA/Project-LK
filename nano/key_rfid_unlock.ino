//* is to validate password
//# is to reset password attempt

/////////////////////////////////////////////////////////////////

#include <Password.h>
#include <Keypad.h>
#include<SPI.h>
#include<MFRC522.h>
#define D1 4
#define out 1

MFRC522 rfid(10, 9);
byte nuidPICC[4];
int a = 0;

Password password = Password( "1234" );

int countdown = 5;

const byte ROWS = 4; // Four rows
const byte COLS = 3; //  columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = { 8, 7, 6, 5, }; // Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte colPins[COLS] = { 4, 3, 2 }; // Connect keypad COL0, COL1 and COL2 to these Arduino pins.

bool rfid_open() {
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
    //Serial.println(nuidPICC[i]);
  }

  if ((nuidPICC[0] == 50 && nuidPICC[1] == 199 && nuidPICC[2] == 153 && nuidPICC[3] == 27))
  {
    //Serial.println("Success");
    digitalWrite(1, HIGH);
    delay(500);
    digitalWrite(1, LOW);
    return true;
  }
  else {
    //Serial.println("Wrong");
    return false;
  }
}

// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
    //Serial.begin(9600);
    
  pinMode(out, OUTPUT);
  //Serial.println("all setup.");
  digitalWrite(out, LOW);
  SPI.begin();
  rfid.PCD_Init();


  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
}

void loop() {
  keypad.getKey();
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) { //找卡并验证可读
    rfid_open();
    delay(1000);
  }
}

//take care of some special events
void keypadEvent(KeypadEvent eKey) {
  switch (keypad.getState()) {
    case PRESSED:
      //Serial.print("Pressed: ");
      //Serial.println(eKey);
      switch (eKey) {
        case '*': checkPassword(); break;
        case '#': password.reset(); break;
        default: password.append(eKey);
      }
  }
}

void checkPassword() {
  if (password.evaluate() && countdown >= 0) {
    //Serial.println("Success");
    digitalWrite(out, HIGH);
    delay(500);
    digitalWrite(out, LOW);
    //Add code to run if it works
    password.reset();
  } else {
    //Serial.println("Wrong");
    //add code to run if it did not work
    password.reset();
    countdown = countdown - 1;
  }
}
