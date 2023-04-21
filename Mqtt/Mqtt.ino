#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define seaLevelPressure_hPa 1013.25

Adafruit_BMP085 bmp;

const char* ssid = "homeless";
const char* password = "ausoleto12";
const char* mqtt_server = "192.168.0.8";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (128)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int photo = A0;
int count = 0;

//timestamp
const char *timestampFormat = "HH:MM:SS";
String getFormattedTimestamp() {
  unsigned long currentTime = millis();
  
  // Calculate hours, minutes, seconds, and milliseconds
  int hours = (currentTime / (1000 * 60 * 60)) % 24;
  int minutes = (currentTime / (1000 * 60)) % 60;
  int seconds = (currentTime / 1000) % 60;
  int milliseconds = currentTime % 1000;

  // Create the timestamp string
  String timestamp = String(timestampFormat);
  timestamp.replace("HH", String(hours));
  timestamp.replace("MM", String(minutes));
  timestamp.replace("SS", String(seconds));
  timestamp.replace("sss", String(milliseconds));

  return timestamp;
}
//조도센서
int Photo()
{
  int cds = analogRead(photo);
  int cdspercent = map(cds, 0, 1023, 0, 100);
  return cdspercent;
}
// 센서값 json 직렬화(json 형태로 변환)
void MyJson(String time, float temp, float pres,float light)
{
  DynamicJsonDocument  doc(300); 
  doc["label"]=time;
  //DHT
  doc["Dht11"][0] = temp;
  doc["Dht11"][1] = pres;
  //doc["Temp"]=temp;
  //doc["Pres"]=pres;
  //CDS
  doc["Solar"]=light;

  Serial.print("Json Data :");
  serializeJson(doc,Serial);
  Serial.println();
  serializeJson(doc, msg);
}
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  /*if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }*/

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!bmp.begin())
  {
    Serial.println("BMP180 Not Found. Check Circuit");
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  int s = Photo();
  float t = bmp.readTemperature();
  float p = bmp.readPressure();
  float rp = p / seaLevelPressure_hPa;
  String time = getFormattedTimestamp();
  MyJson(time,t,rp,s);

  if(!client.connected())
  {
    reconnect(); 
  }
  client.loop();

  long now = millis(); // 현재시간 - 과거시간(시작시간)
  if(now - lastMsg > 2000)
  {
    lastMsg = now;
    Serial.print("Publish message:");
    Serial.print(msg);
    client.publish("Sensor",msg);
  }
  delay(2000);

}
