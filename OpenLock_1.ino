void Open(){
  pinMode(8,OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(8,LOW);
  digitalWrite(7,HIGH);
  delay(600);
  digitalWrite(8,LOW);
  digitalWrite(7,LOW);
}
void Close(){
  pinMode(8,OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(8,HIGH);
  digitalWrite(7,LOW);
  delay(600);
  digitalWrite(8,LOW);
  digitalWrite(7,LOW);
}
