/********************************************
 * Inserts data from DHT22 temperature and humidity sensor and photoresistor into
 * a MySQL database and to a Thing Speak Channel. Data is sent using ESP8266 wifi
 * module.
 * 	
 * GitHub Repo: https://github.com/mwhprojects/Arduino-ESP8266
 ********************************************/
 
/////////////////////////////////////////////////// FILL IN DETAILS
// For wifi connection.
#define SSID ""		// SSID
#define PASS ""      // Network Password

// For webhost connection.
#define HOST ""  // Webhost

// For Thing Speak connection.
#define TSKEY ""    // Thingspeak Key
#define IP ""          // thingspeak.com

// A password for somewhat secure connection.
// Must be consistent with $passcode in data.php.
#define PASSCODE ""

// How often to send data, in seconds. Try to keep under 3 digits. (Default: 180)
#define SENDDELAY 180

// DHT22 pin. (Default: A1)
#define DHTPIN A1

// LCD backlight pin. (Default: 6)
#define LCDLIGHT 6
// LCD backlight enable switch pin. (Default: 7)
#define LCDLIGHTSW 7

/////////////////////////////////////////////////// FILL IN DETAILS

// LCD library
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int LCDLIGHTSWstate = 0;

// DHT22 library
#include "DHT.h"
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int brightness;		// Variable for LDR
int triggersend=0;      // Variable to keep track of LCD updating. After certain number, data send through wifi module.
int displaytrack=1;      // Variable to keep track of what's on the LCD. LCD changes to display one value at a time.
int displaytracktime=0;  // Variable to count 5 seconds, time each data value is displayed.

int errorcount = 0;    // Variable for triggering error message.
// Variable to decide which error message to display.
// 0: DHT22 error
// 1: ESP8266 error
int errortype = 0;

void setup()
{
  lcd.begin(16, 2);  // LCD
  pinMode(LCDLIGHT, OUTPUT);  // LCD backlight
  pinMode(LCDLIGHTSW, INPUT); // LCD backlight switch
  if(digitalRead(LCDLIGHTSW)==HIGH){
    analogWrite(LCDLIGHT, 255);
  }
  else{
    analogWrite(LCDLIGHT, 0);
  }

  dht.begin();	// DHT22
  pinMode(A0, INPUT);	// LDR

  lcd.print("Initializing...");
  lcd.setCursor(0,1);
  lcd.print("Ver: php+TS");  // Differentiating between the couple ATmega328's I'm using...
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
  }
  else{
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
      }
      else{
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
      }
      else{
        delay(4000);
      }
    }

    triggersend = 0;    // Reset timer.
  }
  else{
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
    lcd.print(SENDDELAY-triggersend);
    lcd.print("secs  ");
    delay(1000);
    triggersend++;

    // After 120 seconds, or 2 minutes, elapse, send the data through ESP8266.
    if(triggersend == SENDDELAY){
      sendData(String(brightness), String(t), String(h));	// Call function to send data to Thing Speak.
      triggersend = 0;
    }
  }
}

void sendData(String tBrightness, String tTemp, String tHum){
  // SEND DATA TO WEBHOST
  lcd.clear();
  lcd.print("Sending to");
  lcd.setCursor(0,1);
  lcd.print("webhost...");
  // Set up TCP connection.
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += HOST;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    errortype = 1;  // Sets appropriate error message.
    errorcount = 1;   // Increase error counter.
    return;
  }

  // Send data.
  cmd = "GET /esp8266/data.php?code=";
  cmd += PASSCODE;
  cmd += "&l=";
  cmd += tBrightness;
  cmd += "&t=";
  cmd += tTemp;
  cmd += "&h=";
  cmd += tHum;
  cmd += " HTTP/1.1\r\nHost: ";
  cmd += HOST;
  cmd += "\r\n\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  lcd.clear();
  lcd.print("Webhost Send");
  lcd.setCursor(0,1);
  if(Serial.find(">")){
    Serial.print(cmd);
    lcd.print("OK");
  }
  else{
    lcd.print("FAIL");
  }
  delay(2000);
    
  // SEND DATA TO THING SPEAK
  lcd.clear();
  lcd.print("Sending to");
  lcd.setCursor(0,1);
  lcd.print("Thing Speak...");
  
  // Set up TCP connection for Thing Speak attempt.
  Serial.println("AT+CIPCLOSE");
  delay(7000);
  cmd = "AT+CIPSTART=\"TCP\",\"";
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
  lcd.clear();
  lcd.print("ThingSpeak Send");
  lcd.setCursor(0,1);
  if(Serial.find(">")){
    Serial.print(cmd);
    lcd.print("OK");
  }
  else{
    lcd.print("FAIL");
  }
  delay(2000);
  Serial.println("AT+CIPCLOSE");
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






