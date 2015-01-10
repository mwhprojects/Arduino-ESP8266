#include "DHT.h"
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SSID "[MY_SSID]"
#define PASS "[MY_PASSWORD]"
#define IP "184.106.153.149"    // thingspeak.com
#define TSKEY "[MY_TS_KEY]" // Thingspeak Key

int brightness;

void setup()
{
  dht.begin();
  
  pinMode(A0, INPUT);  // LDR
  
  Serial.begin(9600);
  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }
}

void loop(){
  brightness = analogRead(A0);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    t = -1;
    h = -1;
    return;
  }
  sendData(String(brightness), String(t), String(h));
  delay(120000);
}

void sendData(String tBrightness, String tTemp, String tHum){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    return;
  }
  cmd = "GET /update?key=";
  cmd += TSKEY;
  cmd += "&field1=";
  cmd += tBrightness;
  cmd += "&field2=";
  cmd += tTemp;
  cmd += "&field3=";
  cmd += tHum;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")){
    Serial.print(cmd);
  }else{
    Serial.println("AT+CIPCLOSE");
  }
}

 
boolean connectWiFi(){
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  delay(5000);
  if(Serial.find("OK")){
    return true;
  }else{
    return false;
  }
}
