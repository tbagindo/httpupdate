#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <FS.h>

WiFiManager wm;
ESP8266WebServer server(80);

//variable
int spState=0; // 0 webserver, 1 wifimanager
File fsUploadFile;
//String fname;
int dataCount=0;
const int nRow=4001;
int data[nRow];
int fcount=0;
const int maxFile=20;

struct FTstruct{
  String name;
  int nRow;
  int state;
};

FTstruct fTable[maxFile];

void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  wm.autoConnect("ApaITU-StringArt");
  
  //webserver router
  server.on("/",handleRoot);
  server.on("/upload",HTTP_POST,[](){server.send(200);},handleFileUpload);
  server.on("/upform",indexPage);
  server.on("/success",handleSuccess);
  server.on("/dataArray",handleDataArray);
  server.on("/delete",handleDelete);
  server.on("/wifiPortal", startPortal);
  server.on("/reset",[](){wm.resetSettings();Serial.println("Reboot...");for(int i=0;i<20; i++){Serial.print("*"); delay(10);ESP.restart();}});
  server.on("/reboot", [](){Serial.println("Reboot...");for(int i=0;i<20; i++){Serial.print("*"); delay(10);ESP.restart();}});
  server.onNotFound(handleNotFound);
  
  //start webserver
  server.begin();
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}
void startPortal(){
  Serial.println("Stop webserver ");
  server.stop();
  Serial.println("Start wmPortal");
  wm.startWebPortal();
  spState =1;
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
       Serial.println("tmp : " + tmp);
       entry.close();
       if(tmp.endsWith(".sta")){
             fTable[_i].name = tmp;
             fTable[_i].nRow = numRow(tmp);
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
  file.close();
  for(int z=0; z<maxFile; z++){
    if(fname.endsWith(fTable[z].name)){
      fTable[z].state = 1;
    }else{
      fTable[z].state = 0;
    }
  }
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



void loop() {
  if(spState == 0){
    server.handleClient();
  } else {
    wm.process();
  }

}
