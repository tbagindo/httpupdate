void initButton(){
  bUp.attachClick(upPos);
  bUp.setPressTicks(2000);
  //bUp.attachLongPressStart(reboot);
  bDown.attachClick(downPos);
  //bDown.attachLongPressStart(resetWiFi);
  bDown.setPressTicks(6000);
  attachInterrupt(digitalPinToInterrupt(D5), checkTicks, CHANGE);
  attachInterrupt(digitalPinToInterrupt(D7), checkTicks, CHANGE);
}
void upSclick(){
  if(bMode==0)upPos();
  if(bMode==1)wifiToggle();
}
void downSclick(){
  if(bMode==0)downPos();
  if(bMode==1)wifiConfigure();
}

void upPos(){
  if(mFlag){
    cPos++;
    if(cPos>dataCount){
      Serial.println("cPos HIT Maximum!!! " + String(cPos));
      cPos=dataCount;
    }
    lcdPos();
    pShow(cPos);
    logger();
    Serial.println("cPos : " +String(cPos));
  } else {
    Serial.println("load file first from web browser");
  }
}

void downPos(){
  if(mFlag){
    cPos--;
    if(cPos<0){
      Serial.println("cPos HIT minimum!!! " + String(cPos));
      cPos=0;
    }
    lcdPos();
    pShow(cPos);
    logger();
    Serial.println("cPos : " +String(cPos));
  } else {
    Serial.println("load file first from web browser");
  }
}

void pShow(int pos){
  pixels.clear();
  pixels.setPixelColor(data[pos],pixels.Color(150,0,0));
  //pixels.setPixelColor(data[pos+1],pixels.Color(0,150,0));
  pixels.show();
  
}
