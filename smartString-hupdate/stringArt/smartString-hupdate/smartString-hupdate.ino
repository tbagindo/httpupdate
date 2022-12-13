#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <FS.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
//#include <OneButton.h>

#define repo "https://raw.githubusercontent.com/tbagindo/httpupdate/main/smartString.bin"
#define PIN D4
#define NUMPIXELS 300

//OneButton bUp = OneButton(D5,false);
//OneButton bDown = OneButton(D7,false);
//Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x27,16,2);

ESP8266WebServer server(80);


String gHost;
String gSSID;
String gPass;
ESP8266WiFiMulti WiFiMulti;

//variable
int spState=0; // 0 webserver, 1 wifimanager
File fsUploadFile;
String afname;
int fdi;
int dataCount=0;
const int nRow=3001;
int data[nRow];
int fcount=0;
const int maxFile=20;
int cPos=-1;
bool mFlag=false;
bool hFlag=false;

struct FTstruct{
  String name;
  int nRow;
  int state;
};

FTstruct fTable[maxFile];

void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  lcdInit();
  WiFiManager wm;
  wm.autoConnect("ApaITU-StringArt");
  lcdWifiPrint();
  
  
  //webserver router
  server.on("/",handleRoot);
  server.on("/upload",HTTP_POST,[](){server.send(200);},handleFileUpload);
  server.on("/upform",indexPage);
  server.on("/success",handleSuccess);
  server.on("/dataArray",handleDataArray);
  server.on("/delete",handleDelete);
  server.on("/wifiPortal", startPortal);
  server.on("/reset",[](){WiFiManager wm;wm.resetSettings();Serial.println("Reboot...");for(int i=0;i<20; i++){Serial.print("*"); delay(10);ESP.restart();}});
  server.on("/reboot", [](){Serial.println("Reboot...");for(int i=0;i<20; i++){Serial.print("*"); delay(10);ESP.restart();}});
  server.on("/update",[](){hFlag=true;SPIFFS.end();server.send(200);});
  server.onNotFound(handleNotFound);
  



