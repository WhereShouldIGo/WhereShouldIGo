/****************************************************
  This is a library example for the MLX90614 Temp Sensor
 
  Designed specifically to work with the MLX90614 sensors in the
  adafruit shop
  ----> https://www.adafruit.com/products/1748
  ----> https://www.adafruit.com/products/1749
 
  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
 
  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#define DEBUG true
 
#include <Wire.h>
#include <Adafruit_MLX90614.h>
 
// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
 
//#include <Adafruit_Sensor.h>
#include "DHT.h"
 
#include <SoftwareSerial.h>
 
const char* ssid = "lame";
const char* pass = "lame@tkddnr";
 
String private_server = "119.192.202.112";
const int serverPort = 8867;
SoftwareSerial esp8266(2,3);//TX,RX
 
#define DHTPIN 8     // what digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
 
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
 
// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);
 
boolean connect_ap() {
  Serial.println();
  sendData("AT+CWMODE=3\r\n",1000,DEBUG);
  Serial.print("connecting to WiFi ");
  Serial.println(ssid);
  String cmd = "AT+CWJAP=\"";
  cmd+=ssid;
  cmd+="\",\"";
  cmd+=pass;
  cmd+="\"";
  esp8266.println(cmd);
  Serial.println(cmd);
  delay(5000);
 
  if(esp8266.find("OK")){
     Serial.println("\n Got WiFi ");
     return true;
  }
  else return false;
}

void mlx_measure(float *temp_ambient, float *temp_object){

  *temp_ambient=mlx.readAmbientTempC();
  *temp_object=mlx.readObjectTempC();
  Serial.print("Ambient = "); Serial.print(*temp_ambient);
  Serial.print("*C\tObject = "); Serial.print(*temp_object); Serial.println("*C");
}

void dht_measure(float *rel_hum,float *temp,float *hic){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  *rel_hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  *temp = dht.readTemperature();
 
  // Check if any reads failed and exit early (to try again).
  if (isnan(*rel_hum) || isnan(*temp)) {
    Serial.println("Failed to read from DHT sensor!");
    //return;
  }
 
  // Compute heat index in Celsius (isFahreheit = false)
  *hic = dht.computeHeatIndex(*temp, *rel_hum, false);
 
  Serial.print("Humidity: ");
  Serial.print(*rel_hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(*temp);
  Serial.print(" *C ");
  Serial.print("Heat index: ");
  Serial.print(*hic);
  Serial.println(" *C ");
  Serial.println();
}

String sendData(String command, const int timeout, boolean debug){
  String response="";

  esp8266.print(command);

  long int time =millis();
  while((time+timeout)>millis()){
    while(esp8266.available()){
      char c = esp8266.read();
      response+=c;
    }
  }
  if(debug){
    Serial.print(response);
  }
  return response;
}

void send_post_packet(float temp_ambient,float temp_object,float rel_hum,float temp,float hic){
  Serial.println("Starting connection to server...");
  String content="?amb="+String(temp_ambient)+"&obj="+String(temp_object)+"&hum="+String(rel_hum);

  sendData("AT+CIPSTART=\"TCP\",\""+private_server+"\", "+String(serverPort)+"\r\n",1000,DEBUG);
  //Serial.println("AT+CIPSTART=\"TCP\",\""+private_server+"\", "+String(serverPort));

  /*if(esp8266.find("OK")){
    Serial.println("TCP connection ready");
  } delay(1000);*/
  //sendData("AT+CIPMODE=1\r\n",1000,DEBUG);
  //delay(1000);

  String postReq="GET /getdata"+content+" HTTP/1.0\r\n\r\n";
  //              +"\r\nAccept: */**\r\nContent-Length: "+String(content.length())
  //              +"\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n";
  String sendCmd="AT+CIPSEND="+String(postReq.length())+"\r\n";
  sendData(sendCmd,500,DEBUG);
  sendData(postReq,1000,DEBUG);
  //Serial.println(esp8266.readString());
  /*if(esp8266.find(">")) { 
    Serial.println("Sending..");
    esp8266.print(postReq);
    if(esp8266.find("SEND OK")){
      Serial.println("Packent sent");
      while(esp8266.available()){
        String tmpResp=esp8266.readString();
        delay(1000);
        Serial.println(tmpResp);
      }
      esp8266.println("AT+CIPCLOSE");
    }
  }*/
  sendData("AT+CIPCLOSE\r\n",1000,DEBUG);
}
 
void setup() {
  Serial.begin(9600);
  
  esp8266.begin(9600);
  //sendData("AT+RST",2000,DEBUG);
  Serial.println("ESP8266 connect");
  Serial.println("Adafruit MLX90614 & Dht11");
  
  boolean connected = false;
  for(int i=0;i<10;i++){
    if(connect_ap()){
      connected=true;
      //TEST
      //esp8266.println("AT+UART_DEF=9600,8,1,0,0");
      break;
    }
  }
  delay(5000);

  Serial.println("Single Connection");
  sendData("AT+CIPMUX=0\r\n",1000,DEBUG);
  mlx.begin();
  dht.begin();
}
 
void loop() {
  float temp_ambient,temp_object;
  float rel_hum,temp,hic;
  mlx_measure(&temp_ambient,&temp_object);
  dht_measure(&rel_hum,&temp,&hic);

  send_post_packet(temp_ambient,temp_object,rel_hum,temp,hic);
  delay(10000);
}
