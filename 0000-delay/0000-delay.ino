

void setup() {
  Serial.begin(115200);
  Serial.println();
  
}

void loop() {
  static int state=0;
  state = !state;
  Serial.println(state?"ON":"OFF");
  delay(1000);
}
