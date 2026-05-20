#include <Wire.h>
#include <Servo.h>

// =====================================================
// ULTRASON
// =====================================================

#define TRIG_PIN 7
#define ECHO_PIN 6

// =====================================================
// CAPTEURS LIGNE
// =====================================================

#define CAPTEUR_GAUCHE 2
#define CAPTEUR_DROIT 3

// =====================================================
// MOTEURS
// =====================================================

#define MOTEUR_DROIT  0x66
#define MOTEUR_GAUCHE 0x68

#define ARRET   0x00
#define AVANT   0x01
#define ARRIERE 0x02

// =====================================================
// VARIABLES
// =====================================================

int etape = 0;

// =====================================================
// FONCTION MOTEUR
// =====================================================

void piloterMoteur(byte adresse, byte direction, byte vitesse) {

  if (vitesse > 63) vitesse = 63;

  byte commande = (vitesse << 2) | direction;

  Wire.beginTransmission(adresse);
  Wire.write(0x00);
  Wire.write(commande);
  Wire.endTransmission();
}

// =====================================================
// DEPLACEMENTS
// =====================================================

void avancer(int vitesse) {

  piloterMoteur(MOTEUR_DROIT, AVANT, vitesse);
  piloterMoteur(MOTEUR_GAUCHE, AVANT, vitesse);
}

void stopRobot() {

  piloterMoteur(MOTEUR_DROIT, ARRET, 0);
  piloterMoteur(MOTEUR_GAUCHE, ARRET, 0);
}

void tournerDroite(int vitesse) {

  piloterMoteur(MOTEUR_DROIT, ARRIERE, vitesse);
  piloterMoteur(MOTEUR_GAUCHE, AVANT, vitesse);
}

void tournerGauche(int vitesse) {

  piloterMoteur(MOTEUR_DROIT, AVANT, vitesse);
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, vitesse);
}

// =====================================================
// DISTANCE
// =====================================================

long mesurerDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duree = pulseIn(ECHO_PIN, HIGH);

  return duree * 0.034 / 2;
}

// =====================================================
// PREMIER OBSTACLE -> DROITE
// =====================================================

void obstacleDroite() {

  stopRobot();
  delay(300);

  tournerDroite(40);
  delay(500);

  avancer(40);
  delay(1200);

  tournerGauche(40);
  delay(500);

  avancer(40);
  delay(1500);

  tournerGauche(40);
  delay(500);

  avancer(40);
  delay(1200);

  tournerDroite(40);
  delay(500);

  stopRobot();
}

// =====================================================
// VIRAGE
// =====================================================

void faireVirage() {

  avancer(40);
  delay(1000);

  tournerGauche(40);
  delay(700);

  avancer(40);
  delay(1200);

  stopRobot();
}

// =====================================================
// SECOND OBSTACLE -> GAUCHE
// =====================================================

void obstacleGauche() {

  stopRobot();
  delay(300);

  tournerGauche(40);
  delay(500);

  avancer(40);
  delay(1200);

  tournerDroite(40);
  delay(500);

  avancer(40);
  delay(1500);

  tournerDroite(40);
  delay(500);

  avancer(40);
  delay(1200);

  tournerGauche(40);
  delay(500);

  stopRobot();
}

// =====================================================
// SUIVI LIGNE
// =====================================================

void suivreLigne() {

  int gauche = digitalRead(CAPTEUR_GAUCHE);
  int droite = digitalRead(CAPTEUR_DROIT);

  if (gauche == LOW && droite == LOW) {

    avancer(35);
  }

  else if (gauche == LOW) {

    tournerGauche(30);
  }

  else if (droite == LOW) {

    tournerDroite(30);
  }

  else {

    avancer(25);
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Wire.begin();

  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(CAPTEUR_GAUCHE, INPUT);
  pinMode(CAPTEUR_DROIT, INPUT);

  Serial.println("Parcours complet");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  long distance = mesurerDistance();

  // ==========================================
  // ETAPE 0 -> PREMIER OBSTACLE
  // ==========================================

  if (etape == 0) {

    avancer(40);

    if (distance < 20) {

      obstacleDroite();

      etape = 1;
    }
  }

  // ==========================================
  // ETAPE 1 -> VIRAGE
  // ==========================================

  else if (etape == 1) {

    faireVirage();

    etape = 2;
  }

  // ==========================================
  // ETAPE 2 -> SECOND OBSTACLE
  // ==========================================

  else if (etape == 2) {

    avancer(40);

    if (distance < 20) {

      obstacleGauche();

      etape = 3;
    }
  }

  // ==========================================
  // ETAPE 3 -> RETROUVER LIGNE NOIRE
  // ==========================================

  else if (etape == 3) {

    suivreLigne();
  }

  delay(50);
}