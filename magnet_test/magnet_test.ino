int magnet = 13;
int button = 3;

void setup () {
	
	pinMode(magnet, OUTPUT);
	pinMode(button, INPUT_PULLUP);
}

void loop(){

  int val = digitalRead(button);
	if (val == HIGH)
		digitalWrite(magnet, LOW);
	else
		digitalWrite(magnet, HIGH);
}
