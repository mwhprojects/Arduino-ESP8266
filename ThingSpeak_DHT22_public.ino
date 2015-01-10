// Set up DHT22 library.
#include "DHT.h"
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Set up macros for wifi and Thing Speak connection.
#define SSID "[MY_SSID]"		// SSID
#define PASS "[MY_PASSWORD]"	// Password
#define TSKEY "[MY_TS_KEY]"		// Thingspeak Key
#define IP "184.106.153.149"    // thingspeak.com

void setup()
{
  dht.begin();	// DHT22
  pinMode(A0, INPUT);	// LDR
  
  Serial.begin(9600);
  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }
}

int brightness;		// Variable for LDR

void loop(){
  brightness = analogRead(A0);			// Get LDR reading
  float t = dht.readTemperature();		// Get temperature (in C) from DHT22 sensor
  float h = dht.readHumidity();			// Get humidity (in %) from DHT22 sensor
  
  // If there is an error with the DHT22 sensor readings, send obvious bad reading.
  if (isnan(h) || isnan(t)) {
    t = -99;
    h = -99;
    return;
  }
  
  
  sendData(String(brightness), String(t), String(h));	// Call function to send data to Thing Speak.
  delay(120000);	// Send data every 2 minutes.
}

void sendData(String tBrightness, String tTemp, String tHum){
  // Set up TCP connection.
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    return;
  }
  
  // Send data.
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
