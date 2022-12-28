/*
 * rename sta done
 * todo :
 * standalone read from wmode.txt
 * if wmode.txt = 1 standalone
 * configuration wmode via pb
 * upload file via button
 * 
 */

#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <FS.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

#define PIN D4
#define NUMPIXELS 300

OneButton bUp = OneButton(D5,false);
OneButton bDown = OneButton(D7,false);
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x27,16,2);
WiFiManager wm;
ESP8266WebServer server(80);



//variable
int spState=0; // 0 webserver, 1 wifimanager
File fsUploadFile;
String afname;
int fdi;
int dataCount=0;
const int nRow=4000;
int data[nRow];
int fcount=0;
const int maxFile=32;
int cPos=-1;
bool mFlag=false;
int bMode=0;
int wToggle=0;

struct FTstruct{
  String name;
  int nRow;
  int state;
};

FTstruct fTable[maxFile];

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  SPIFFS.begin();
  Serial.begin(115200);
  keystart();
  lcdInit();
  //WiFi.setOutputPower(5);
  // if mode standalone skip wm.autoconnect and startwebserver  
  // file config
  if(wMode()) {
    //start wifi manager
    wm.autoConnect("ApaITU-StringArt"); //blocking
  }else{
    // standalone mode
    apStart();
  }
  
  lcdWifiPrint();
  lcdLastState(lastState());
  initWebServer();
  initButton();

}


ICACHE_RAM_ATTR void checkTicks() {
  bUp.tick();
  bDown.tick();
}

void reboot(){
  Serial.println("Reboot...");
  for(int i=0;i<20; i++){
    Serial.print("*"); 
    delay(10);
  }
  ESP.restart();
}



void startPortal(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Stop webserver");
  server.stop();
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Start wmPortal");
  wm.startWebPortal();
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  spState =1;
}

void loop() {
  if(spState == 0){
    server.handleClient();
  } else {
    wm.process();
  }
  bUp.tick();
  bDown.tick();
}
