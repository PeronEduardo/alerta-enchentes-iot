/*
 * ============================================================
 *  Sistema IoT de Alerta de Enchentes - Estação de Monitoramento do Rio
 *  Disciplina: Internet das Coisas (IoT) - UNIVAP
 *  Prof. Wagner dos Santos Clementino de Jesus
 *  Grupo: Eduardo Peron, Diogo Gil de Oliveira, Gustavo Albano Nunes
 * ============================================================
 *
 *  Fluxo (ciclo da IoT: Percepção -> Transmissão -> Análise -> Ação):
 *    1. PERCEPÇÃO   : HC-SR04 mede a distância até a água; DHT22 mede temp/umidade
 *    2. ANÁLISE     : ESP32 converte distância em nível e classifica o risco
 *    3. AÇÃO LOCAL  : LEDs e sirene (buzzer) reagem mesmo SEM internet (Edge Computing)
 *    4. TRANSMISSÃO : envia telemetria em JSON via MQTT/TLS para o ThingsBoard Cloud
 *    5. AÇÃO REMOTA : recebe comando RPC do dashboard para ligar/desligar a sirene
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include "secrets.h"   // credenciais (NÃO vai para o GitHub) - veja secrets_exemplo.h

// ---------------- Pinos ----------------
const int PINO_TRIG     = 5;
const int PINO_ECHO     = 18;
const int PINO_DHT      = 15;
const int PINO_LED_VERDE    = 25;
const int PINO_LED_AMARELO  = 26;
const int PINO_LED_VERMELHO = 27;
const int PINO_BUZZER   = 13;

// ---------------- Parâmetros físicos ----------------
// Altura do sensor em relação ao fundo do rio (cm)
const float ALTURA_SENSOR_CM = 400.0;

// Limites de nível (cm) para cada estado de risco
const float LIMITE_ATENCAO    = 200.0;
const float LIMITE_ALERTA     = 280.0;
const float LIMITE_EMERGENCIA = 340.0;

// ---------------- Estados de risco ----------------
enum EstadoRisco { NORMAL, ATENCAO, ALERTA, EMERGENCIA };
const char* NOMES_ESTADO[] = { "NORMAL", "ATENCAO", "ALERTA", "EMERGENCIA" };

// ---------------- Tópicos MQTT do ThingsBoard ----------------
const char* TOPICO_TELEMETRIA = "v1/devices/me/telemetry";
const char* TOPICO_RPC_REQ    = "v1/devices/me/rpc/request/+";
const char* PREFIXO_RPC_RESP  = "v1/devices/me/rpc/response/";

// ---------------- Objetos globais ----------------
WiFiClientSecure clienteSeguro;          // camada TLS (criptografia)
PubSubClient     mqtt(clienteSeguro);    // MQTT rodando por cima do TLS
DHTesp           dht;

bool sireneRemota = false;               // controlada pelo dashboard (RPC)
EstadoRisco estadoAtual   = NORMAL;
EstadoRisco estadoAnterior = NORMAL;

unsigned long ultimoEnvio       = 0;
unsigned long ultimaLeitura     = 0;
unsigned long ultimaTentativaMqtt = 0;

// Últimas leituras (usadas no envio)
float distanciaCm = 0, nivelCm = 0, temperatura = 0, umidade = 0;

// ============================================================
//  1) PERCEPÇÃO: leitura dos sensores
// ============================================================

// Mede a distância (cm) até a superfície da água.
// d = (tempo * velocidade do som) / 2   -> divide por 2 porque o som vai e volta
float medirDistanciaCm() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);         // pulso de 10 µs dispara o sensor
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);

  unsigned long duracaoUs = pulseIn(PINO_ECHO, HIGH, 30000);  // timeout 30 ms
  if (duracaoUs == 0) return -1;         // sem eco = leitura inválida

  return (duracaoUs * 0.0343) / 2.0;     // 0,0343 cm/µs = 343 m/s
}

// ============================================================
//  2) ANÁLISE: processamento dos dados
// ============================================================

// nível = altura do sensor - distância até a água
float calcularNivelCm(float distancia) {
  float nivel = ALTURA_SENSOR_CM - distancia;
  return constrain(nivel, 0, ALTURA_SENSOR_CM);
}

EstadoRisco classificarRisco(float nivel) {
  if (nivel >= LIMITE_EMERGENCIA) return EMERGENCIA;
  if (nivel >= LIMITE_ALERTA)     return ALERTA;
  if (nivel >= LIMITE_ATENCAO)    return ATENCAO;
  return NORMAL;
}

// Eficiência energética: rio calmo -> envia pouco; risco alto -> envia rápido
unsigned long intervaloEnvioMs(EstadoRisco estado) {
  switch (estado) {
    case NORMAL:     return 60000;   // 60 s
    case ATENCAO:    return 15000;   // 15 s
    case ALERTA:     return 5000;    //  5 s
    case EMERGENCIA: return 2000;    //  2 s
  }
  return 60000;
}

// ============================================================
//  3) AÇÃO LOCAL: LEDs e sirene
// ============================================================
void atualizarSaidas(EstadoRisco estado) {
  digitalWrite(PINO_LED_VERDE,    estado == NORMAL);
  digitalWrite(PINO_LED_AMARELO,  estado == ATENCAO);
  digitalWrite(PINO_LED_VERMELHO, estado == ALERTA || estado == EMERGENCIA);

  // Sirene: liga automaticamente na EMERGÊNCIA ou por comando remoto
  if (estado == EMERGENCIA || sireneRemota) {
    tone(PINO_BUZZER, 1000);
  } else {
    noTone(PINO_BUZZER);
  }
}

// ============================================================
//  4) TRANSMISSÃO: Wi-Fi + MQTT/TLS (com reconexão automática)
// ============================================================
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" conectado! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" falhou (tentará de novo).");
  }
}

void conectarMQTT() {
  // Tenta no máximo a cada 5 s, sem travar o loop (os LEDs continuam funcionando)
  if (millis() - ultimaTentativaMqtt < 5000) return;
  ultimaTentativaMqtt = millis();

  Serial.print("Conectando ao ThingsBoard (MQTT/TLS)... ");
  // No ThingsBoard, o ACCESS TOKEN do dispositivo é usado como "usuário" do MQTT
  if (mqtt.connect("estacao-rio-01", TB_TOKEN, NULL)) {
    Serial.println("ok!");
    mqtt.subscribe(TOPICO_RPC_REQ);      // passa a ouvir comandos do dashboard
  } else {
    Serial.print("falhou, código = ");
    Serial.println(mqtt.state());
  }
}

void enviarTelemetria() {
  char json[200];
  snprintf(json, sizeof(json),
           "{\"nivel_cm\":%.1f,\"distancia_cm\":%.1f,\"temperatura\":%.1f,"
           "\"umidade\":%.1f,\"estado\":\"%s\",\"sirene\":%s}",
           nivelCm, distanciaCm, temperatura, umidade,
           NOMES_ESTADO[estadoAtual],
           (estadoAtual == EMERGENCIA || sireneRemota) ? "true" : "false");

  mqtt.publish(TOPICO_TELEMETRIA, json);
  Serial.print("Enviado: ");
  Serial.println(json);
}

// ============================================================
//  5) AÇÃO REMOTA: comandos RPC vindos do dashboard
//  Exemplo recebido: {"method":"setSirene","params":true}
// ============================================================
void aoReceberMensagem(char* topico, byte* payload, unsigned int tamanho) {
  String msg;
  for (unsigned int i = 0; i < tamanho; i++) msg += (char)payload[i];
  Serial.print("RPC recebido: ");
  Serial.println(msg);

  // O id da requisição é o final do tópico: .../rpc/request/{id}
  String topicoStr = String(topico);
  String idRequisicao = topicoStr.substring(topicoStr.lastIndexOf('/') + 1);
  String topicoResposta = String(PREFIXO_RPC_RESP) + idRequisicao;

  if (msg.indexOf("\"setSirene\"") >= 0) {
    sireneRemota = (msg.indexOf("true") >= 0);
    atualizarSaidas(estadoAtual);
  }
  // Responde com o estado atual (o widget "switch" usa isso no getSirene)
  mqtt.publish(topicoResposta.c_str(), sireneRemota ? "true" : "false");
}

// ============================================================
//  setup() e loop()
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_AMARELO, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);

  dht.setup(PINO_DHT, DHTesp::DHT22);

  // TODO (segurança): trocar setInsecure() por setCACert(CA_THINGSBOARD)
  // setInsecure() já CRIPTOGRAFA os dados, mas não verifica a identidade do servidor.
  clienteSeguro.setInsecure();
  clienteSeguro.setHandshakeTimeout(30);
  mqtt.setSocketTimeout(30);
  mqtt.setKeepAlive(60);

  mqtt.setServer(TB_SERVIDOR, TB_PORTA);
  mqtt.setCallback(aoReceberMensagem);

  conectarWiFi();
}

void loop() {
  // --- Reconexão automática (requisito de Confiabilidade) ---
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  // --- Leitura e processamento a cada 1 s (funciona mesmo offline) ---
  if (millis() - ultimaLeitura >= 1000) {
    ultimaLeitura = millis();

    float d = medirDistanciaCm();
    if (d > 0) {
      distanciaCm = d;
      nivelCm = calcularNivelCm(d);
    }

    TempAndHumidity th = dht.getTempAndHumidity();
    if (dht.getStatus() == 0) {
      temperatura = th.temperature;
      umidade = th.humidity;
    }

    estadoAnterior = estadoAtual;
    estadoAtual = classificarRisco(nivelCm);
    atualizarSaidas(estadoAtual);
  }

  // --- Envio: no intervalo adaptativo OU imediatamente se o estado mudou ---
  bool mudouEstado = (estadoAtual != estadoAnterior);
  if (mqtt.connected() &&
      (mudouEstado || millis() - ultimoEnvio >= intervaloEnvioMs(estadoAtual))) {
    ultimoEnvio = millis();
    estadoAnterior = estadoAtual;
    enviarTelemetria();
  }
}