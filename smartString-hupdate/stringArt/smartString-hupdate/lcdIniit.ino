void lcdInit(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("StringArt");
  delay(2000);
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

void lcdWifiPrint(){
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SSID:");
  lcd.print(WiFi.SSID());
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
}
