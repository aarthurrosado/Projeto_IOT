import paho.mqtt.client as mqtt
import csv, json, os
from datetime import datetime

BROKER = "9bda570589c64d8cb4d66bac489fa7f7.s1.eu.hivemq.cloud"
PORT = 8883
USERNAME = "Projeto_Agro"
PASSWORD = "Pegasus@24"
TOPIC = "agro/sensores/telemetria/esp32_01/#"
CSV_FILE = "dados_sensores.csv"

if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "topic"])
    header = ["timestamp", "topic"]
else:
    with open(CSV_FILE, "r", encoding="utf-8") as f:
        reader = csv.reader(f)
        try:
            header = next(reader)
        except StopIteration:
            header = ["timestamp", "topic"]


def salvar_linha(timestamp, topic, dados):
    global header

    novas_colunas = [k for k in dados.keys() if k not in header]
    if novas_colunas:
        header.extend(novas_colunas)

        with open(CSV_FILE, "r", encoding="utf-8") as f:
            linhas = list(csv.reader(f))

        with open(CSV_FILE, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(header)
            for linha in linhas[1:]:
                while len(linha) < len(header):
                    linha.append("")
                writer.writerow(linha)

    linha = []
    for col in header:
        if col == "timestamp":
            linha.append(timestamp)
        elif col == "topic":
            linha.append(topic)
        else:
            linha.append(dados.get(col, ""))

    with open(CSV_FILE, "a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(linha)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao broker")
        client.subscribe(TOPIC)
    else:
        print("Falha na conexão, código:", rc)

def on_disconnect(client, userdata, rc):
    print("Desconectado do broker.")

def on_message(client, userdata, msg):
    try:
        dados = json.loads(msg.payload.decode())
    except:
        dados = {"raw_message": msg.payload.decode()}

    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"{timestamp} | {msg.topic} -> {dados}")

    salvar_linha(timestamp, msg.topic, dados)

client = mqtt.Client(protocol=mqtt.MQTTv311)
client.username_pw_set(USERNAME, PASSWORD)
client.tls_set()

client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_forever()
