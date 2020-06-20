//Inclusão de bibliotecas necessárias
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#define ServoPin 14 // GPIO 5
Servo myservo;  

// variaveis de conexão wifi

const char* ssid = "Nome Da Rede Wifi ";
const char* senha = "Senha da rede Wifi";
// MQTT Broker utilizado
const char* mqtt_server = "mqtt.eclipse.org"; 

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Conexão Wifi
void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, senha);

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
// Caso o Wifi caia, faz a reconexão
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("TopicoSaida", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(2000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Inicia a saída de luz padrão do NodeMCU
  myservo.attach(ServoPin);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    int sensorValue = analogRead(A0);
    int pos;
    if (sensorValue >= 500) {
      digitalWrite(BUILTIN_LED, LOW);
      pos = 180;
      myservo.write(pos);
      client.publish("TopicoSaida", "Aberto");
      } else {
        pos = 0;
        myservo.write(pos);
        digitalWrite(BUILTIN_LED, HIGH);
        client.publish("TopicoSaida", "Fechado");
      }
        
    lastMsg = now;
    snprintf (msg, MSG_BUFFER_SIZE, "Luminosidade #%1ad", sensorValue);
    Serial.println(msg);
    client.publish("QtdLuz", msg);
    delay(2000);
    
  }
}
