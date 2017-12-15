#include <SoftwareSerial.h>

bool debug = false;

/* Based on MonsterMoto Shield Example Sketch
 * code by: Jim Lindblom
 * hardware by: Nate Bernstein
 */
#define BRAKEVCC 0
#define CW   1
#define CCW  2
#define BRAKEGND 3
#define CS_THRESHOLD 15

/*  VNH2SP30 pin definitions
 xxx[0] controls '1' outputs
 xxx[1] controls '2' outputs */
int inApin[2] = {7, 4};  // INA: Clockwise input
int inBpin[2] = {8, 9}; // INB: Counter-clockwise input
int pwmpin[2] = {5, 6}; // PWM input
int cspin[2] = {2, 3}; // CS: Current sense ANALOG input
int enpin[2] = {0, 1}; // EN: Status of switches output (Analog pin)

// pins
const byte ledPin = 13;
const byte buzzerPin = 12;
const byte powerPin = 11;
const byte btTx = 2;
const byte btRx = 3;

// bluetooth TX to the Arduino RX
// bluetooth RX to the Arduino TX
SoftwareSerial SerialBT(btTx, btRx); // RX | TX

bool ledStatus = false; // led
bool buzzerStatus = false; // buzzer
// timer
unsigned long timer;
const long timeout = 3000;
bool stopped = true;
// variables to receive parsed commands
int motor0 = 0;
int motor1 = 0;

const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;


void setup() {
  Serial.begin(9600);
  // The default baud rate for the HC-06s I have is 9600
  SerialBT.begin(9600);

  pinMode(ledPin, OUTPUT); // LED
  pinMode(buzzerPin, OUTPUT); // Buzzer
  pinMode(powerPin, OUTPUT); // To keep arduino on
  // default state
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(powerPin, HIGH);

  timer = millis(); // set initial timer

  
  Serial.println("<Arduino is ready>");
  beep(1);
}

void loop() {
  if (SerialBT.available()) {
    getDataFromPC(SerialBT.read());
  }

  if (debug && Serial.available()) {
    getDataFromPC(Serial.read());
  }

  // getDataFromPC();
  // updateMotors();
  updateTimer();
}


// receive data from PC and save it into inputBuffer
void getDataFromPC(char x) {
  if (x == endMarker) {
    timer = millis(); // reset timer
    readInProgress = false;
    newDataFromPC = true;
    inputBuffer[bytesRecvd] = 0;
    parseData();
  }
  
  if(readInProgress) {
    inputBuffer[bytesRecvd] = x;
    bytesRecvd ++;
    if (bytesRecvd == buffSize) {
      bytesRecvd = buffSize - 1;
    }
  }
  
  if (x == startMarker) { 
    bytesRecvd = 0; 
    readInProgress = true;
  }
}


// split the data into its parts
void parseData() {
  char * strtokIndx; // this is used by strtok() as an index
  char cmd[2];

  if (debug) {
    Serial.println(inputBuffer);
  }
  
  strtokIndx = strtok(inputBuffer,",");      // get the first part - the string
  strcpy(cmd, strtokIndx); // copy it to messageFromPC

  switch(cmd[0]) {
    case 'M':
      // motors case
      strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
      motor0 = atoi(strtokIndx);     // convert this part to an integer
      strtokIndx = strtok(NULL, ","); 
      motor1 = atoi(strtokIndx);

      updateMotors();
      break;
    case 'X':
      // button X case
      strtokIndx = strtok(NULL, ",");
      updatePin(buzzerPin, atoi(strtokIndx));
      break;
    case 'C':
      // button center case
      motorOff(0);
      motorOff(1);
      beep(5);
      digitalWrite(powerPin, LOW);
      break;
    default:
      // optional
      break;
  }
}


// check timer status
void updateTimer() {
  if ((millis() - timer) > timeout) {
    if(!stopped) {
      if (debug) {
        Serial.println(String("Timeout stopping"));
      }
      
      stopped = true;
  
      motorOff(0);
      motorOff(1);
      beep(3);
    }
  } else {
    stopped = false;
  }
}


// execute buzzer beeps x times
void beep(int times) {
  for(int i = 0; i < times; ++i) {
    digitalWrite(buzzerPin, HIGH);
    delay(50);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
}


// pin action
void updatePin(byte pin, int status) {
  if(status == 1) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}


// make motors work
void updateMotors() {
  // set pwm and direction for motor 0
  if(motor0 > 0) {
    motorGo(0, CW, motor0);
  } else if (motor0 < 0) {
    motorGo(0, CCW, abs(motor0));
  } else {
    motorOff(0);
  }

  // set pwm and direction for motor 1
  if(motor1 > 0) {
    motorGo(1, CW, motor1);
  } else if (motor1 < 0) {
    motorGo(1, CCW, abs(motor1));
  } else {
    motorOff(1);
  }

  //if ((analogRead(cspin[0]) > CS_THRESHOLD) && (analogRead(cspin[1]) > CS_THRESHOLD)) {
  //    SerialBT.println(String("HC:")
  //      + analogRead(cspin[0]) 
  //      + String(",") 
  //      + analogRead(cspin[1])
  //    );

  //    if (debug) {
  //      Serial.println(String("HC:")
  //        + analogRead(cspin[0]) 
  //        + String(",") 
  //        + analogRead(cspin[1])
  //      );
  //    }
  //}

  // delay(500);
}

/*
 * Turn motor off
 */
void motorOff(int motor)
{
  // Initialize braked
  digitalWrite(inApin[motor], LOW);
  digitalWrite(inBpin[motor], LOW);
  
  analogWrite(pwmpin[motor], 0);
}



/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.
 
 motor: this should be either 0 or 1, will selet which of the two
 motors to be controlled
 
 direct: Should be between 0 and 3, with the following result
 0: Brake to VCC
 1: Clockwise
 2: CounterClockwise
 3: Brake to GND
 
 pwm: should be a value between ? and 1023, higher the number, the faster
 it'll go
 */
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{
  if (motor <= 1)
  {
    if (direct <=4)
    {
      // Set inA[motor]
      if (direct <=1)
        digitalWrite(inApin[motor], HIGH);
      else
        digitalWrite(inApin[motor], LOW);

      // Set inB[motor]
      if ((direct==0)||(direct==2))
        digitalWrite(inBpin[motor], HIGH);
      else
        digitalWrite(inBpin[motor], LOW);

      analogWrite(pwmpin[motor], pwm);
    }
  }
}
