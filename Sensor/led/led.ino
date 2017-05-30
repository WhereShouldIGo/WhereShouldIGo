int grnled=16;
int redled=5;
int yelled=4;
int brightup=0;

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

int interval = 10000;

const char* ssid     = "lame";
const char* password = "lame@tkddnr";

const char* private_server = "119.192.202.112";
const int serverPort       = 8867;

void connect_ap() {
  Serial.println();
  Serial.print("connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n Got WiFi, IP address: ");
  Serial.println(WiFi.localIP());  
}

void connecting_http() { 
  WiFiClient client;

  Serial.print("\nconnecting to Host: ");
  Serial.println(private_server);

  //Connect to Private Server
  if(client.connect(private_server,serverPort)){
    //http call to server by using GET Method.
    String getheader = "GET /readdata HTTP/1.1";
    client.println(getheader);
    client.println("User-Agent: ESP8266 Sang Uk");  
    client.println("Host: " + String(private_server));  
    client.println("Connection: close");  
    client.println();
  
    Serial.println(getheader);//To Check
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      String check="Green\n";
      Serial.println(line);
      //Serial.print(check);
      if(line=="Green\r"){
       Serial.println("LED turn to Green");
       brightup=grnled;
      }
      else if(line=="Yellow\r"){
        Serial.println("LED turn to  Yellow");
        brightup=yelled;
      }
      else if(line=="Red\r"){
        Serial.println("LED turn to Red");
        brightup=redled;
      }
    }
  }
  else{
    Serial.print("connection failed to ");
    Serial.println(private_server);
  }
  
  Serial.println("Done cycle.");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(grnled,OUTPUT);
  pinMode(redled,OUTPUT);
  pinMode(yelled,OUTPUT);
  connect_ap();
}

unsigned long mark=0;
void loop() {
   if (millis() > mark ) {
     mark = millis() + interval;
     connecting_http();
     digitalWrite(brightup,HIGH);
     if(brightup!=grnled) digitalWrite(grnled,LOW);
     if(brightup!=yelled) digitalWrite(yelled,LOW);
     if(brightup!=redled) digitalWrite(redled,LOW);
     //delay(2000);
     //digitalWrite(brightup,LOW);
     //delay(1000);
  }
  
  // put your main code here, to run repeatedly:
  
}
