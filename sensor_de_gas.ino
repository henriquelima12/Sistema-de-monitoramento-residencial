#include <ESP8266WiFi.h> //inclui suporte ao NodeMCU
#include <PubSubClient.h> //inclui suporte ao MQTT no HiveMQ Cloud

const char* ssid = "Henrique2G"; //SSID da rede WiFi
const char* password = "GHTA2122"; //senha da rede WiFi

const char* mqtt_server = "broker.mqtt-dashboard.com"; //URL do broker MQTT
const int mqtt_server_port = 1883; //porta do broker MQTT

#define MSG_BUFFER_SIZE (500) //define MSG_BUFFER_SIZE como 500
WiFiClient client; //cria o objeto client
PubSubClient mqtt_client(client); //cria o objeto mqtt_client
long lastMsg = 0;

String clientID = "ESP8266Client-"; //identificacao do cliente

String topicoPrefixo = "MACK32091702"; //para o prefixo do topico, utilizar MACK seguido do TIA
String topicoTodos = topicoPrefixo + "/#"; //para retornar todos os topicos
String topico_0 = topicoPrefixo + "/hello"; //topico para teste
String mensagem_0 = "NodeMCU Connected"; //mensagem para o topico 0
String topico_1 = topicoPrefixo + "/sensor"; //topico para o sensor 1
String mensagem_1 = ""; //mensagem para o topico 1
String topico_2 = topicoPrefixo + "/gasvalue";
String mensagem_2 = "";
String mensagemTemp = ""; //armazena temporariamente as mensagens recebidas via MQTT

int pinValue = 0; 

int sensor = A0;
int buzzer = D5;

// Funcao para configurar a conexao WiFi
void setup_wifi() {
  //WiFi.mode(WIFI_STA); //ativa o modo de estacao
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

// Funcao para receber mensagens
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagemTemp = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    mensagemTemp += (char)payload[i];
  }

  Serial.println();
}


// Funcao para conectar no broker MQTT
void reconnect() {
  // Loop until we’re reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection…");

  // Create a random client ID
    randomSeed(micros()); //inicializa a semente do gerador de numeros aleatorios
    clientID += String(random(0xffff), HEX);

  // Attempt to connect
    if (mqtt_client.connect(clientID.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish(topico_0.c_str(), mensagem_0.c_str());
      // ... and resubscribe
      mqtt_client.subscribe(topicoTodos.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(9600); //inicializa a saida serial
  pinMode(sensor, INPUT);
  pinMode(buzzer, OUTPUT);
  setup_wifi();

  mqtt_client.setServer(mqtt_server, mqtt_server_port); //configura a conexao com o broker MQTT
  mqtt_client.setCallback(callback); //configura o recebimento de mensagens

}

void loop() {

  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop(); //processa as mensagens e mantem a conexao com o broker MQTT

  pinValue = analogRead(sensor);
  Serial.print("Leitura de gás: ");
  Serial.println(pinValue);

   if(pinValue > 500) {
    digitalWrite(buzzer, HIGH);
    Serial.println("Perigo! Nível de gás acima do normal");
    mensagem_1 = "Perigo! Nível de gás acima do normal";
    mensagem_2 = pinValue; 
   }
  
  if(pinValue <= 500) {
      digitalWrite(buzzer, LOW);
      Serial.println("Nível de gás dentro do normal");
      mensagem_1 = "Nível de gás dentro do normal";
      mensagem_2 = pinValue; 
  }

  delay(50); //espera para evitar efeito bouncing

  //Publica mensagem
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    mqtt_client.publish(topico_1.c_str(), mensagem_1.c_str());
    mqtt_client.publish(topico_2.c_str(), mensagem_2.c_str());
  }

}