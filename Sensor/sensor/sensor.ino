#define DEBUG true
 
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

SoftwareSerial mp3Serial(6,7);

const char* ssid = "lame";
const char* pass = "lame@tkddnr";
 
String private_server = "119.192.202.112";
const int serverPort = 8867;
SoftwareSerial esp8266(2,3);//TX,RX
 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
 
// UltraSonic Sensor interfacing to Arduino .

//int buzzer = 9;
int triggerPin = 11; //triggering on pin 7
int echoPin = 12; //echo on pin 8

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
  mp3Serial.begin(9600);
  mp3_set_serial (mp3Serial);      // DFPlayer-mini mp3 module 시리얼 세팅
  delay(1);                     // 볼륨값 적용을 위한 delay
  mp3_set_volume (15);          // 볼륨조절 값 0~30
  
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
  
  pinMode(triggerPin, OUTPUT); //defining pins
  pinMode(echoPin, INPUT);

  
}

int dist_measure(){

  int duration, distance; //Adding duration and distance
  digitalWrite(triggerPin, HIGH); //triggering the wave(like blinking an LED)
  delay(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH); //a special function for listening and waiting for the wave
  distance = (duration/2) / 29.1; //transforming the number to cm(if you want inches, you have to change the 29.1 with a suitable number
  delay(10);
  Serial.print(distance); //printing the numbers
  Serial.println("cm"); //and the unit
  
  return distance;
}

#define interval 1000
unsigned long mark=0;
void loop() {
  boolean play_state = digitalRead(8);
  float temp_ambient,temp_object;
  float rel_hum,temp,hic;
  int distance;
  
  if(millis()>mark){
    mark=millis()+interval;
    distance=dist_measure();
    
    while(distance<100){
      mlx_measure(&temp_ambient,&temp_object);
      mp3_play(1);
      delay(5000);
      distance=dist_measure();
    }
    mp3_stop();
//Serial.println(" "); //just printing to a new line

    //send_post_packet(temp_ambient,temp_object,rel_hum,temp,hic);
  //delay(10000);
  }
}
