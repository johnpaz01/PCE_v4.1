// Bibliotecas utilizadas
#include <SFE_BMP180.h>
#include <Servo.h>
#include <EEPROM.h>

// Definições Padrões
#define Sinalizador 8
#define EntradaServo 9
#define MAX_ALTITUDE_ADDRESS 0

// Criação dos objetos atribuídos aos sensores e servo motores
SFE_BMP180 Altimetro;
Servo fechadura;

// Variáveis globais
bool armado = false;
double MaxAltitude = 0;
double pressaoAtual = 0;
double pressaoReferencia = 0;
double alturaAtual = 0;
int cont = 0;
int PosicaoNaMemoria = 0;

// Funções Utilizadas no Código
double getPressure();
void respTrue(int n);
void saveAltitudeToEEPROM(double Altitude);
void openParaquedas();

//#####################
// SETUP INICIAL:
//#####################

void setup() {
  // Inicializando as entradas digitais
  pinMode(Sinalizador, OUTPUT);
  pinMode(EntradaServo, OUTPUT);

  // Inicializando o servoMotor
  fechadura.attach(EntradaServo);
  respTrue(35);

  // Confirmando a inicialização
  digitalWrite(Sinalizador, HIGH);
  delay(2000);
  digitalWrite(Sinalizador, LOW);

  // Inicializando o barômetro e tratando seus possíveis casos
  if (Altimetro.begin()) {
    respTrue(35);
  } else {
    digitalWrite(Sinalizador, HIGH);
    delay(1000);
    digitalWrite(Sinalizador, LOW);
    return;
  }

  // Obtém a pressão atual e verifica se é válida
  pressaoReferencia = getPressure();
  if (pressaoReferencia != -1) {
    respTrue(35);
  } else {
    digitalWrite(Sinalizador, HIGH);
    delay(1000);
    digitalWrite(Sinalizador, LOW);
    return;
  }
}

//#####################
// FIM SETUP INICIAL:
//#####################

//#####################
// LOOP PRINCIPAL:
//#####################

void loop() {
  digitalWrite(Sinalizador, HIGH);

  // Obtendo a altura atual
  pressaoAtual = getPressure();

  if (pressaoAtual != -1) {
    alturaAtual = Altimetro.altitude(pressaoAtual, pressaoReferencia);
    digitalWrite(Sinalizador, LOW);
    respTrue(3);
  } else {
    return;
  }

  // Verificando se a altura é suficiente para salvar na memória
  if (alturaAtual >= 4.0) {
    cont++;
    if (cont % 10 == 0 && PosicaoNaMemoria < 1000) {
      PosicaoNaMemoria += sizeof(double);
      respTrue(3);
    }
    if (alturaAtual > MaxAltitude) {
      MaxAltitude = alturaAtual;
      respTrue(3);
    }
  }

  // Verificação para abertura do paraquedas
  if (alturaAtual <= MaxAltitude - 5.0) {
    openParaquedas();
  }
}

//#####################
// FIM LOOP
//#####################

//######################################
// Funções Utilizadas no Código:
//######################################

// Função para obter a pressão
double getPressure() {
  char status;
  double P, T;

  // Inicia a medição de pressão
  status = Altimetro.startPressure(3); // 3 para alta resolução
  if (status != 0) {
    delay(status);
    // Obtém a pressão e a temperatura
    status = Altimetro.getPressure(P, T);
    if (status != 0) {
      return P;
    } else {
      digitalWrite(Sinalizador, HIGH); // Acende o LED indicando erro na leitura da pressão
    }
  } else {
    digitalWrite(Sinalizador, HIGH); // Acende o LED indicando erro ao iniciar a medição de pressão
  }
  return -1; // Retorna um valor padrão em caso de falha
}

// Função de resposta para o usuário
void respTrue(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(Sinalizador, HIGH);
    delay(35);
    digitalWrite(Sinalizador, LOW);
    delay(35);
  }
}

// Função de salvar altitude na EEPROM
void saveAltitudeToEEPROM(double Altitude) {
  byte* p = (byte*)(void*)&Altitude;
  for (int i = 0; i < sizeof(Altitude); i++) {
    EEPROM.write(PosicaoNaMemoria + i, *p++);
  }
}

// Função de ativação de abertura de paraquedas
void openParaquedas() {
  fechadura.write(180); // Movimenta o servo para a posição de 180 graus
  respTrue(10);
}