//  pixels.begin();

  //bUp.attachClick(upPos);
  //bUp.setPressTicks(6000);
  //bUp.attachLongPressStart(hUpdateFlag);
  //bDown.attachClick(downPos);
  //bDown.attachLongPressStart(resetWiFi);
  //bDown.setPressTicks(6000);
  //attachInterrupt(digitalPinToInterrupt(D5), checkTicks, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(D7), checkTicks, CHANGE);
  lastState();
  
  Serial.println();
  Serial.println();
  Serial.println();
  wm.setShowPassword(true);
  gHost = wm.getWiFiHostname();
  gSSID = wm.getWiFiSSID();
  gPass = wm.getWiFiPass();
  WiFi.disconnect();
  for (uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(gSSID.c_str(), gPass.c_str());

  while((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
    delay(500);
  }
  delay(1000);
  Serial.println(WiFi.localIP());
  
  //start webserver
  server.begin();
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
}

//ICACHE_RAM_ATTR void checkTicks() {
//  bUp.tick();
//  bDown.tick();
//}

void logger(){
  if(afname == fTable[fdi].name){
    File f=SPIFFS.open("/logger.txt","w");
    String log = afname + "," + String(cPos) + "," + String(dataCount) +  "," + String(data[cPos]) +",";
    f.print(log);
    f.close();
    
  }
  Serial.println("afname : "  + afname);
  Serial.println("fTable[fdi].name : " + fTable[fdi].name);
  Serial.println("fdi : " + String(fdi));
}

void clearfTable(){
  for(int i=0; i<maxFile; i++){
    fTable[i].name="";
    fTable[i].state=0;
    fTable[i].nRow=0;
    fdi=0;
  }
}

void lastState(){
  clearfTable();
  File f= SPIFFS.open("/logger.txt","r");
  if(!f)Serial.print("lastState could not open logger.txt");
  String s;
  int y=0;
  while(f.available()){
    char c = char(f.read());
    s += c;
    //Serial.println("s : " + s);
    int iof = s.indexOf(",");
    //Serial.println("iof : " +String(iof));
    if(iof != -1){
      //Serial.println("subString " + s.substring(0,iof));
      if(y==0)afname = s.substring(0,iof);
      if(y==1)cPos = (s.substring(0,iof)).toInt();
      if(y==2)dataCount = (s.substring(0,iof)).toInt();
      s="";
      y++;
    }
  }
  f.close();
  Serial.println("====================/logger.txt=================");
  Serial.println("cPos : " + String(cPos));
  Serial.println("afname : " + afname);
  Serial.println("dataCOunt : " + String(dataCount));
  SPIFFS.remove("/logger.txt");
  logger();
  updateList();
  Serial.println("=====================updateList=================");
  Serial.println("cPos : " + String(cPos));
  Serial.println("afname : " + afname);
  Serial.println("dataCOunt : " + String(dataCount));

  if(initloadDataArray(afname)){
    Serial.println("===============initloadDataArray================");
    Serial.println("cPos : " + String(cPos));
    Serial.println("afname : " + afname);
    Serial.println("dataCOunt : " + String(dataCount));
    Serial.println("fdi = " + String(fdi));
    Serial.println("ftName : " + fTable[fdi].name);
    Serial.println("ftnRow : " + String(fTable[fdi].nRow));
    Serial.println("ftState : " + String(fTable[fdi].state));
  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("F: " + fTable[fdi].name);
    lcd.setCursor(0,1);
    lcd.print("cPos: " +  String(cPos));
    pShow(cPos);
  }else{
    Serial.println("could not open file  : " +  afname + "open web-browser");
  }
}

int getIndex(String f){
  for(int i=0; i<maxFile; i++){
    if(fTable[i].name == f) return i;
  }
  return -1;
}
int initloadDataArray(String f){
  fdi = getIndex(f);
  if(fdi == -1) return -1;  // file not exist
  dataCount=0;
  for(int i=0; i<nRow; i++)data[i]=0;
  Serial.println("f\ : " + f);
  if(!f.startsWith("/"))f = "/" + f;
  Serial.println("f  to open : " + f);
  File file  = SPIFFS.open(f,"r");
  if(!file){Serial.println("could not open " + f); return 0;}
  String str;
  while(file.available()){
    char c_tmp = char(file.read());
    if(c_tmp != '{' ||  c_tmp != '}'){
      str +=c_tmp;
      int iof = str.indexOf(",");
      if(iof != -1){
        data[dataCount] = str.toInt();
        if(dataCount%100 == 0)Serial.println("data[" + String(dataCount) + "] : " + String(data[dataCount]));
        dataCount++;
        str=""; 
      }
    }
  }
  data[dataCount] = str.toInt();
  file.close();
  
  for(int z=0; z<maxFile; z++){
    if(z==fdi){
      fTable[z].state = 1;
      Serial.println("initload fdi=19 dari mana");
      Serial.println("f  : "  + f);
      Serial.println("fdi = z " + String(z));
      Serial.println("fTable[" + String(z) + "].name : " + fTable[z].name);
    }else{
      fTable[z].state = 0;
    }
  }
  afname = f;
  Serial.println("[Done] dataCount : " + String(dataCount));
  Serial.println("cPos :  " + String(cPos));
  mFlag=true;
  return 1;
}


int numRow(String _fname){
    int r=0;
    if(!_fname.startsWith("/"))_fname = "/" + _fname;
    File f = SPIFFS.open(_fname,"r");
    String stmp ="";
    //Serial.println("fname : " + _fname);
    while(f.available()){
       char _c = char(f.read());
       stmp += _c;
       int iof = stmp.indexOf(",");
       if(iof != -1){
          //if(r%100 ==0)Serial.println("r : " + String(r));
          r++;
          stmp="";
       }
     }
     f.close();
     return r;
}
void updateList(){
   int _i=0;
   String path = "/";
   Dir dir  = SPIFFS.openDir(path);
    while(dir.next()){
       File entry = dir.openFile("r");
       String tmp = entry.name();
       //Serial.println("tmp : " + tmp);
       entry.close();
       if(tmp.endsWith(".sta")){
             fTable[_i].name = tmp;
             fTable[_i].nRow = numRow(tmp);
             if(afname != "" && fTable[_i].name==afname){
              fTable[_i].state=1;
              fdi=_i;
             }else{
              fTable[_i].state=0;
             }
             _i++;
             fcount = _i;
       }
   }
}


void handleRoot(){
  updateList();
  String s ="<html><body><h3> File List</h3>";
  s+="<table><tr><th>State</th><th> FileName </th><th> numRow </th><th> load To Memory </th><th> delete </th></tr>";
  for(int i=0; i<fcount; i++){
      s+="<tr><td>" + String(fTable[i].state?"*":"") +  "</td>";
      s+="<td>" + fTable[i].name + "</td>";
      s+="<td>" + String(fTable[i].nRow) + "</td>";
      s+="<td><a href=\"/dataArray?fname=" + fTable[i].name + "\">Load to Program</a></td>";
      s+="<td><a href=\"/delete?fname=" + fTable[i].name + "\">DELETE</a></td></tr>";
  }        
  s+="</table><a href=\"/upform\">Upload</a></body></html>";
  server.send(200,"text/html",s);
}

void indexPage(){
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleFileUpload(){
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/"))filename = "/" + filename;
    //Serial.println("handleFileUpload Name: " + filename);
    fsUploadFile = SPIFFS.open(filename,"w");
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile){
      fsUploadFile.write(upload.buf,upload.currentSize);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    fsUploadFile.close();
    //Serial.println("handleFileUpload Size: " + String(upload.totalSize));
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(500,"text/plain","500 : couldn't create file");
  }
  
}

void handleSuccess(){
  handleRoot();
}

void handleDataArray(){
  dataCount=0;
  for(int i=0; i<nRow; i++)data[i]=0;
  String fname = server.arg("fname");
  Serial.println("fname : " + fname);
  if(!fname.startsWith("/"))fname = "/" + fname;
  Serial.println("fname  to open : " + fname);
  File file  = SPIFFS.open(fname,"r");
  String str;
  while(file.available()){
    char c_tmp = char(file.read());
    //Serial.println("c_tmp :  " + c_tmp);
    if(c_tmp != '{' ||  c_tmp != '}'){
      str +=c_tmp;
      int iof = str.indexOf(",");
      if(iof != -1){
        data[dataCount] = (str.substring(0,iof)).toInt();
        if(dataCount%100 == 0)Serial.println("data[" + String(dataCount) + "] : " + String(data[dataCount]));
        dataCount++;
        str=""; 
      }
    
    }
  }
  data[dataCount] = str.toInt();
  file.close();
  for(int z=0; z<maxFile; z++){
    if(fname.endsWith(fTable[z].name)){
      fTable[z].state = 1;
      fdi=z;
      mFlag=true;
    }else{
      fTable[z].state = 0;
    }
  }
  cPos=-1;
  afname = fname;
  Serial.println("[Done] dataCount : " + String(dataCount));
  server.sendHeader("Location", "/");
  server.send(303);
  //handleRoot();
  //server.send(200,"text/html","proses");
}



void  handleDelete(){
  String f=server.arg("fname");
  for(int i=0; i<maxFile; i++){
    if(fTable[i].name == f){
      if(fTable[i].state==1){
        for(int x=0; x<nRow; x++){
          data[x]=0;
        }
        dataCount=0;
        cPos=-1;
        afname="";
        mFlag=false;
        SPIFFS.remove("/logger.txt");
      }
      fTable[i].state=0;
      fTable[i].nRow=0;
      fTable[i].name="";
    }
  }
  SPIFFS.remove(f);
  //handleRoot();
  fcount--;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
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
  lcd.setCursor(10,1);
  lcd.print(data[cPos]);

}

void upPos(){
  if(mFlag){
    cPos++;
    if(cPos>dataCount){
      Serial.println("cPos HIT Maximum!!! " + String(cPos));
      cPos=dataCount;
    }
    lcdPos();
 //   pShow(cPos);
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
//    pShow(cPos);
    logger();
    Serial.println("cPos : " +String(cPos));
  } else {
    Serial.println("load file first from web browser");
  }
}

void pShow(int pos){
//  pixels.clear();
//  pixels.setPixelColor(data[pos],pixels.Color(150,0,0));
//  pixels.setPixelColor(data[pos+1],pixels.Color(0,150,0));
//  pixels.show();
  
}

void reboot(){
  Serial.println("Reboot...");
  for(int i=0;i<20; i++){
    Serial.print("*"); 
    delay(10);
  }
  ESP.restart();
}

void resetWiFi(){
  WiFiManager wm;
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reset WiFi");
  delay(1000);
  wm.resetSettings();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Reboot...");
  for(int i=0;i<16; i++){
    lcd.setCursor(0,1);
    lcd.print("*"); 
    delay(100);
  }
  ESP.restart();
}

void startPortal(){
  WiFiManager wm;
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
  if(hFlag){
    if (WiFiMulti.run() == WL_CONNECTED) {

      WiFiClientSecure client;
      client.setInsecure();


      ESPhttpUpdate.onStart(update_started);
      ESPhttpUpdate.onEnd(update_finished);
      ESPhttpUpdate.onProgress(update_progress);
      ESPhttpUpdate.onError(update_error);

      t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://raw.githubusercontent.com/tbagindo/httpupdate/main/smartString.bin");


      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;

        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          break;
      }
    }
  }else{
    server.handleClient();
  }
  
}
