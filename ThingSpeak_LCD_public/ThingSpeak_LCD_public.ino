/********************************************
 * This code collects readings from a DHT22 temperature and humidity sensor
 * (requires DHT library) and a photoresistor (aka light dependent resistor,
 * or "LDR"), then sends the data to Thing Speak using an ESP8266 module. The
 * values from the sensors and a countdown to when the data will be sent are
 * displayed on an LCD (requires LiquidCrystal library).
 * 	
 * GitHub Repo: https://github.com/mwhprojects/Arduino-ESP8266
 ********************************************/

// Set up LCD library and variables.
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define LCDLIGHT 6    // Backlight pin
#define LCDLIGHTSW 7  // Backlight enable switch
int LCDLIGHTSWstate = 0;

// Set up DHT22 library.
#include "DHT.h"
#define DHTPIN A1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Set up macros for wifi and Thing Speak connection.
#define SSID "[MY_SSID]"		// SSID
#define PASS "[MY_PASSWORD]"	      // Password
#define TSKEY "[MY_THINGSPEAK_KEY]"    // Thingspeak Key
#define IP "184.106.153.149"          // thingspeak.com

int brightness;		// Variable for LDR
int triggersend=0;      // Variable to keep track of LCD updating. After certain number, data send through wifi module.
int displaytrack=1;      // Variable to keep track of what's on the LCD. LCD changes to display one value at a time.
int displaytracktime=0;  // Variable to count 5 seconds, time each data value is displayed.

int errorcount = 0;    // Variable for triggering error message.
/*
  Variable to decide which error message to display.
 0: DHT22 error
 1: ESP8266 error
 */
int errortype = 0;

void setup()
{
  lcd.begin(16, 2);  // LCD
  pinMode(LCDLIGHT, OUTPUT);  // LCD backlight
  pinMode(LCDLIGHTSW, INPUT); // LCD backlight switch
  if(digitalRead(LCDLIGHTSW)==HIGH){
    analogWrite(LCDLIGHT, 255);
  }else{
    analogWrite(LCDLIGHT, 0);
  }

  dht.begin();	// DHT22
  pinMode(A0, INPUT);	// LDR

  lcd.print("Initializing...");
  Serial.begin(9600);
  // Test ESP8266 module.
  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }
}

void LCDerrorfade(){
  int backlightfade = 0;
  int i = 0;
  for(i=0; i<3; i++){
    for(backlightfade = 255; backlightfade>55; backlightfade--){
      analogWrite(LCDLIGHT, backlightfade);
      delay(2);
    }
    for(backlightfade = 55; backlightfade<255; backlightfade++){
      analogWrite(LCDLIGHT, backlightfade);
      delay(2);
    }
  }
  delay(2000);
}

void loop(){
  // LCD backlight switch
  LCDLIGHTSWstate = digitalRead(LCDLIGHTSW);
  if(LCDLIGHTSWstate == HIGH){
    analogWrite(LCDLIGHT, 255);
  }else{
    analogWrite(LCDLIGHT, 0);
  }
  
  // Error message.
  if(errorcount == 1){
    errorcount = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    if(errortype == 0){
      // DHT22 error
      lcd.print("ERROR: DHT22");
      lcd.setCursor(0, 1);
      lcd.print("Check connection");
      if(LCDLIGHTSWstate == HIGH){
        LCDerrorfade();
      }else{
        delay(4000);
      }
    }
    else{
      // ESP8266 error
      lcd.print("ERROR: ESP8266");
      lcd.setCursor(0, 1);
      lcd.print("Check connection");
      if(LCDLIGHTSWstate == HIGH){
        LCDerrorfade();
      }else{
        delay(4000);
      }
    }
    
    triggersend = 0;    // Reset timer.
  }else{
    // Get sensor readings.
    brightness = analogRead(A0);			// Get LDR reading
    float t = dht.readTemperature();		// Get temperature (in C) from DHT22 sensor
    float h = dht.readHumidity();			// Get humidity (in %) from DHT22 sensor
  
    // If there is an error with the DHT22 sensor readings, send obvious bad reading.
    if (isnan(h) || isnan(t)) {
      t = -99;
      h = -99;
      errortype = 0;  // Sets appropriate error message.
      errorcount = 1;   // Increase error counter.
      return;
    }
    // First line of LCD.
    lcd.setCursor(0, 0);
    if(displaytrack==1){
      lcd.print("Temp: ");
      lcd.print(t);
      lcd.print((char)223);
      lcd.print("C     ");
      displaytracktime++;
      if(displaytracktime == 5){
        displaytracktime = 0;
        displaytrack = 2;
      }
    }
    else if(displaytrack==2){
      lcd.print("Hum: ");
      lcd.print(h);
      lcd.print("%     ");
      displaytracktime++;
      if(displaytracktime == 5){
        displaytracktime = 0;
        displaytrack = 3;
      }
    }
    else if(displaytrack==3){
      lcd.print("Lum: ");
      lcd.print(brightness);
      lcd.print("/1024     ");
      displaytracktime++;
      if(displaytracktime == 5){
        displaytracktime = 0;
        displaytrack = 1;
      }
    }
    // Second line of LCD.
    lcd.setCursor(0, 1);
    lcd.print("Send in: ");
    lcd.print(120-triggersend);
    lcd.print("secs  ");
    delay(1000);
    triggersend++;
  
    // After 120 seconds, or 2 minutes, elapse, send the data through ESP8266.
    if(triggersend == 120){
      sendData(String(brightness), String(t), String(h));	// Call function to send data to Thing Speak.
      triggersend = 0;
    }
  }
}

void sendData(String tBrightness, String tTemp, String tHum){
  // Set up TCP connection.
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    errortype = 1;  // Sets appropriate error message.
    errorcount = 1;   // Increase error counter.
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
  }
  else{
    errortype = 1;  // Sets appropriate error message.
    errorcount=1;   // Increase error counter.
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
  }
  else{
    return false;
  }
}




