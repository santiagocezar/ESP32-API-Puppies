#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "utn-frsfco/comedero";
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

  client.publish(topic, R"({"ok":true})");
  client.subscribe(topic);
}

void callback(char *topic, byte *data, unsigned int length) {
  byte command = data[0];
}

const int platoPin = 32;
const int tanqueEcho = 33;
const int tanqueTrig = 25;
int platoValue = 0;
long tanqueValue = 0;

long getDistance() {
  long duration, distanceCm;

  digitalWrite(tanqueTrig, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(tanqueTrig, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(tanqueTrig, LOW);

  duration = pulseIn(tanqueEcho, HIGH);  // medimos el tiempo entre pulsos, en microsegundos

  // 340 (velocidad del sonido en metros por segundo) / 10⁶ (conversión de µs a s) / 2 (ida y vuelta) × 100 (m a cm)
  // = 0,017 ≈ 1 / 50
  distanceCm = duration / 58;

  // Serial.print("distancia (en cm): ");
  // Serial.println(distanceCm);

  return distanceCm;
}

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(platoPin, INPUT);
  pinMode(tanqueTrig, OUTPUT); //pin como salida
  pinMode(tanqueEcho, INPUT);  //pin como entrada
  digitalWrite(tanqueTrig, LOW);  //pin como entrada
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
  platoValue = analogRead(platoPin);
  tanqueValue = getDistance();

  String json = "{";
  json += "\"plato\":"+String(platoValue / 4095.0 * 100.0)+",";
  json += "\"tanque\":"+String(tanqueValue);
  json += "}";

  client.publish(topic, json.c_str());

  client.loop();

  delay(500);
}
