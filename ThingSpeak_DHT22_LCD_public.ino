// Set up LCD library.
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

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

void setup()
{
  lcd.begin(16, 2);  // LCD
  dht.begin();	// DHT22
  pinMode(A0, INPUT);	// LDR

  lcd.print("Initializing...");
  Serial.begin(9600);
  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }
}

int brightness;		// Variable for LDR
int triggersend=0;      // Variable to keep track of LCD updating. After certain number, data send through wifi module.
int displaytrack=1;      // Variable to keep track of what's on the LCD. LCD changes to display one value at a time.
int displaytracktime=0;  // Variable to count 5 seconds, time each data value is displayed.

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
  // First line of LCD.
  lcd.setCursor(0, 0);
  if(displaytrack==1){
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print((char)223);
    lcd.print("C     ");
    displaytracktime++;
    if(displaytracktime == 5){
      displaytracktime=0;
      displaytrack=2;
    }
  }
  else if(displaytrack==2){
    lcd.print("Hum: ");
    lcd.print(h);
    lcd.print("%     ");
    displaytracktime++;
    if(displaytracktime == 5){
      displaytracktime=0;
      displaytrack=3;
    }
  }
  else if(displaytrack==3){
    lcd.print("Lum: ");
    lcd.print(brightness);
    lcd.print("/1024     ");
    displaytracktime++;
    if(displaytracktime == 5){
      displaytracktime=0;
      displaytrack=1;
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
  }
  else{
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



