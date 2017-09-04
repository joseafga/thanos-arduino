#include <SoftwareSerial.h>

bool debug = true;

// pins
const byte ledPin = 13;
const byte buzzerPin = 7;
const byte btTx = 2;
const byte btRx = 3;

// bluetooth TX to the Arduino RX 
// bluetooth RX to the Arduino TX
SoftwareSerial SerialBT(btTx, btRx); // RX | TX

bool ledStatus = false; // led
bool buzzerStatus = false; // buzzer

const byte numChars = 10; // max length of command
char receivedCmd[numChars]; // an array to store the received data 
boolean newCmd = false;

void setup() {  
  Serial.begin(9600);  
  // The default baud rate for the HC-06s I have is 9600
  SerialBT.begin(9600);

  pinMode(ledPin, OUTPUT); // LED
  pinMode(buzzerPin, OUTPUT); // Buzzer

  Serial.println("<Arduino is ready>");
}

void loop() {
  if (SerialBT.available() > 0 ){
    recvWithStartEndMarkers(SerialBT.read());
  }

  if (debug) {
    if (Serial.available() > 0 ){
      recvWithStartEndMarkers(Serial.read());
    }
  }
  
  if (newCmd) {
    command(receivedCmd);
    newCmd = false;
  }
}     


void command(char *cmd) {
  if (debug) {
    Serial.println(String("received: ") + receivedCmd);
    SerialBT.println(String("sended: ") + receivedCmd);
  }

  switch (cmd[0]) {
    case 'L':
      digitalToggle(ledPin, ledStatus);
      
      ledStatus = !ledStatus;
      break;
    case 'B':
      digitalToggle(buzzerPin, buzzerStatus);
  
      buzzerStatus = !buzzerStatus;
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
    break;
  }
}

// turn on/off digital pin
// if off will turn on
// if on will turn off
void digitalToggle(byte pin, bool on) {
  if (on){
    digitalWrite(pin, LOW); // turn off
  } else {
    digitalWrite(pin, HIGH); // turn on
  }
}

void recvWithStartEndMarkers(char rc) {
  static bool recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';

  if (debug) {
    Serial.println(String("read: ") + rc);
  }
  
  if (recvInProgress == true) {
    
    if (rc != endMarker) {
      receivedCmd[ndx] = rc;
      ndx++;
      
      if (ndx >= numChars) {
        ndx = numChars - 1; 
      }
      
    } else {
      receivedCmd[ndx] = '\0'; // terminate the string
      recvInProgress = false;
      ndx = 0;
      newCmd = true;
    }
  }
  
  else if (rc == startMarker) {
    recvInProgress = true;
  }
}
