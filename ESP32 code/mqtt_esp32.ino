/*
MQTT Client for werkplaats TkkrLab
Author : Dave Borghuis

Colors for Fastlib
http://fastled.io/docs/3.1/group___pixeltypes.html#gaeb40a08b7cb90c1e21bd408261558b99

CRGB &   fadeToBlackBy (uint8_t fadefactor)
  fadeToBlackBy is a synonym for nscale8( ..., 255-fadefactor)

Hercheck pins met Renze, Dave was papiertje kwijt.
GND     Blauw 
Voeding Oranje 

Wit 
Geel
pin 23 - MOSI voor hardware SPI ondersteuning
pin 22 - IC2 SCL

Groen
pin 17 16 voor buttons


BOARD
DOIT ESP32 DEVKIT V1 

*/

#include <WiFi.h>
#include <PubSubClient.h>

//Defenitions for the leds
#include "FastLED.h"
#define NUM_LEDS 8
#define DATA_PIN 23
#define CLK_PIN 22
#define LED_TYPE    APA102
#define COLOR_ORDER GRB
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
CRGB leds[NUM_LEDS];

const int first_buttonPin = 17; 
const int second_buttonPin = 16; 

const char* ssid = "www.tkkrlab.nl";
const char* password = "*";
const char* mqtt_server = "server";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
//int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    
    setColor(0,7,CRGB::Black);
    FastLED.show();  

    delay(500);
    Serial.print(".");
    
    setColor(0,7,CRGB::Blue);
    FastLED.show();  

  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if (strcmp(topic,"tkkrlab/werkplaats/state")==0) {

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      Serial.println("Werkplaats Turn space open.");
      setColor(0,3,CRGB::Green);
    } else {
      Serial.println("Werkplaats Turn space closed.");  
      setColor(0,3,CRGB::Red);
    }
  };

  if ( strcmp(topic,"tkkrlab/test/spacestate")==0) {
      // Switch on the LED if an 1 was received as first character
      if ((char)payload[0] == '1') {
        Serial.println("Space Turn space open.");
        setColor(4,7,CRGB::Green);
      } else {
        Serial.println("Space Turn space closed.");    
        setColor(4,7,CRGB::Red);
      }
  }

  FastLED.show(); 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    setColor(0,7,CRGB::Blue);
    FastLED.show();  

    Serial.print("Attempting MQTT connection...");
    String clientId = "MQTT_Werkplaats";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("tkkrlab/werkplaats/io/input/0", "0");
      // ... and resubscribe
      client.subscribe("tkkrlab/werkplaats/state");
      //TODO check channel
      //client.subscribe("tkkrlab/spacestate");
      client.subscribe("tkkrlab/test/spacestate");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      setColor(0,7,CRGB::Black);
      FastLED.show();  

    }
  }
}

void setup() {
    
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //FastLED.addLeds<NEOPIXEL, 12>(leds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  FastLED.setBrightness(BRIGHTNESS);

  //button
  pinMode(first_buttonPin, INPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    if(digitalRead(first_buttonPin)) {
      client.publish("tkkrlab/werkplaats/io/input/0", "0");
      Serial.print("Button Werkplaats !!");    
    };
  };
  
  //For testing only, no need in final product.
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:
    int action = Serial.parseInt();
   
    // look for the newline. That's the end of your sentence:
    if (Serial.read() == '\n') {
        Serial.print("Action value is :");
        Serial.println(action, DEC);
        
        switch (action) { 
          case 0:
            client.publish("tkkrlab/werkplaats/io/input/0", "0");
            break;            
          case 1:
            client.publish("tkkrlab/werkplaats/io/input/0", "1");
            break;
          default:
            Serial.println("Invalid value for action!!");
        }; 
      }
    }

   //client.loop(); 
   //EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void setColor(int ledstart,int ledend, CRGB color) {
  for(int i = ledstart;i<ledend;i++){
    leds[i] = color;
    //fadeToBlackBy();
  }
}
