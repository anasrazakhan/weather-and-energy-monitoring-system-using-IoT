int LDR = A0;

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "DHT.h"
#define DHTTYPE DHT11
#define DHTPIN 14
DHT dht(DHTPIN, DHTTYPE);

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

#include <PZEM004T.h>
HardwareSerial hwserial(UART0);     // Use hwserial UART0 at pins GPIO1 (TX) and GPIO3 (RX)
PZEM004T pzem(&hwserial);           // Attach PZEM to hwserial
IPAddress ip(192,168,1,1);
bool pzemrdy = false;

#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>

Ticker ticker;

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int ldrVal;
float dsb_temp, dht_temp;
float dht_hum, curr, powe, ener;
int volts;
//String myStatus = "";


void setup() {
  //Serial.begin(115200);      
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  ticker.attach(30, senddata);
  WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
  dht.begin();
  ds18b20.begin();
  lcd.begin();
  lcd.backlight();
  while (!pzemrdy) {
      lcd.println("Connecting to PZEM...");
      pzemrdy = pzem.setAddress(ip);
      delay(1000);
      lcd.clear();
      delay(1000);
   }
}

void loop() {
  // set the status
  //ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
    if(WiFi.status() != WL_CONNECTED){
      lcd.clear();
      lcd.print("Connecting to");
      lcd.setCursor(0,1);
      lcd.print(SECRET_SSID);
      delay(4000);
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        lcd.print(".");
        delay(2000); 
      }
      lcd.clear(); 
      lcd.print("Connected.");
      delay(2000);
    }
    if(WiFi.status() == WL_CONNECTED){
      lcd.clear();
      lcd.print("Connected to");
      lcd.setCursor(0,1);
      lcd.print(SECRET_SSID);
      delay(2500);
      lcd.clear();
    }
    
  ldrVal = analogRead(LDR);
  ds18b20.requestTemperatures(); // Send the command to get temperatures
  dsb_temp = ds18b20.getTempCByIndex(0);
  lcd.setCursor(0,0);
  lcd.print("LDR Value: ");
  lcd.print(ldrVal);
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.print(dsb_temp);
  lcd.print("*C");
  delay(3000);
  lcd.clear();
  //Serial.print(", ");
  //Serial.println();
  dht11Function();
  //Serial.print("Requesting temperatures...");
  //Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  //Serial.print("Temperature for the device 1 (index 0) is: ");
  //Serial.println(ds18b20.getTempCByIndex(0));
  lcd.setCursor(0,0);
  lcd.print(" PZEM READINGS ");
  delay(2000);
  lcd.clear(); 
  lcd.setCursor(0,0);
  float v = pzem.voltage(ip);
  if (v < 0.0) v = 0.0;
   volts = v;
   lcd.print(v);lcd.print("V; ");

  float i = pzem.current(ip);
   if(i >= 0.0){ lcd.print(i);lcd.print("A; "); curr = i;}

  lcd.setCursor(0,1);
  float p = pzem.power(ip);
   if(p >= 0.0){ lcd.print(p);lcd.print("W; "); powe = p;}

  float e = pzem.energy(ip);
   if(e >= 0.0){ lcd.print(e);lcd.print("Wh; "); ener = e;}

  delay(3000);
  lcd.clear();

}
void senddata()
{
  // You can send any value at any time.
  ThingSpeak.setField(1, ldrVal);
  ThingSpeak.setField(2, dsb_temp);
  ThingSpeak.setField(3, dht_temp);
  ThingSpeak.setField(5, dht_hum);
  ThingSpeak.setField(4, volts);
  ThingSpeak.setField(6, curr);
  ThingSpeak.setField(7, powe);
  ThingSpeak.setField(8, ener);
}

void dht11Function(void){
  float h = dht.readHumidity();
  dht_hum = h;

  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  dht_temp = t;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //Serial.print("Humidity: ");
  //Serial.print(h);
  //Serial.print(" %\t");
  //Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.print(" *C ");
  //Serial.print(f);
  //Serial.print(" *F\t");
  //Serial.print("Heat index: ");
  //Serial.print(hic);
  //Serial.print(" *C ");
  //Serial.print(hif);
  //Serial.println(" *F"); 
  lcd.clear();
  lcd.print("  DHT READINGS  ");
  delay(2000);
  lcd.clear(); 
  lcd.setCursor(0,0);   
  lcd.print("H:");
  lcd.print(h);
  lcd.print("%");
  lcd.print(" T:");
  lcd.print(t);
  lcd.print("*C");
  lcd.setCursor(0,1);
  lcd.print("HI: ");
  lcd.print(hic);
  lcd.print("*C");
  delay(2000);
  lcd.clear();
}
