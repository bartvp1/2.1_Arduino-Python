int greenLedPin = 2;
int redLedPin = 4;

int lichtwaarde;

void setup() {
  Serial.begin (9600);
  pinMode(A1, INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin,OUTPUT);
}

void loop() {
  lichtwaarde = analogRead(A0);
  Serial.print("lichtwaarde = ");
  Serial.println(lichtwaarde);

  if (lichtwaarde < 100)        //Moeten nog even kijken op hoeveel hij moet
  {
    digitalWrite(redLedPin, HIGH);
  }
  else{
    digitalWrite(greenLedPin, HIGH);
  }
  delay(20);       //want moet om de 30 seconden gemeten worden
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);
}
