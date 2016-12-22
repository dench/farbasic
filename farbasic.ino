#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FS.h>

#define DEBUG true

#define M1R 13
#define M1L 12
#define M2R 14
#define M2L 16

#define MQTT_SERVER "dench.ddns.net"

Ticker timer;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length)
{
  if ((char)payload[0] == 'm')
  {
    timer.detach();
    
    int val = payload[2] - '0';
    int l = map(constrain(val, 0, 4), 0, 4, 870, 0);
    int r = map(constrain(val, 4, 8), 4, 8, 0, 870);
    analogWrite(M1R, r);
    analogWrite(M1L, l);

    val = payload[3] - '0';
    l = map(constrain(val, 0, 4), 0, 4, 1023, 0);
    r = map(constrain(val, 4, 8), 4, 8, 0, 1023);
    analogWrite(M2R, r);
    analogWrite(M2L, l);

    timer.once(1, moveStop);
  }
}

void setup()
{
  #if defined DEBUG
    Serial.begin(115200);
    Serial.println(" ");
  #endif

  initFileSystem();
  
  initMotor();

  initWiFi();

  initClient();
  
  delay(500);
}

void loop()
{
  clientLoop();
}
