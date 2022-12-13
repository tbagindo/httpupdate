void lcdInit(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("StringArt");
  delay(1000);
  lcd.setCursor(0,0);
  lcd.print("Starting WiFiman");
  lcd.setCursor(0,1);
  String s = (WiFi.softAPIP()).toString();
  if(s == "(IP unset)"){
    lcd.print("192.168.4.1");
  }else{
    lcd.print(WiFi.softAPIP());
  }
}

void clearRow(int c,  int r){
 lcd.setCursor(c,r);
 for(int i=0; i<16-c; i++){
  lcd.print(" ");
 }
 lcd.setCursor(c,r);
}

void lcdWifiPrint(){
  delay(300);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SSID:");
  lcd.print(WiFi.SSID());
  delay(300);
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
}

void lcdLastState(bool b){
  delay(300);
  if(b){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ready to use");
    lcd.setCursor(0,1);
    lcd.print("Press UP/Down");
  }
}

void  lcdPos(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(afname);
  lcd.setCursor(0,1);
  lcd.print("[");
  lcd.setCursor(1,1);
  lcd.print(cPos);
  lcd.print("/");
  lcd.print(dataCount);
  lcd.print("]");
  lcd.setCursor(12,1);
  lcd.print(data[cPos]);

}
