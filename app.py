from flask import Flask, render_template, jsonify, json
from flask_mqtt import Mqtt
import cx_Oracle
from SqlQuery import user_conn, check, create_tables

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = '192.168.0.8'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_REFRESH_TIME'] = 1.0

mqtt = Mqtt(app)

sensor_data = []

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe("Sensor")

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    payload = message.payload.decode()
    payload_json = json.loads(payload)  # Convert payload to JSON object
    sensor_data.append(payload_json)  # Append JSON object to sensor_data list
    if len(sensor_data) > 10:
        sensor_data.pop(0)

@app.route('/')
def index():
    return render_template('index.html', sensor_data=sensor_data)

@app.route('/update-data')
def update_data():
    return jsonify(sensor_data[-1] if sensor_data else 0)

create_tables()
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
