//============== Declaração de Bibliotecas ====================
// Biblioteca para criar mensagem Json.
#include <ArduinoJson.h>

// Biblioteca para enviar dados MQTT.
#include <PubSubClient.h>

// Biblioteca do NTP.
#include <NTPClient.h>

// Biblioteca do UDP.
#include <WiFiUdp.h>

// Biblioteca do WiFi.
#include <WiFi.h>

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

//===============SENSOR DE VAZÃO===============
//DEFININDO PORTA DE SINAL PARA O SENSOR DE VAZÃO
const int portaVazao = GPIO_NUM_35;

//VARIÁVEIS PARA CÁLCULO DE VAZÃO 
static void atualizaVazao();
volatile int pulsosVazao = 0;
double calculoDaVazao;

// INTERRUPÇÃO NA CONTAGEM DE UM PULSO
void IRAM_ATTR gpio_isr_handler_up(void* arg)
{
  pulsosVazao++;
  portYIELD_FROM_ISR();
}

void iniciaVazao(gpio_num_t Port){
  gpio_set_direction(Port, GPIO_MODE_INPUT);
  gpio_set_intr_type(Port, GPIO_INTR_NEGEDGE);
  gpio_set_pull_mode(Port, GPIO_PULLUP_ONLY);
  gpio_intr_enable(Port);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(Port, gpio_isr_handler_up, (void*) Port);
}

void setup()
{
  //INICIAR PORTA SERIAL
  Serial.begin(115200);

  //INICIAR PINO DO SENSOR DE VAZAO
  iniciaVazao((gpio_num_t) portaVazao);

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
  
void loop() {

  // Conecta-se ao broker MQTT.
  if ( !client.connected() ) {
   reconnect();
   }

  // Mantém conexão MQTT aberta.
  client.loop();
   
  // Atualiza novas medidas a cada 1s.
  if ( millis() - last_time > 1000 ) {
    

  //TRANSFORMAÇÃO DOS PULSOS PARA CORRESPONDENTE VAZÃO
  calculoDaVazao = pulsosVazao * 2.25;
  
  //VAZÃO POR MINUTO
  calculoDaVazao = calculoDaVazao * 60;
  calculoDaVazao = calculoDaVazao / 1000;

  pulsosVazao = 0;

  // Obtém horário em épocas.
  ntp.update();
  hour = (int) ntp.getEpochTime();

  // Atualiza diferença entre horários em segundos.
  if(last_hour != hour){
    diff_hour = hour - last_hour;
    last_hour = hour;
    }
    
  //Envia dados do sensor ao Broker MQTT no formato Json
    StaticJsonDocument<200> datas;
    datas["Litros por minuto"] = calculoDaVazao;
    datas["Hour"] = hour;
    datas["Diff_hour"] = diff_hour;
    String payload;
    serializeJson(datas, payload);

  // Publica mensagem MQTT
    client.publish("flowmeter/send", payload.c_str());
    last_time = millis();
   }
}

  //REALIZAR O PRINT DA LEITURA NO SERIAL
  //Serial.println("Litros por minutos: ");
  //Serial.println(calculoDaVazao);

  //REALIZAR UM DELAY E INICIALIZAR LEITURA DAQUI A 1 SEGUNDO
  //delay(1000);

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
