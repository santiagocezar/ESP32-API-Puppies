#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

int sensorPin = 32;   // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "santiagocezar/rgblitz";
const char *topic_out = "santiagocezar/rgblitz-out";
const char *mqtt_username = "santiagocezar";
const char *mqtt_password = "rgblitz0401";
const int mqtt_port = 1883;

WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

void mqttSetup(void) {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += WiFi.macAddress();
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.publish(topic, "juego para el MUIC");
  client.subscribe(topic);
}

void callback(char *topic, byte *data, unsigned int length) {
  byte command = data[0];

  switch (command) {
    case HELLO: break;
    case SET_LED:
      if (length == 8) {
        float d = distanceRGB(data[5], data[6], data[7], guessR, guessG, guessB);

        Serial.printf("distance: %.2f\n", d);

        setLed(LED1, data[5], data[6], data[7]);
        if (d > closest) {
          // setLed(LED1, data[5], data[6], data[7]);
          closest = d;
          byte* res = (byte*)malloc(6);
          memcpy(res, data, 6);
          res[0] = CLOSEST_CLIENT;
          res[5] = (int)d;
          client.publish(topic_out, res, 6);
          free(res);
        }
        if (d > 80) {
          randomize();
        }
      }
      break;
  }
  //setLed(data[0] == 1 ? LED1 : LED2, data[1], data[2], data[3]);
}

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);

  wifiMulti.addAP("UTN_FRSFCO_LIBRE");
  wifiMulti.addAP("WifiCR", "CR2546938");
  wifiMulti.addAP("Estudiantes", "educar_2018");

  if (wifiMulti.run() == WL_CONNECTED) {
    // GOOD ENDING: el wifi anda :D
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("BSSID: ");
    Serial.println(WiFi.BSSIDstr());
    Serial.print("Canal: ");
    Serial.println(WiFi.channel());

    mqttSetup();
  }
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  // turn the ledPin on
  Serial.printf("valor potenciometro: %03.2f%\n", sensorValue / 4095.0 * 100.0);
  
  client.loop();
}
