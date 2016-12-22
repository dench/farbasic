/*
 * Монтирование файловой системы
 */
void initFileSystem()
{
  SPIFFS.begin();
}

/*
 * Инициализация пинов моторов
 */
void initMotor()
{
  pinMode(M1R, OUTPUT);
  pinMode(M1L, OUTPUT);
  pinMode(M2R, OUTPUT);
  pinMode(M2L, OUTPUT);

  moveStop();
}

/*
 * Инициализация MQTT client
 */
void initClient()
{
  String port = readConfig("server_port");

  #if defined DEBUG
    Serial.println("Init client");
    Serial.println(MQTT_SERVER);
    Serial.println(port.toInt());
  #endif
  
  client.setServer(MQTT_SERVER, port.toInt());
  client.setCallback(callback);
}

/*
 * Конфигурация Wi-Fi
 */
void initWiFi()
{
  String ssid = readConfig("my_ssid");
  String password = readConfig("my_password");

  #if defined DEBUG
    Serial.print("SSID: ");
    Serial.println(ssid);
  #endif

  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if defined DEBUG
      Serial.print(".");
    #endif
  }

  #if defined DEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}

/* 
 *  Остановка моторов
 */
void moveStop()
{
  analogWrite(M1R, 0);
  analogWrite(M1L, 0);
  analogWrite(M2R, 0);
  analogWrite(M2L, 0);
}

/*
 * Прослушка MQTT и подключение, если соединение было сброшено
 */
void clientLoop()
{
  while (!client.connected()) {
    
    String robot_name = readConfig("robot_name");
    String robot_id = readConfig("robot_id");
    String robot_password = readConfig("robot_password");
    String robot_subscribe = readConfig("robot_subscribe");

    robot_name += "-" + String(random(0xffff), HEX);

    #if defined DEBUG
      Serial.println(robot_name);
      Serial.println(robot_id);
      Serial.println(robot_password);
      Serial.println(robot_subscribe);
      Serial.print("Attempting MQTT connection...");
    #endif
    
    if (client.connect(robot_name.c_str(), robot_id.c_str(), robot_password.c_str())) {
      #if defined DEBUG
        Serial.println("connected");
      #endif
      client.publish("connected", robot_name.c_str());
      client.subscribe(robot_subscribe.c_str());
    } else {
      #if defined DEBUG
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      #endif
      delay(5000);
    }
  }

  client.loop();
}

/**
 * Чтение файла
 * 
 * @param key String - путь к файлу
 * @return String
 */
String readFile(String filename)
{
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    #if defined DEBUG
      Serial.print("Failed open file ");
      Serial.println(filename);
    #endif
    return "";
  }
  String text = "";
  while(file.available()) {
    text += file.readStringUntil('\n') + '\n';
  }
  text.trim();
  return text;
}

/**
 * Запись в файла
 * 
 * @param filename String - путь к файлу
 * @param data String - данные
 * @return bool
 */
bool writeFile(String filename, String data)
{
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    #if defined DEBUG
      Serial.print("Failed open file ");
      Serial.println(filename);
    #endif
    return false;
  }

  file.println(data);
  
  return true;
}

/**
 * Чтение конфигурации
 * 
 * @param key String - название конфигурации
 * @return String
 */
String readConfig(String key)
{
  return readFile("/config/" + key + ".cfg");
}
