void initWebServer(){
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

void handleRoot(){
  int z=0;
  updateList();
  String s ="<html><body><h3> File List</h3>";
  s+="<table><tr><th>State</th><th> FileName </th><th> numRow </th><th> load To Memory </th><th> delete </th></tr>";
  for(int i=0; i<fcount; i++){
      s+="<tr><td>" + String(fTable[i].state?"*":"") +  "</td>";
      s+="<td>" + fTable[i].name + "</td>";
      s+="<td>" + String(fTable[i].nRow) + "</td>";
      s+="<td><a href=\"/dataArray?fname=" + fTable[i].name + "\">Load to Program</a></td>";
      s+="<td><a href=\"/delete?fname=" + fTable[i].name + "\">DELETE</a></td></tr>";
      if(fTable[i].state == 1){
        clearRow(0,1);
        lcd.print(fTable[i].name + "[*]");
        z++;
      }
  }        
  s+="</table><a href=\"/upform\">Upload</a></body></html>";
  server.send(200,"text/html",s);
  if(z==0){
    delay(200);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("[web] listTable");
  }

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
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile){
      fsUploadFile.write(upload.buf,upload.currentSize);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    fsUploadFile.close();

    //Serial.println("handleFileUpload Size: " + String(upload.totalSize));
    server.sendHeader("Location", "/");
    server.send(303);
    lcd.setCursor(0,1);
    lcd.print("[FS] : Uploaded");
    delay(500);
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
  //Serial.println("fname : " + fname);
  if(!fname.startsWith("/"))fname = "/" + fname;
  //Serial.println("fname  to open : " + fname);
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("[RAM] " + afname);
  lcd.setCursor(0,1);
  lcd.print("Line:  " + String(dataCount));
  delay(500);
  logger();
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(f);
  lcd.setCursor(0,1);
  lcd.print("[FS] : Deleted");
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
