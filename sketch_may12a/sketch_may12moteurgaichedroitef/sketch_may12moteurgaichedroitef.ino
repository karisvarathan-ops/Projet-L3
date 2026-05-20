#include <Wire.h>  // On importe la bibliotheque I2C
                   // pour communiquer avec les cartes moteurs

// ============================================
// ADRESSES DES CARTES MOTEURS
// (trouvées avec le scanner I2C)
// ============================================
#define MOTEUR_DROIT  0x66  // carte CC  = chenille droite
#define MOTEUR_GAUCHE 0x68  // carte D0  = chenille gauche

// ============================================
// DIRECTIONS POSSIBLES
// ============================================
#define ARRET   0x00  // moteur s'arrete
#define AVANT   0x01  // moteur tourne en avant
#define ARRIERE 0x02  // moteur tourne en arriere

// ============================================
// FONCTION POUR PILOTER UN MOTEUR
// adresse  = quelle carte (0x66 ou 0x68)
// direction = AVANT, ARRIERE ou ARRET
// vitesse  = de 0 (lent) a 63 (max)
// ============================================
void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  
  // Securite : vitesse max = 63
  if (vitesse > 63) vitesse = 63;
  
  // On combine vitesse et direction en 1 seul octet
  // << 2 signifie "decale les bits de 2 vers la gauche"
  // | signifie "colle la direction a droite"
  byte commande = (vitesse << 2) | direction;
  
  // On envoie la commande a la carte via I2C
  Wire.beginTransmission(adresse); // on commence a parler a la carte
  Wire.write(0x00);                // registre de controle du DRV8830
  Wire.write(commande);            // on envoie vitesse + direction
  Wire.endTransmission();          // on termine la communication
}

// ============================================
// SETUP : s'execute UNE SEULE FOIS au demarrage
// ============================================
void setup() {
  Wire.begin();          // on demarre le bus I2C
  Serial.begin(9600);    // on demarre le moniteur serie
  Serial.println("Robot pret !");
  delay(500);            // on attend 0.5 seconde
}

// ============================================
// LOOP : s'execute EN BOUCLE en permanence
// ============================================
void loop() {

  // --- AVANCE ---
  Serial.println("Avance");
  piloterMoteur(MOTEUR_DROIT,  AVANT, 50); // chenille droite en avant
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 50); // chenille gauche en avant
  delay(2000); // on attend 2 secondes

  // --- STOP ---
  Serial.println("Stop");
  piloterMoteur(MOTEUR_DROIT,  ARRET, 0); // chenille droite stop
  piloterMoteur(MOTEUR_GAUCHE, ARRET, 0); // chenille gauche stop
  delay(1000); // on attend 1 seconde

  // --- RECULE ---
  Serial.println("Recule");
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, 50); // chenille droite en arriere
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, 50); // chenille gauche en arriere
  delay(2000); // on attend 2 secondes

  // --- STOP ---
  Serial.println("Stop");
  piloterMoteur(MOTEUR_DROIT,  ARRET, 0); // chenille droite stop
  piloterMoteur(MOTEUR_GAUCHE, ARRET, 0); // chenille gauche stop
  delay(1000); // on attend 1 seconde

}