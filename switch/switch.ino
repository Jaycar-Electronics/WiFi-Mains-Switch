#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "radio.h"

const short output_pin = D4;

#define ADAFRUIT_USERNAME "doXXXr"
#define ADAFRUIT_AIOKEY "aio_AXXXXXXXXXXXXY51oaAUB67uz4qc"
#define ADAFRUIT_FEED "espswitch"

#define WIFI_SSID "your wifi name"
#define WIFI_PASS "your wifi password"

WiFiClient esp;
Adafruit_MQTT_Client mqtt(
    &esp,
    "io.adafruit.com",
    1883,
    ADAFRUIT_USERNAME,
    ADAFRUIT_AIOKEY);

Adafruit_MQTT_Subscribe switchActivation = Adafruit_MQTT_Subscribe(
    &mqtt,
    ADAFRUIT_USERNAME "/feeds/" ADAFRUIT_FEED);

bool state = 0;
void setup()
{
  Serial.begin(115200);
  setupPins();
  connectWiFi(WIFI_SSID, WIFI_PASS);
  mqtt.subscribe(&switchActivation);
}

void loop()
{
  if (!mqtt.connected())
  {
    reconnectMQTT();
  }

  //get a subscription reading
  Serial.println("Getting updates..");
  Adafruit_MQTT_Subscribe *reading;
  while ((reading = mqtt.readSubscription(5000)))
  {
    //same subscription as our activation subscription
    if (reading == &switchActivation)
    {
      Serial.print(F("Got: "));
      String value = String((char *)switchActivation.lastread);
      Serial.println(value);

      if (value.equals("toggle"))
      {
        //toggle the switch
        state = !state;
        radioSwitch(output_pin, 1, state);
      }
      else if (value.startsWith("enter"))
      {
        //turn on the switch
        radioSwitch(output_pin, 1, 1);
      }
      else if (value.startsWith("exit"))
      {
        //turn off the switch
        radioSwitch(output_pin, 1, 0);
      }
    }
  }

  //if lost connection, disconnect to reconnect later
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }

  delay(1000 * 5); //5 seconds
}
// -----------------------------------------------------------
void setupPins()
{
  pinMode(output_pin, OUTPUT);
}
void connectWiFi(const char *ssid, const char *password)
{
  WiFi.begin(ssid, password);
  Serial.println("Connecting to network:");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to WiFi!");
}
void reconnectMQTT()
{
  Serial.print("Attempting to connect to MQTT... ");
  short retries = 3;
  short ret;
  while (retries > 0)
  {
    if (mqtt.connect() == 0)
    {
      break;
    }
    mqtt.disconnect();
    Serial.printf("%d.. ", retries);
    delay(5000);
    retries--;
  }

  if (!mqtt.connected()) //still not connected
  {
    //hang and wait for WDT to restart
    Serial.println("Unable to connect to MQTT server");
    Serial.println("Make sure your details are correct");
    Serial.println("Restarting soon ...");
    delay(5000);
    // this for loop will crash the ESP, causing it to reset
    for (;;)
      ;
  }
  Serial.println("Connected to MQTT server!");
}