




void setup() {

  Serial.begin(115200);
  pinMode(32, INPUT); 
  pinMode(33, INPUT);

}

void loop() {
  
  if((digitalRead(32) == 1)||(digitalRead(33) == 1)){
    Serial.println('!');
  }
  else{
    
      Serial.println(analogRead(34));
  }
  
  delay(10);
}
