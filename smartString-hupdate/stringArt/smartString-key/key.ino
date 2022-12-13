void keystart(){
  String s = WiFi.macAddress();
  File f = SPIFFS.open("/key.txt","r");
  if(!f){
    while(1){Serial.print(".");delay(1000);}
  }
  String key;
  while(f.available()){
  char c = char(f.read());
  key+= c;
    
  }
  for(int i=0; i<10; i++)s+=" ";
  s+="ApaITU";
  f.close();
  if(key != s){
    
    while(1){Serial.print(".");delay(1000);}
  }
  Serial.println("[start lcd]");
   
}
 
