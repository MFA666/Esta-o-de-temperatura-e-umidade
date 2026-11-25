// =========================================================
// BLOCO 1: BIBLIOTECAS, PINOS E CONFIGURAÇÃO DOS SENSORES
// =========================================================
// Bibliotecas para conectar o ESP32 ao Wi-Fi e criar um servidor web local
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

// Importa o arquivo que guarda o código HTML da página dentro da memória do ESP32 (variável MAIN_page)
#include "index.h"  
WebServer server(80); // Cria o servidor na porta 80 (padrão HTTP)

// Configura o sensor de Temperatura e Umidade (DHT22) conectado no pino digital 4
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configura o sensor ultrassônico (usado aqui para simular um anemômetro/vento) nos pinos 19 e 18
#include <NewPing.h>
#define trigger 19
#define echo 18
#define max_distancia 300 // Distância máxima de leitura em cm
NewPing sonar(trigger, echo, max_distancia);

// =========================================================
// BLOCO 2: CREDENCIAIS WI-FI E ROTAS DO SERVIDOR WEB
// =========================================================
// Nome e senha da rede Wi-Fi onde o ESP32 vai se conectar
const char* ssid = "Rede";
const char* password = "Senha";

// Função chamada quando o usuário acessa o endereço principal (ex: http://192.168.1)
void handleRoot() {
  String s = MAIN_page;             // Lê o HTML armazenado no arquivo index.h
  server.send(200, "text/html", s); // Envia a página completa para o navegador (código 200 = OK)
}

// Função chamada automaticamente pelo JavaScript em segundo plano para buscar dados atualizados
void handleADC() {
  XML_response(); // Gera e envia o XML com as leituras atuais dos sensores
}

// =========================================================
// BLOCO 3: INICIALIZAÇÃO DO SISTEMA (EXECUTA UMA VEZ AO LIGAR)
// =========================================================
void setup() {
  Serial.begin(115200); // Inicia a comunicação serial para exibir mensagens de diagnóstico no PC
  Serial.println();
  Serial.println("Inicializando Sketch...");

  // Configura o ESP32 no modo Estação (STA) para se conectar a um roteador Wi-Fi existente
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  Serial.println("Connecting to ");
  Serial.print(ssid);

  // Trava a execução enquanto o Wi-Fi não conecta, imprimindo pontos na tela
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {      
    Serial.print(".");
  }

  // Exibe no Monitor Serial o IP atribuído ao ESP32 (é esse IP que você digita no navegador)
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Associa os endereços URL às suas respectivas funções de resposta
  server.on("/", handleRoot);       // Rota principal: entrega o visual (HTML)
  server.on("/readADC", handleADC); // Rota de dados: entrega os valores dos sensores (XML)

  server.begin(); // Liga o servidor web
  Serial.println("HTTP server started");
  dht.begin();    // Inicializa o sensor DHT22
}

// =========================================================
// BLOCO 4: LOOP PRINCIPAL (EXECUTA CONTINUAMENTE)
// =========================================================
void loop() {   
  server.handleClient(); // Fica escutando e respondendo às requisições feitas pelo navegador
  delay(1);              // Pequena pausa de 1ms para estabilidade do processador
}

// =========================================================
// BLOCO 5: LEITURA DOS SENSORES E ENVIO EM XML
// =========================================================
void XML_response() {
  // 1. Lê temperatura (°C) e umidade (%) do sensor DHT22
  byte temperature = dht.readTemperature();
  byte humidity = dht.readHumidity();

  // 2. Faz 5 leituras rápidas no ultrassônico, tira a mediana para evitar ruído e converte para cm (simula o vento)
  unsigned int distancia = sonar.convert_cm(sonar.ping_median(5));

  // 3. Estima a probabilidade de chuva (0% a 100%) proporcionalmente à umidade entre 40% e 95%
  int probChuva = map(humidity, 40, 95, 0, 100);
  probChuva = constrain(probChuva, 0, 100); // Garante que o resultado nunca passe de 100 nem fique negativo

  // 4. Monta o pacote de texto no formato XML contendo os 4 valores na ordem exata dos medidores da tela
  String content = "<?xml version = \"1.0\" ?>";
  content += "<inputs>";
  content += "<analog>" + String((int)temperature) + "</analog>"; // Medidor 1: Temperatura
  content += "<analog>" + String((int)humidity) + "</analog>";    // Medidor 2: Umidade
  content += "<analog>" + String((int)distancia) + "</analog>";   // Medidor 3: Vento
  content += "<analog>" + String(probChuva) + "</analog>";        // Medidor 4: Prob. Chuva
  content += "</inputs>";

  // Envia o pacote XML para o JavaScript atualizar os ponteiros na tela sem precisar recarregar a página
  server.send(200, "text/xml", content);
}
