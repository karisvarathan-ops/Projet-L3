#include <Wire.h>

#define ADDR_LF     0x20
#define REG_DIGITAL 0x07

#define MOTEUR_DROIT  0x66
#define MOTEUR_GAUCHE 0x68

#define AVANT_DROIT    0x02
#define ARRIERE_DROIT  0x01
#define AVANT_GAUCHE   0x01
#define ARRIERE_GAUCHE 0x02
#define ARRET          0x00

// ======================================================
// PID ADAPTATIF
// << Kp change selon si on est en ligne droite ou virage
// ligne droite (erreur 0-1) : Kp faible = stable
// virage (erreur 2-3)       : Kp fort  = réactif
// ======================================================

float Kp_droit  = 1.5;  // << ligne droite : doux
float Kp_virage = 3.0;  // << virage : réactif
float Kd        = 0.3;

float erreur     = 0;
float erreurPrec = 0;
float correction = 0;

#define VITESSE_BASE        30
#define VITESSE_BASE_VIRAGE 23
#define VITESSE_MAX         42
#define VITESSE_MIN         22

int derniereErreur = 0;

int compteurNoir = 0;
#define SEUIL_ARRET 6

bool enVirage = false;

void piloterMoteur(byte adresse, byte direction, byte vitesse)
{
  vitesse = constrain(vitesse, 0, 63);
  byte commande = (vitesse << 2) | direction;
  Wire.beginTransmission(adresse);
  Wire.write(0x00);
  Wire.write(commande);
  Wire.endTransmission();
}

void stopRobot()
{
  piloterMoteur(MOTEUR_GAUCHE, ARRET, 0);
  piloterMoteur(MOTEUR_DROIT,  ARRET, 0);
}

uint8_t lireCapteurs()
{
  Wire.beginTransmission(ADDR_LF);
  Wire.write(REG_DIGITAL);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)ADDR_LF, (uint8_t)1);
  if (!Wire.available()) return 0xFF;
  return Wire.read() & 0x0F;
}

int calculErreur(uint8_t e)
{
  switch(e)
  {
    case 0b1001: return  0;
    case 0b0110: return  0;
    case 0b0001: return -1;
    case 0b0011: return -2;
    case 0b0111: return -3;
    case 0b1000: return  1;
    case 0b1100: return  2;
    case 0b1110: return  3;
  }
  return derniereErreur;
}

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial.println("=== PID V3 KP ADAPTATIF ===");
}

void loop()
{
  uint8_t e = lireCapteurs();

  // ==================================================
  // ARRET FINAL
  // ==================================================

  if (e == 0b0000)
  {
    compteurNoir++;
    Serial.print("Noir détecté : ");
    Serial.println(compteurNoir);

    if (compteurNoir >= SEUIL_ARRET)
    {
      stopRobot();
      Serial.println("ARRET FINAL CONFIRME");
      while(true);
    }

    piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, 24);
    piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  24);
    delay(5);
    return;
  }
  else
  {
    compteurNoir = 0;
  }

  // ==================================================
  // BLANC = ligne perdue
  // récupération gauche/droite — inchangé
  // ==================================================

  if (e == 0b1111)
  {
    if (derniereErreur < 0)
    {
      piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, 18);
      piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  30);
    }
    else
    {
      piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, 30);
      piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  18);
    }
    return;
  }

  // ==================================================
  // GROS VIRAGES
  // ==================================================

  if (e == 0b0011 || e == 0b0111)
  {
    piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, 10);
    piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  28);
    derniereErreur = -3;
    enVirage = true;
    return;
  }

  if (e == 0b1100 || e == 0b1110)
  {
    piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, 28);
    piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  10);
    derniereErreur = 3;
    enVirage = true;
    return;
  }

  // ==================================================
  // PID
  // ==================================================

  erreur = calculErreur(e);
  derniereErreur = erreur;

  int P = erreur;
  int D = erreur - erreurPrec;

  static float correctionLisse = 0;

  if (enVirage)
  {
    correctionLisse = 0;
    erreurPrec      = 0;
    enVirage        = false;
  }

  // << Kp adaptatif selon l'erreur
  float Kp = (abs(erreur) <= 1) ? Kp_droit : Kp_virage;

  float correctionBrute = (Kp * P) + (Kd * D);

  correctionLisse = 0.7 * correctionLisse + 0.3 * correctionBrute;
  correction = correctionLisse;

  if (abs(erreur) >= 2)
    correction *= 0.7;

  correction = constrain(correction, -8, 8);

  // << zone morte élargie sur ligne droite
  float zoneMorte = (abs(erreur) <= 1) ? 2.0 : 1.5;
  if (abs(correction) < zoneMorte)
    correction = 0;

  erreurPrec = erreur;

  // ==================================================
  // CALCUL VITESSES
  // ==================================================

  int vitesseBase = (abs(erreur) >= 2) ? VITESSE_BASE_VIRAGE : VITESSE_BASE;

  int vitesseGauche = vitesseBase - correction;
  int vitesseDroite = vitesseBase + correction;

  if (vitesseGauche < 22) vitesseGauche = 22;
  if (vitesseDroite < 22) vitesseDroite = 22;

  vitesseGauche = constrain(vitesseGauche, VITESSE_MIN, VITESSE_MAX);
  vitesseDroite = constrain(vitesseDroite, VITESSE_MIN, VITESSE_MAX);

  // ==================================================
  // ENVOI AUX MOTEURS
  // ==================================================

  piloterMoteur(MOTEUR_GAUCHE, AVANT_GAUCHE, vitesseGauche);
  piloterMoteur(MOTEUR_DROIT,  AVANT_DROIT,  vitesseDroite);

  // ==================================================
  // DEBUG
  // ==================================================

  Serial.print("Capteurs : ");
  Serial.print(e, BIN);
  Serial.print("  Kp:");
  Serial.print(Kp);
  Serial.print("  erreur:");
  Serial.print(erreur);
  Serial.print("  corr:");
  Serial.print(correction);
  Serial.print("  VG:");
  Serial.print(vitesseGauche);
  Serial.print("  VD:");
  Serial.println(vitesseDroite);

  delay(5);
}