//Pot Variables
int speed_setting = 0;
int val = 0;
int setting = 0;

//Switch Variables
int ledPin = 7; // LED connected to digital pin 13
int inPin = 4;   // pushbutton connected to digital pin 7
int val1 = 0;     // variable to store the read value

//Totem Control Pins
int R = 10;
int L = 9;


void setup()  { 
  pinMode(R, OUTPUT);
  pinMode(L, OUTPUT);
  pinMode(ledPin, OUTPUT);      // sets the digital pin 13 as output
  pinMode(inPin, INPUT);      // sets the digital pin 7 as input
  Serial.begin(9600);
} 

void loop()  {   
  val = analogRead(speed_setting); 
  setting = map(val, 0, 1023, 0, 254);
  
  val1 = digitalRead(inPin);   // read the input pin
  digitalWrite(ledPin, val1);    // sets the LED to the button's value
      
  if (val1 == HIGH) {
      forward(setting);
      Serial.println("FORWARD");
  }
  else {
      backward(setting);
      Serial.println("BACKWARD");		
  }

}


void forward(int setting) {
  digitalWrite(R, LOW);
  //Serial.println(setting);
  analogWrite(L, setting);
}

void backward(int setting) {
  digitalWrite(L, LOW);
  val = analogRead(speed_setting); 
  setting = map(val, 0, 1023, 0, 254);
  //Serial.println(setting);
  analogWrite(R, setting);
}
