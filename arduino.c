#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include "time.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
// hora 
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // Brasil UTC-3
const int daylightOffset_sec = 0;

// ======== Wi-Fi =========
const char* WIFI_SSID = "alvesrosado";
const char* WIFI_PASS = "hulk@2005";

// Servidor MQTT
const char* mqtt_broker = "9bda570589c64d8cb4d66bac489fa7f7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "Projeto_Agro";
const char* mqtt_password = "Pegasus@24";

// ======== DHT22 =========
#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======== Sensor Solo =========
#define SOIL_PIN 34
int soloSeco = 3170;     // calibrado no seco
int soloMolhado = 1485;  // calibrado no molhado

// ======== Servidor Web =========
WebServer server(80);

// HTML com atualização automática via fetch()
const char* HTML = R"HTML(
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Monitor ESP32</title>
<style>
  :root { font-family: system-ui, Arial, sans-serif; }
  body { max-width: 560px; margin: 40px auto; padding: 0 16px; }
  h1 { margin: 0 0 16px; color: #2c3e50; }
  .card { border: 1px solid #e5e7eb; border-radius: 12px; padding: 16px 20px; box-shadow: 0 2px 12px rgba(0,0,0,.04); }
  .row { display: flex; justify-content: space-between; margin: 10px 0; font-size: 18px; }
  .label { color: #6b7280; }
  .value { font-weight: 600; }
  .ok { color: #10b981; }
  .bad { color: #ef4444; }
  .muted { color: #9ca3af; font-size: 14px; }
</style>
</head>
<body>
  <h1>Monitor de Sensores — ESP32</h1>
  <div class="card">
    <div class="row"><span class="label">Temperatura</span> <span id="temp" class="value">--.- °C</span></div>
    <div class="row"><span class="label">Umidade do ar</span> <span id="uar" class="value">--.- %</span></div>
    <div class="row"><span class="label">Umidade do solo</span> <span id="usolo" class="value">-- %</span></div>
    <div class="row"><span class="label">Status</span> <span id="status" class="value muted">conectando...</span></div>
    <div class="muted" id="ts">—</div>
  </div>

<script>
async function atualizar() {
  try {
    const r = await fetch('/data', {cache: 'no-store'});
    if (!r.ok) throw new Error('HTTP ' + r.status);
    const j = await r.json();

    // atualiza UI
    document.getElementById('temp').textContent = (j.temperatura ?? NaN).toFixed(1) + ' °C';
    document.getElementById('uar').textContent  = (j.umidade_ar ?? NaN).toFixed(1) + ' %';
    document.getElementById('usolo').textContent= (j.umidade_solo ?? 0) + ' %';
    document.getElementById('status').textContent = 'online';
    document.getElementById('status').className  = 'value ok';
    document.getElementById('ts').textContent = 'Atualizado: ' + new Date(j.timestamp * 1000).toLocaleString();
  } catch (e) {
    document.getElementById('status').textContent = 'offline';
    document.getElementById('status').className  = 'value bad';
  }
}

// primeira carga + atualização a cada 2s
atualizar();
setInterval(atualizar, 2000);
</script>
</body>
</html>
)HTML";

// gera JSON com as leituras atuais
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


void handleRoot() {
  server.send(200, "text/html; charset=utf-8", HTML);
}

void handleData() {
  server.send(200, "application/json", leituraJSON());
}

void setup_wifi(){
    // Wi-Fi
  Serial.print("Conectando no Wi-Fi ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\n Conectado!");
  Serial.print("Acesse: http://");
  Serial.println(WiFi.localIP());

  // rotas
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Servidor web iniciado ");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("conectado!");
      client.subscribe("agro/arthur1/cmd"); // exemplo de tópico
    } else {
      Serial.print("falha rc=");
      Serial.print(client.state());
      Serial.println(" tentando em 5s");
      delay(5000);
    }
  }
}

void publicaTelemetria() {
  String payload = leituraJSON();
  client.publish("agro/sensores/telemetria/esp32_01", payload.c_str());
  Serial.println("Publicado em telemetria: " + payload);
}
void conectaMQTT() {
  while (!client.connected()) {
    if (client.connect("esp32_01", mqtt_username, mqtt_password,
                       "agro/sensores/status/esp32_01",  // tópico LWT
                       1, true, "offline")) {            // QoS 1, retain
      client.publish("agro/sensores/status/esp32_01", "online", true);
      client.subscribe("agro/sensores/cmd/esp32_01");
    } else {
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("Recebido em [%s]: %s\n", topic, msg.c_str());

  if (String(topic) == "agro/sensores/cmd/esp32_01") {
    // exemplo: {"soil_target":70}
    if (msg.indexOf("soil_target") >= 0) {
      int valor = msg.substring(msg.indexOf(":") + 1).toInt();
      Serial.printf("Novo alvo de solo = %d%%\n", valor);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi(); // executa a funcao de wifi
  dht.begin(); // comeca as leituras no dht22
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  espClient.setInsecure(); // ignora verificação de certificado TLS
  client.setServer(mqtt_broker, mqtt_port);

}

void loop() {
  // Testar se a conexao com o servidor mqtt esta funcinando, se cair ele tenta reconectar
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  server.handleClient();
   static unsigned long lastPub = 0;
  if (millis() - lastPub > 5000) { // a cada 5s
    lastPub = millis();
    publicaTelemetria();
  }
}
