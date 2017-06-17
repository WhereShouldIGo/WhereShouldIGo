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

void send_post_packet(float duration,float temp_object){
  Serial.println("Starting connection to server...");
  String content="?dur="+String(duration)+"&obj="+String(temp_object);

  sendData("AT+CIPSTART=\"TCP\",\""+private_server+"\", "+String(serverPort)+"\r\n",1000,DEBUG);

  String postReq="GET /getdata"+content+" HTTP/1.0\r\n\r\n";
  String sendCmd="AT+CIPSEND="+String(postReq.length())+"\r\n";
  sendData(sendCmd,500,DEBUG);
  sendData(postReq,1000,DEBUG);
  
  sendData("AT+CIPCLOSE\r\n",1000,DEBUG);
}
 
void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);
  mp3_set_serial (mp3Serial);      // DFPlayer-mini mp3 module 시리얼 세팅
  delay(1);                     // 볼륨값 적용을 위한 delay
  mp3_set_volume (20);          // 볼륨조절 값 0~30
  mp3_stop();
  
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

long dist_measure(){

  long duration, distance=0; //Adding duration and distance
  while(distance==0){
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH,32760); //a special function for listening and waiting for the wave
    distance = (duration/2) / 29.1; //transforming the number to cm(if you want inches, you have to change the 29.1 with a suitable number
    delay(10);
  }
  Serial.print(distance); //printing the numbers
  Serial.println("cm"); //and the unit
  
  return distance;
}

#define interval 1000
unsigned long mark=0;
boolean playing_flag=0;
float temp_object_max=0;
int max_dist=50;
void loop() {
  boolean play_state = digitalRead(8);
  float temp_ambient,temp_object;
  float rel_hum,temp,hic;
  long distance;
  
  //mark=millis()+interval;
  distance=dist_measure();
 // if(distance<200){
    mlx_measure(&temp_ambient,&temp_object);
    if(temp_object>temp_object_max) temp_object_max=temp_object;
    
    if(playing_flag==0 &&distance<max_dist){
      //MP3 Start
      mark=millis();
      playing_flag=1;
      mp3_play(1);
      delay(4000);      
    }  
    else if(playing_flag==1 && distance>=max_dist){
      //Drop in middle
      int sum=0;
      for(int i=0;i<10;i++){
        if(dist_measure()>=max_dist) sum++;
      }
      if(sum>=9){
        temp_object_max=0;
        mp3_stop();
        playing_flag=0;
        float duration=(millis()-mark)/1000;
        send_post_packet(duration,temp_object);

      }
    }
    else if(playing_flag==1 && play_state==HIGH){
      //LISTEN UNTIL END
      temp_object_max=0;
      playing_flag=0;
      float duration=(millis()-mark)/1000;
      send_post_packet(duration,temp_object);
     
    }
 // }

  delay(1000);
  
}
