#include "WizFi360.h"
#include <SPI.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DS3231.h>
#include <Servo.h>

#define SENSOR_PIN 27
#define BUZZER 8

RTClib myRTC;
Servo myservo;  // create servo object to control a servo

/* Baudrate */
#define SERIAL_BAUDRATE   115200
#define Serial2_BAUDRATE  115200

char ssid[] = "xxxxxxxxxx";       // your network SSID (name)
char pass[] = "xxxxxxxxxx";   // your network password
int status = WL_IDLE_STATUS;  // the Wifi radio's status

char server[] = "xxxxxxxxxxx";

IPAddress ip(0, 0, 0, 0); // Update these with your ip
//IPAddress server(0, 0, 0, 0); // Update these with your server ip

// Initialize the Ethernet client object
WiFiClient client;
PubSubClient pclient(client);

boolean feed = true; // condition for alarm

void setup()
{
  // initialize serial for debugging
  Serial.begin(SERIAL_BAUDRATE);
  // initialize serial for WizFi360 module
  Serial2.begin(Serial2_BAUDRATE);
  // initialize WizFi360 module
  WiFi.init(&Serial2);

  Wire.begin();
  pinMode(SENSOR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  delay(500);
  Serial.println("WizFi360 Ready!");

  pclient.setServer(server, 1883);
  pclient.setCallback(callback);

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  printWifiStatus();

}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  if (!pclient.connected()) {
    reconnect();
  }
  pclient.loop();

  int hr, sensor_val = 0;
  DateTime now = myRTC.now();
  hr = now.hour();
  sensor_val = digitalRead(SENSOR_PIN);
  if ((hr == 7 && feed == true) || (hr == 12 && feed == true) || (hr == 17 && feed == true) || (hr == 20 && feed == true) || (sensor_val == 1))
  {
    myservo.write(100);
    digitalWrite(BUZZER, HIGH);
    delay(400);
    myservo.write(55);
    digitalWrite(BUZZER, LOW);
    delay(15);
    feed = false;
  }

   if ((hr == 8 && feed == false) || (hr == 13 && feed == false) || (hr == 18 && feed == false) || (hr == 21 && feed == false))
  {
    feed = true;
  }

  // wait to let all the input command in the serial buffer
  delay(5);
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!pclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (pclient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      pclient.publish("outTopic", "hello world");
      // ... and resubscribe
      pclient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
