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
const int temperatuurSensor = A0;

TM1638 module(diopin, clkpin, stbpin);

int basisTemperatuur = 22;
int ingerold = 0;
int uitgerold = 0;
int afstand = 30;
int inuitrollen = 0;
int moduleSet = 0;
int temperatuur = 0;
int i = 0;
int temp_sum = 0;
int buttons = 0;
int startValue = 0;
int buttonPressed = 1;
int dataOnScreen = 0;
int buttonChanged = 0;
int isError = 0;
int incOrDec = 0;
int incOrDecPressed = 0;
int maxRoll = 50;
int temp_gem;

void setup() {
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

  Serial.begin(9600);
  Serial.flush();
  Serial.print("temperatuursensor\n");
}

void loop() {
  i++; 

  int sensorInput = analogRead(temperatuurSensor);
  float voltage = sensorInput * (5000/1024);
  temperatuur= (voltage - 500) / 10.0;
  temp_sum += temperatuur;
  
  if (i%10==9) { // elke 10 ticks
    temp_gem = (temp_sum / 10);
    afstand = berekenAfstand();
    temp_sum = 0;
    
    String temp_gem_s = "temp="+(String)temp_gem+"\n";
    for (int x = 0; x < temp_gem_s.length(); x++){
      Serial.print(temp_gem_s[x]);   // Push each char 1 by 1 on each loop pass
    }
  }


  if(temp_gem != 0){
    if (temp_gem > basisTemperatuur && uitgerold == 0) { // temperatuur hoog, uitrollen
      uitrollen();
    }
    
    if (temp_gem <= basisTemperatuur && ingerold == 0) { // temperatuur laag, inrollen
      inrollen();
    }
  }

  if (ingerold == 1) { // groen aan
    digitalWrite(LED, HIGH);
    digitalWrite(LED3, LOW);
  }
  else if (uitgerold == 1) { // rood aan
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
  }
  binnenCommand();
  delay(200);
}


int berekenAfstand() {
  digitalWrite(trig, LOW);      
  digitalWrite(trig, HIGH);    
  delayMicroseconds( 10 );
  digitalWrite(trig, LOW); 
  
  int looptijd = pulseIn(echo, HIGH); 
  afstand = (looptijd / 2) * 0.0343; 
  return afstand;
}

void inrollen() {
  uitgerold = 0;
  inuitrollen = 1;
  digitalWrite(LED2, LOW);
  digitalWrite(LED, HIGH);
  digitalWrite(LED3, HIGH);
  delay(200);
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
  delay(200);
  digitalWrite(LED3, LOW);
  if (afstand < 5 && afstand != 0) {
    uitgerold = 1;
    inuitrollen = 0;
  }
}


void binnenCommand() {
  String string = "";

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    string += inChar;
  }
  //for testing purposes
  for (int x = 0; x < string.length(); x++){
    //Serial.print(string[x]);   // Push each char 1 by 1 on each loop pass
  }
}
