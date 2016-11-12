#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

const char* ssid = "SD";
const char* password = "kupava555";

const char* robot_name = "robot01";
const char* robot_id = "2";
const char* robot_password = "123456";
const char* robot_subscribe = "chernobyl/robot01/#";

const char* host = "dench.ddns.net";
int port = 1883;

// Camera motor
const int pinA1 = 4; // black 4
const int pinA2 = 2; // brown 2 
const int pinB1 = 0; // yellow 0
const int pinB2 = 15; // blue 15
const float koef = 15.357; // 15.357
float pos = 0;
int flag = 0;
bool flag2 = false;
bool flag3 = false;
AccelStepper stepper(4, pinA1, pinA2, pinB1, pinB2);

// Main motor
int RPWM1 = 13;
int LPWM1 = 12;
int RPWM2 = 14;
int LPWM2 = 16;

Ticker timer;
WiFiClient ethClient;
PubSubClient client(ethClient);

void callback(char* topic, byte* payload, unsigned int length)
{
  if ((char)payload[0] == 'm')
  {
    timer.detach();
    
    int val = payload[2] - '0';
    int l = map(constrain(val, 0, 4), 0, 4, 1023, 0);
    int r = map(constrain(val, 4, 8), 4, 8, 0, 1023);
    analogWrite(LPWM1, l);
    analogWrite(RPWM1, r);

    val = payload[3] - '0';
    l = map(constrain(val, 0, 4), 0, 4, 1023, 0);
    r = map(constrain(val, 4, 8), 4, 8, 0, 1023);
    analogWrite(LPWM2, l);
    analogWrite(RPWM2, r);

    timer.once(1, moveStop);
  }

  if ((char)payload[0] == 'c')
  {
    int val = payload[2] - '0';
    if (val == 0) {
      camCalibrate();
    } else {
      camMove(val);
    }
  }
}

void moveStop()
{
  analogWrite(LPWM1, 0);
  analogWrite(RPWM1, 0);
  analogWrite(LPWM2, 0);
  analogWrite(RPWM2, 0);
}

void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(robot_name, robot_id, robot_password)) {
      Serial.println("connected");
      client.publish("connected", robot_name);
      client.subscribe(robot_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}



void setup()
{
  Serial.begin(115200);
  
  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);

  moveStop();

  client.setServer(host, port);
  client.setCallback(callback);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  stepper.setMaxSpeed(500);
  stepper.setSpeed(500);
  stepper.setAcceleration(1000);
  camCalibrate();
  
  delay(500);
}

void loop()
{
  stepper.run();
  if (!flag) {
    if (!stepper.distanceToGo() && !flag2) {
      if (pos == 0) {
        if (flag3) {
          pos = -0.1;
        } else {
          pos = 2.9*koef;
        }
        stepper.moveTo(pos);
      } else {
        flag2 = true;
        Serial.println("distanceToGo");
        stepper.disableOutputs();
        Serial.println("disableOutputs");
      }
    }
    if (!client.connected()) {
      flag = 0;
      flag2 = false;
      flag3 = false;
      reconnect();
    }
    client.loop();
  }
}

void camCalibrate() {
  if (flag == 0) {
    stepper.enableOutputs();
    Serial.println("enableOutputs");
    stepper.move(285*koef);
    flag3 = 1;
    flag = 1;
    timer.attach(10, camCalibrate);
  } else if (flag == 1) {
    stepper.move(-147*koef);
    flag3 = 0;
    flag = 2;
    pos = 1;
    timer.attach(5, camCalibrate);
  } else if (flag == 2) {
    flag = 0;
    stepper.setCurrentPosition(0);
    stepper.disableOutputs();
    Serial.println("disableOutputs");
    timer.detach();
  }
}

void camMove(int n) {
  stepper.enableOutputs();
  Serial.println("enableOutputs");
  if (n == 1) {
    pos = -133*koef;
    flag3 = 0;
  } else if (n == 2) {
    pos = -90*koef;
    flag3 = 0;
  } else if (n == 3) {
    pos = -35*koef;
    flag3 = 0;
  } else if (n == 4) {
    pos = 0;
  } else if (n == 5) {
    pos = 35*koef;
    flag3 = 1;
  } else if (n == 6) {
    pos = 90*koef;
    flag3 = 1;
  } else if (n == 7) {
    pos = 140*koef;
    flag3 = 1;
  }
  stepper.moveTo(pos);
  flag2 = false;
}
