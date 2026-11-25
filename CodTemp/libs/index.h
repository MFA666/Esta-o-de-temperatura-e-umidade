// =========================================================
// BLOCO 1: BIBLIOTECAS, PINOS E VARIÁVEIS GLOBAIS
// =========================================================
#include <Servo.h> // Biblioteca para controlar o servo motor

// Mapeamento dos pinos digitais do Arduino
const int PIN_VERMELHO = 2;
const int PIN_AMARELO  = 3;
const int PIN_VERDE    = 4;
const int PIN_SERVO    = 5;
const int PIN_ECHO     = 7;
const int PIN_TRIG     = 8;

Servo servo1; // Cria o objeto que representa o servo motor

// Variáveis para temporização assíncrona (permite mover o servo a cada 2s sem pausar a leitura do sensor)
unsigned long tempoAnteriorServo = 0; // Guarda o momento da última movimentação
const long intervaloServo = 2000;     // Intervalo desejado em milissegundos (2000 ms = 2 segundos)
int posicaoServo = 0;                 // Guarda o ângulo atual do servo (0° ou 180°)

// =========================================================
// BLOCO 2: CONFIGURAÇÃO INICIAL (EXECUTA UMA VEZ AO LIGAR)
// =========================================================
void setup() {
  // Vincula o servo ao pino 5 e o posiciona no ângulo inicial (0°)
  servo1.attach(PIN_SERVO);
  servo1.write(posicaoServo);

  // Configura os pinos dos LEDs do semáforo como saída de energia
  pinMode(PIN_VERMELHO, OUTPUT);
  pinMode(PIN_AMARELO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);

  // Configura o pino TRIG (emissor de som) como saída e o ECHO (receptor) como entrada
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Inicia a comunicação com o computador para exibir as leituras e garante que os LEDs comecem apagados
  Serial.begin(9600);
  apagarSemaforo();
}

// =========================================================
// BLOCO 3: CICLO PRINCIPAL (EXECUTA CONTINUAMENTE)
// =========================================================
void loop() {
  // --- SUB-BLOCO 3.1: MOVIMENTAÇÃO DO SERVO SEM USAR DELAY ---
  unsigned long tempoAtual = millis(); // Lê há quantos milissegundos o Arduino está ligado
  
  // Verifica se já se passaram 2 segundos desde o último movimento
  if (tempoAtual - tempoAnteriorServo >= intervaloServo) {
    tempoAnteriorServo = tempoAtual; // Atualiza o cronômetro para o próximo ciclo
    
    // Alterna o ângulo entre 0° e 180° (efeito de varredura/radar)
    if (posicaoServo == 0) {
      posicaoServo = 180;
    } else {
      posicaoServo = 0;
    }
    servo1.write(posicaoServo); // Envia o comando físico para o motor girar
  }

  // --- SUB-BLOCO 3.2: DISPARO E CÁLCULO DO SENSOR ULTRASSÔNICO ---
  // Gera um pulso ultrassônico limpo de 10 microssegundos no pino TRIG
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Mede o tempo (em microssegundos) que o som levou para ir, bater no obstáculo e voltar ao pino ECHO
  unsigned long duracao = pulseIn(PIN_ECHO, HIGH);
  
  // Converte tempo em centímetros: (tempo * velocidade do som 0,0343 cm/µs) / 2 (ida e volta)
  float distancia = duracao * 0.0343 / 2;

  // Imprime a distância calculada no Monitor Serial
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // --- SUB-BLOCO 3.3: LÓGICA DO SEMÁFORO DE PROXIMIDADE ---
  // Obstáculo muito perto (entre 0 e 50 cm): acende apenas o Vermelho
  if (distancia > 0 && distancia <= 50) { 
    digitalWrite(PIN_VERMELHO, HIGH);
    digitalWrite(PIN_AMARELO, LOW);
    digitalWrite(PIN_VERDE, LOW);
  } 
  // Obstáculo em distância média (entre 50 e 100 cm): acende apenas o Amarelo
  else if (distancia > 50 && distancia <= 100) {
    digitalWrite(PIN_VERMELHO, LOW);
    digitalWrite(PIN_AMARELO, HIGH);
    digitalWrite(PIN_VERDE, LOW);
  } 
  // Caminho livre (acima de 100 cm ou sem eco): acende apenas o Verde
  else {
    digitalWrite(PIN_VERMELHO, LOW);
    digitalWrite(PIN_AMARELO, LOW);
    digitalWrite(PIN_VERDE, HIGH);
  }

  // Pausa de 60ms para evitar que ecos antigos interfiram na próxima leitura do sensor
  delay(60); 
}

// =========================================================
// BLOCO 4: FUNÇÕES AUXILIARES
// =========================================================
// Desliga todos os LEDs do semáforo simultaneamente
void apagarSemaforo() {
  digitalWrite(PIN_VERMELHO, LOW);
  digitalWrite(PIN_AMARELO, LOW);
  digitalWrite(PIN_VERDE, LOW);
}
