void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("httpUpdate Started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
//  lcd.clear();
 // lcd.setCursor(0,0);
//  lcd.print("httpUpdate Finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
 // lcd.clear();
 // lcd.setCursor(0,0);
 // lcd.print("Progress : ");
 // lcd.print(cur);
 // lcd.setCursor(0,1);
 // lcd.print("Total : ");
 // lcd.print(total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void hUpdateFlag(){
  hFlag=true;
}

void httpUpdateStart() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClientSecure client;
    client.setInsecure();


    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, repo);


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
}
