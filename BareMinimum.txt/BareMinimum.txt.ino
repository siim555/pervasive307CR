float tempC = 5;
float lightL = 1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  tempC +=1;
  Serial.print(tempC);
  Serial.print(" , ");
  Serial.println(lightL);
  delay(250);

}
