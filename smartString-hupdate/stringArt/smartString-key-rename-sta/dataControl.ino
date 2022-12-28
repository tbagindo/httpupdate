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

bool lastState(){
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
    return true;
  }else{
    Serial.println("could not open file  : " +  afname + "open web-browser");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("No File loaded");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Upload File via");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    return false;
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
