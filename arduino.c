#include <PubSubClient.h>
#include <WiFi.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include "time.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // Brasil UTC-3
const int daylightOffset_sec = 0;

const char* WIFI_SSID = "alvesrosado";
const char* WIFI_PASS = "hulk@2005";

const char* mqtt_broker = "9bda570589c64d8cb4d66bac489fa7f7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "Projeto_Agro";
const char* mqtt_password = "Pegasus@24";

#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN 34
int soloSeco = 3170;     // calibrado no seco
int soloMolhado = 1485;  // calibrado no molhado


String getDateTime();

String leituraJSON() {
  float t = dht.readTemperature();
  float uar = dht.readHumidity();
  int cru = analogRead(SOIL_PIN);
  int usolo = map(cru, soloSeco, soloMolhado, 0, 100);
  usolo = constrain(usolo, 0, 100);

  String horario = getDateTime();

  String json = "{";
  json += "\"temperatura\":" + String(isnan(t)?0:t, 2) + ",";
  json += "\"umidade_ar\":" + String(isnan(uar)?0:uar, 2) + ",";
  json += "\"umidade_solo\":" + String(usolo) + ",";
  json += "\"horario\":\"" + horario + "\"";
  json += "}";

  return json;
}

String getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "00/00/0000 00:00:00";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

void setup_wifi(){
  Serial.print("Conectando no Wi-Fi ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.localIP());


  Serial.println("Servidor web iniciado");
}

void conectaMQTT() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("esp32_01", mqtt_username, mqtt_password,
                       "agro/sensores/status/esp32_01",
                       1, true, "offline")) {
      Serial.println("Conectado ao broker!");
      client.publish("agro/sensores/status/esp32_01", "online", true);
      client.subscribe("agro/sensores/cmd/esp32_01");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s...");
      delay(180000);
    }
  }
}

void publicaTelemetria() {
  String payload = leituraJSON();
  client.publish("agro/sensores/telemetria/esp32_01", payload.c_str());
  Serial.println("Publicado em telemetria: " + payload);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("Recebido em [%s]: %s\n", topic, msg.c_str());

  if (String(topic) == "agro/sensores/cmd/esp32_01") {
    if (msg.indexOf("soil_target") >= 0) {
      int valor = msg.substring(msg.indexOf(":") + 1).toInt();
      Serial.printf("Novo alvo de solo = %d%%\n", valor);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  espClient.setInsecure(); 
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sincronizando hora via NTP...");
}

void loop() {
  if (!client.connected()) {
    conectaMQTT();
  }
  client.loop();
  static unsigned long lastPub = 0;
  if (millis() - lastPub > 180000) {
    lastPub = millis();
    publicaTelemetria();
  }
}
