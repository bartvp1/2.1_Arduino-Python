#include <TM1638.h>

const int LED = 2;
const int LED2 = 3;
const int LED3 = 4;
const int LED4 = 5;
const int trig = 6;
const int echo = 7;
const int diopin = 8;
const int stbpin = 9;
const int clkpin = 10;
const int lichtSensor = A0;

TM1638 module(diopin, clkpin, stbpin);

int basisLichtwaarde = 60;
int ingerold = 0;
int uitgerold = 0;
int afstand = 30;
int tijdLichtwaarde = 0;
int gemLichtwaarde = 0;
int inuitrollen = 0;
int autonoom = 0;
int loopteller = 0;
int moduleSet = 0;
int lichtwaarde = 0;
int i = 0;

int buttons = 0;
int startValue = 0;
int buttonPressed = 1;
int dataOnScreen = 0;
int buttonChanged = 0;
int isError = 0;
int incOrDec = 0;
int incOrDecPressed = 0;
int autonoomPressed = 0;
int maxRoll = 50;
String gemLichtwaardeString = "";
String distance = "";
String basis = "";

void setup() {

  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(LED, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}


void loop() {
  command_naar_centrale("baseLichtwaarde", basisLichtwaarde);
  command_naar_centrale("rollStatus", ingerold ? 1 : (uitgerold ? 2 : 0));
  command_naar_centrale("maxRoll", maxRoll);

  loopteller++; 

  if (autonoom == 0) {
    digitalWrite(LED4, HIGH);
  }
  else if (autonoom == 1) {
    digitalWrite(LED4, LOW);
  }

  if (module.getButtons() > 0 && module.getButtons() != buttonPressed) {
    module.clearDisplay();
    if (module.getButtons() == 64 && autonoom == 0) {
      basisLichtwaarde--;
      buttonPressed = 4;
    }
    else if (module.getButtons() == 128 && autonoom == 0) {
      basisLichtwaarde++;
      buttonPressed = 4;
    }
    else {
      buttonPressed = module.getButtons();
    }
  }
  if (buttonPressed == 1) {
    gemLichtwaardeString = String(gemLichtwaarde);
    module.clearDisplay();
    module.setDisplayToString("GEMI " + gemLichtwaardeString);
  }
  
  if (buttonPressed == 2) {
    if (isError == 1) {
      module.clearDisplay();
      module.setDisplayToString("ERROR");
    }
    else if (isError == 0) {
      distance = String(afstand);
      module.clearDisplay();
      module.setDisplayToString("AFST " + distance);
    }
  }
  if (buttonPressed == 4) {
    basis = String(basisLichtwaarde);
    module.clearDisplay();
    module.setDisplayToString("BASI " + basis);
  }
  if (buttonPressed == 8) {
    if (ingerold == 1) {
      module.clearDisplay();
      module.setDisplayToString("INGEROLD");
    }
    else if (uitgerold == 1) {
      module.clearDisplay();
      module.setDisplayToString("UITGEROLD");
    }
    else if (inuitrollen == 1) {
      module.clearDisplay();
      module.setDisplayToString("BEZIG");
    }
    else {
      module.clearDisplay();
      module.setDisplayToString("METEN");
    }
  }

  if (loopteller % 10 == 1) { //elke 1s
    berekenAfstand();
  }

  if (autonoom == 0) {
    int sensorInput = analogRead(lichtSensor);
    lichtwaarde = sensorInput / 5;

    i++;
    tijdLichtwaarde += lichtwaarde;
    if (i == 100) {
      gemLichtwaarde = (tijdLichtwaarde / 100);
      tijdLichtwaarde = 0;
      i = 0;
      command_naar_centrale("lichtwaarde", gemLichtwaarde);
    }
    // licht hoog, uitrollen
    if (gemLichtwaarde > basisLichtwaarde && uitgerold == 0 && gemLichtwaarde != 0) {
      uitrollen();
    }

    // licht laag, inrollen
    if (gemLichtwaarde <= basisLichtwaarde && ingerold == 0 && gemLichtwaarde != 0) {
      inrollen();
    }
  }

  if (ingerold == 1) {
    digitalWrite(LED, HIGH);
    digitalWrite(LED3, LOW);
  }
  else if (uitgerold == 1) {
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
  }

  if (inuitrollen == 1) {
    delay (50);
  }
  else {
    delay(100);
  }
}

void berekenAfstand() {
  digitalWrite(trig, LOW);      
  digitalWrite(trig, HIGH);    
  delayMicroseconds( 10 );
  digitalWrite(trig, LOW); 

  int looptijd = pulseIn(echo, HIGH); 

  afstand = (looptijd / 2) * 0.0343; 
  if (afstand < 0) {
    afstand = 0;
  }
  if (afstand == 0) {
    isError = 1;
  }
  if (afstand > 0) {
    isError = 0;
  }
}

void inrollen() {
  uitgerold = 0;
  inuitrollen = 1;
  digitalWrite(LED2, LOW);
  digitalWrite(LED, HIGH);
  digitalWrite(LED3, HIGH);
  delay(50);
  digitalWrite(LED3, LOW);
  if (afstand > maxRoll && afstand != 0) {
    ingerold = 1;
    inuitrollen = 0;
  }
}

void uitrollen() {
  inuitrollen = 1;
  ingerold = 0;
  digitalWrite(LED, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  delay(50);
  digitalWrite(LED3, LOW);
  if (afstand < 5 && afstand != 0) {
    uitgerold = 1;
    inuitrollen = 0;
  }
}

void command_naar_centrale(String cmd, int data) {
  String commandString = cmd + " " + data;

  Serial.write(commandString.length());
  Serial.print(commandString);
}

void binnenCommand() {
  String cmd = "";
  bool cmdDone = false;
  String dataString = "";

  while (Serial.available()) {
    char inChar = (char)Serial.read();

    if ((String) inChar == " ") {
      cmdDone = true;
      continue;
    }
    if (!cmdDone) {
      cmd += inChar;
    } else {
      dataString += inChar;
    }
  }

  processCommand(cmd, dataString.toInt());
}

void processCommand(String cmd, int data) {
  if (cmd == "incBase") {
    basisLichtwaarde++;
  }
  if (cmd == "decBase") {
    basisLichtwaarde--;
  }
  if (cmd == "incMaxRoll") {
    maxRoll++;
  }
  if (cmd == "decMaxRoll") {
    maxRoll--;
  }
  if (cmd == "rollIn") {
    if (uitgerold == 1) {
      inrollen();
    }
  }
  if (cmd == "rollOut") {
    if (ingerold == 1) {
      uitrollen();
    }
  }
  if (cmd == "autonoom") {
    if (inuitrollen == 0) {
      autonoom = data;
      command_naar_centrale("autonoomPressed", autonoom);
    }
  }
}
