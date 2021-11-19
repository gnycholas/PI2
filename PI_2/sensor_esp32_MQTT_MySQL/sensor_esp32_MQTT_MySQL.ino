
// teste git comentario

//============== Declaração de Bibliotecas ====================
// Biblioteca para criar mensagem Json.
#include <ArduinoJson.h>

// Biblioteca para enviar dados MQTT.
#include <PubSubClient.h>

// Biblioteca do NTP.
#include <NTPClient.h>

// Biblioteca do UDP.
#include <WiFiUdp.h>

// Biblioteca do WiFi.//
#include <WiFi.h>

// Biblioteca do Sensor de Corrente AC.
#include "EmonLib.h"

//============== Definição de Constantes ======================
#define WIFI_SSID "Nycholas"
#define WIFI_PASS  "Kfi070380"

//============== Criação de Instâncias ========================
// Cria um objeto "UDP".
WiFiUDP udp;

// Cria um objeto "NTP".
NTPClient ntp(udp, "0.br.pool.ntp.org", -3 * 3600, 60000);

// Cria o objeto wifiClient
WiFiClient wifiClient;

// Cria objeto MQTT client.
PubSubClient client(wifiClient);

//============= Declaração de Variáveis Globais ===============
// Armazena o horário obtido do NTP.
int hour;

// Armazena horário da última medida realizada.
int last_hour;

// Armazena diferença em segundos entre duas medidas.
int diff_hour;

// Armazena intervalo de tempo entre medidas.
unsigned long last_time;

// Obtém status de conexão Wifi.
int status = WL_IDLE_STATUS;

//============ Configuração da função void setup() ============
void setup()
{
   //Inicia a comunicação serial.
   Serial.begin(115200);

   // Inicializa parâmetros do sensor de corrente.
   emon1.current(34, CURRENT_CAL);

   // Configura conexão MQTT.
   client.setServer("mqtt.eclipseprojects.io", 1883);
   
   WiFi.begin(WIFI_SSID, WIFI_PASS);
   while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
   }
   // Inicia o cliente NTP.
   ntp.begin();

   // Força o Update.
   ntp.forceUpdate();

   // Armazena primeira medida com a hora atual.
   last_hour = ntp.getEpochTime();
}

//============ Configuração da função void loop() ============
void loop(){

   // Conecta-se ao broker MQTT.
   if ( !client.connected() ) {
    reconnect();
   }

   // Mantém conexão MQTT aberta.
   client.loop();
   
   // Atualiza novas medidas a cada 1s.
   if ( millis() - last_time > 1000 ) {
    
    // Calcula Corrente AC.
    emon1.calcVI(40,500);

    // Obtém valor da corrente calculado.
    float Current = emon1.Irms;

    // Calcula valor aproximado da Potência Aparente.
    float Power = Current * 220.00;

    // Obtém horário em épocas.
    ntp.update();
    hour = (int) ntp.getEpochTime();

    // Atualiza diferença entre horários em segundos.
    if(last_hour != hour){
      diff_hour = hour - last_hour;
      last_hour = hour;
    }

    // Devido as oscilações iniciais, limitamos a corrente em uma margem.
    if(Current < 0.019) {
      Current = 0.00;
      Power = 0.00;
    }

    // Envia dados do sensor ao Broker MQTT no formato Json
    StaticJsonDocument<200> datas;
    datas["Irms"] = Current;
    datas["Power"] = Power;
    datas["Hour"] = hour;
    datas["Diff_hour"] = diff_hour;
    String payload;
    serializeJson(datas, payload);

    // Publica mensagem MQTT
    client.publish("flowmeter/send", payload.c_str());
    last_time = millis();
   } 
}

//============ Configuração da função void reconnect() ============
void reconnect() {
  while (!client.connected()) {
    status = WiFi.status();
    
    // Caso não esteja conectado ao wifi, inicia conexão
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Conectado ao AP");
    }
    Serial.print("Conectando-se ao Broker MQTT ...");
    
    // Inicia conexão MQTT
    if ( client.connect("ESP32 Device") ) {
      
      // Conexão MQTT estabelecida.
      Serial.println( "[MQTT DONE]" );
    } else {
      
      // Conexão MQTT falhou.
      Serial.print( "[MQTT FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      
      // Aguarda 5 segundos antes de se reconectar
      delay( 5000 );
    }
  }
}
