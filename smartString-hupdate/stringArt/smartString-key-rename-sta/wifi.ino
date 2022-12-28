void wifiCfg(){
  bMode = 1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(">WifiManager");
  lcd.setCursor(0,1);
  lcd.print("StandAlone");
}

void wifiConfigure(){
  Serial.print("wToogle : ");
  Serial.println(wToggle);
  switch(wToggle){
    case 0:
    resetWiFi();
    break;
    case 1:
    wStandAloneCfg();
    reboot();
    break;
  }
}

void wStandAloneCfg(){
  File file = SPIFFS.open("/wmode.txt","w");
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }
 
  int bytesWritten = file.print("1");
 
  if (bytesWritten > 0) {
    Serial.println("/wmode.txt was written");
    Serial.println(bytesWritten);
 
  } else {
    Serial.println("/wmode.txt write failed");
  }
  file.close();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("wifi mode 1");
  lcd.setCursor(0,1);
  lcd.print("saved & reboot");
  Serial.println("Standalone mode configured & reboot");
  delay(1000);
}

void wifiToggle(){
  wToggle++;
  if(wToggle == 2)wToggle=0;
  String s0;
  String s1;
  switch(wToggle){
    case 0:
      s0 = ">WiFiManager";
      s1 = "Standalone";
      break;
    case 1:
      s0 = "WiFiManager";
      s1 = ">Standalone";
      break;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(s0);
  lcd.setCursor(0,1);
  lcd.print(s1);
}

void resetWiFi(){
  Serial.println("[resetWiFi]");
  delay(1000);
  SPIFFS.remove("/wmode.txt");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reset WiFi");
  delay(1000);
  wm.resetSettings();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("  Reboot...");
  for(int i=0;i<16; i++){
    lcd.setCursor(0,1);
    lcd.print("*"); 
    delay(100);
  }
  ESP.restart();
}

bool wMode(){
  String s;
  File f = SPIFFS.open("/wmode.txt","r");
  if(!f)return true;
  while(f.available()){
    char c = char(f.read());
    s+= c;
  }
  f.close();
  if(s.equals("1"))return false;
  return true;
}

void apStart(){
    IPAddress local_IP(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
    
    Serial.print("Setting AP (Access Point)…");
    Serial.println(WiFi.softAP("StringArt") ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
}
