#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <sqlite3.h>
#include <vfs.h>
#include <SPI.h>
#include <FS.h>

ESP8266WebServer server;

char* ssid = "mktApaITU";
char* password = "1234567890";
File fsUploadFile;
String fname;
int dataCount=0;
const int numrow=4001;
int data[numrow];
int count=0;
uint32_t prevMillis=0;

void setup(){
  SPIFFS.begin();
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",handleFileList);
  server.on("/upform",indexPage);
  server.on("/list",HTTP_GET,handleFileList);
  server.on("/upload",HTTP_POST,[](){server.send(200);},handleFileUpload);
  server.on("/success",handleSuccess);
  server.on("/dataArray",handleDataArray);
  server.onNotFound(handleNotFound);
  server.begin();

}

void indexPage(){
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleFileList(){
  String path = "/";
  Dir dir = SPIFFS.openDir(path);
  String output ="File list : ";
  while(dir.next()){
    output += "<br>";
    File entry = dir.openFile("r");
    String tmp = String(entry.name()).substring(1);
    //Serial.println(tmp);
    if(tmp.endsWith(".sta")){
      output +="<a href=\"/dataArray?fname=" + tmp + "\">" + tmp + "</a>";
    } else {
      output += tmp;
    }
    //Serial.println("output : " + output);
    entry.close(); 
  }
  output +="<br><a href=\"/upform\">upload</a>";
  server.send(200,"text/html",output);
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
    server.sendHeader("Location", "/success");
    server.send(303);
  } else {
    server.send(500,"text/plain","500 : couldn't create file");
  }
  
}

void handleDataArray(){
  dataCount=0;
  for(int i=0; i<numrow; i++)data[i]=0;
  fname = server.arg("fname");
  Serial.println("fname : " + fname);
  fname = "/" + fname;
  Serial.println("fname : " + fname);
  File file  = SPIFFS.open(fname,"r");
  String str;
  while(file.available()){
    char c_tmp = char(file.read());
    //Serial.println("c_tmp :  " + c_tmp);
    if(c_tmp != '{' ||  c_tmp != '}'){
      str +=c_tmp;
      int iof = str.indexOf(":");
      if(iof != -1){
        data[dataCount] = (str.substring(0,iof)).toInt();
        if(dataCount%100 == 0)Serial.println("data[" + String(dataCount) + "] : " + String(data[dataCount]));
        dataCount++;
        str=""; 
      }
    
    }
  }
  file.close();
  Serial.println("[Done] dataCount : " + String(dataCount));
  handleFileList();
  //server.send(200,"text/html","proses");
}

void handleSuccess(){
  handleFileList();
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

void  loop(){
  server.handleClient();
  if(millis()-prevMillis>500){
    prevMillis=millis();
    Serial.println("data[" + String(count) + "/"+ String(dataCount) + "] : " + String(data[count]));

    if(data[count] == 0){
      count=1;
    }else{
      count++;
    }
  }
}
