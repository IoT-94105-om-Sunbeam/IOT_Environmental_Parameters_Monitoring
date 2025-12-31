import json
import paho.mqtt.client as mqtt
import mysql.connector
import requests

THINGSPEAK_WRITE_KEY = "BSFNNVWKSVBFXE6E"

db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="root",
    database="environment_monitoring"
)

cursor = db.cursor()

def on_message(client, userdata, msg):

    data = json.loads(msg.payload.decode())
    print("Raw MQTT:", data)

    temperature = data["temperature"]
    humidity = data["humidity"]
    gas = data["gas"]

    print("Parsed -> T:", temperature, " H:", humidity, " Gas:", gas)

    cursor.execute(
        "INSERT INTO env_data (temperature, humidity, gas) VALUES (%s, %s, %s)",
        (temperature, humidity, gas)
    )
    db.commit()
    print("Saved to MySQL ✔")

    url = "https://api.thingspeak.com/update"
    params = {
        "api_key": THINGSPEAK_WRITE_KEY,
        "field1": temperature,
        "field2": humidity,
        "field3": gas
    }
    requests.get(url, params=params)
    print("ThingSpeak updated")


client = mqtt.Client()
client.on_message = on_message

client.connect("10.49.201.155", 1883, 60)

client.subscribe("environment/data")

print("Listening for MQTT data…")
client.loop_forever()

