/*MQTT client library from knolleary.net - contains all necessary APIs related to MQTT*/
#include <PubSubClient.h>
/*wifi client library from esp8266 - contains all necessary APIs related to wifi*/
#include <ESP8266WiFi.h>
#include <Servo.h>


/***************************************************/

const char* ssid = "bharti"; /*Defne your own wifi SSID*/
const char* password = "bharti1297"; /*Defne your own wifi password*/

char* server = "osmosis.axelta.com"; /*Axelta MQTT broker-server host name*/
//IPAddress server(54, 201, 150, 33); /*Axelta MQTT broker-server IP address*/


char* pubtopic = "Farm Trap"; /*MQTT Publish topic*/

/**************************************************/

String clientName1; /*Define string variable for client name  */
char message_buff[500]; /*charecter array  for storing incoming msg during subscription */

/*Initialize the variables as integers indexing                         IOs to which the sensors are connected to*/

int PIR=D1;    //Motion Sensor to GPIO5
int TP=D2; //triggerPin of Ultrasonic sensor
int EP=D4; //EchoPin of Ultrasonic sensor
int M;

Servo myservo;


float gDepth;

WiFiClient wifiClient; /*declaring WiFiClient object variable*/

void callback(char* subtopic, byte* payload, unsigned int length); /*This function is called when message arrived for subscribed topic*/
PubSubClient client(server, 1883, callback, wifiClient); /*Necessary parameters for MQTT clients*/


/*To notify the upper application of certain activities occurring in the MQTT-client core.*/
void callback(char* subtopic, byte* payload, unsigned int length) 
{
  int i = 0;
  Serial.println("Message arrived:  topic: " + String(subtopic));
  Serial.println("Length: " + String(length,DEC));
  for(i=0; i<length; i++) 
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  Serial.println("Payload: " + msgString); /*Print all received msg contents for subscribed topics*/
}

/*Create unique client name using MAC address of NodeMCU board*/

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) 
  {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

/*System function that contains all the necessary settings required like- baud rate, input & output pin configurations*/
void setup() 
{
  delay(1000);
  Serial.begin(115200);
  delay(5000);
  Serial.println("All Sensor Publish to Broker");
  Serial.println();

/*Define GPIO as an input or output*/
  
  pinMode(PIR, INPUT);
  pinMode(TP,OUTPUT);
  pinMode(EP,INPUT);
  myservo.attach(D5);

  /*Connect to local wifi network using SSID & pswd */
  
  Serial.println("Wifi disconnecting... ");
  WiFi.disconnect(); 
  delay(1000);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int dot=0;

/*Wait till wifi network gets connected*/
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    Serial.print(dot);
    dot++;
    if(dot==20||dot==40||dot==60||dot==80||dot==100){Serial.println();}
    if(dot>120){Serial.println();Serial.println("Error Connecting Wifi Network");break;}
  }
  Serial.println();
  Serial.println();
  delay(1000);


  /*Generate client name based on MAC address and last 8 bits of microsecond counter*/
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  clientName1 = clientName;
  delay(1000);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName1);
  delay(1000);

  /*Connect to MQTT broker */
    if (client.connect((char*) clientName1.c_str())) 
  {
    Serial.println("Connected to MQTT broker");
    Serial.print("pubTopic is: ");
    Serial.println(pubtopic);
    delay(1000);
    if (client.publish(pubtopic, "Rodent Security System Ready"))  /*publish hello:test message*/
    {
      Serial.println("Publish ok");
    }
    else 
    {
      Serial.println("Publish failed");
    }
  }
  else 
  {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort(); /*abort the program execution and comes out directly from the place of the call.*/
  }
  delay(1000);
}

/*User function definition for reading data from Ultrasonic sensor */
void Depth()
{ digitalWrite(TP, LOW);
  delay(1);
  digitalWrite(TP, HIGH);
  delay(5);
  digitalWrite(TP, LOW);
  long depth= pulseIn(EP, HIGH)/58.2;
  gDepth=depth;
  Serial.print("Depth: ");
  Serial.println(depth);
 
}
/*User function definition for Motion Sensing and opening the trap if motion is detected*/
void Motion()
{ M=digitalRead(PIR);
  if (M == LOW)
  Serial.println("Rat not Found");
  if (M == HIGH)
  {
    Serial.println("Motion Detected_Opening Trap");
    for(int pos = 0; pos <= 100; pos += 1) // goes from 0 degrees to 80 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(10);                       // waits 15ms for the servo to reach the position 
  } 
  delay(50);
   for(int pos = 100; pos>=0; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  }


}




/*Reconnect function definition with defined pub & sub topics*/
void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
 
    if (client.connect((char*) clientName1.c_str())) 
    {
      Serial.println("connected");
      if (client.publish(pubtopic, "hello from ESP8266")) 
      Serial.println("Publish ok");
      else 
      Serial.println("Publish failed");
      delay(1000);
    }
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/*Main function from where the execution begins */
void loop() 
{
Serial.print("Farm Trap System");
delay(1000);

while(1)
  {
    if (!client.connected()) /*This function should be called regularly to allow the client to process incoming messages and maintain its connection to the server.*/
    {
    reconnect();
    }

    else
    {
    /*calling the functions*/
    Motion();
    Depth();
    delay(500);
    
/*Create string payload for publish*/

  String payload = "{\"MOTION\":\"";
  
  if (M == LOW)
  {
    payload += "No Rodent Found";
  }
  else
  {
    payload += "Motion Detected";
  }
  payload += "\",\"Current Depth of Trap\":\"";
  payload += String(gDepth);

  
/*Publish payload string  */

  if (client.connected())
  {
    Serial.print("Sending payload: ");
    Serial.println(payload);
    if (client.publish(pubtopic, (char*) payload.c_str())) 
    {
      Serial.println("Publish ok");
      Serial.println();
      Serial.println();
      Serial.println();
    }
    else 
    {
      Serial.println("Publish failed");
    }
  }
  delay(5000);

    }
   }
}


