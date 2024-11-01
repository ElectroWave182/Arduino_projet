#include <Wire.h>
#include "TM1637.h"

const int CLK = 4;
const int DIO = 5;
TM1637 tm1637(CLK, DIO);


int etat = 1;
/*
etat = 1 : attente d'une nouvelle partie
etat = 2 : jeu en cours
etat = 3 : jeu en pause
etat = 4 : ecran de fin
*/

// Serial1 = RX a 17 et TX a 16
int bouton = 3;
bool rebond = false;
int score = 0;
const int maxi = 9999;
String message;
char lu;
char _;


void vide ()
{
    while (Serial2.available () > 0)
    {
        _ = (char) Serial2.read ();
    }
}

void envoi (String message)
{
    char c;
    for (int num = 0; num < message.length (); num ++)
    {
        c = message.charAt (num);
        Serial.println (num);
        Serial2.print ((char) c);
    }
}


void pause ()
{
  if (! rebond && (etat == 2 || etat == 3))
  {
    rebond = true;
    etat = 5 - etat;
    Serial2.print ('P');
  }
  delay (150);
  rebond = false;
}

void updateScore ()
{
    if (score < maxi)
    {
      score ++;
    }
    tm1637.displayNum (score);
}


void setup ()
{
  Serial.begin (9600);
  Serial2.begin (9600);

  // appareillage de l'afficheur
  tm1637.init ();
  tm1637.set (BRIGHTEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  tm1637.displayStr ("   0");

  // appareillage du bouton
  pinMode (bouton, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt (bouton), pause, FALLING);
}

void loop ()
{
  delay (50);
  switch (etat)
  {

    case 1:
      if (Serial2.available () > 0)
      {
        lu = (char) Serial2.read ();
        vide ();
        if (lu == '2')
        {
            etat = 2;
        }
      }
      break;

    case 2:
      updateScore ();
      if (Serial2.available () > 0)
      {
        lu = (char) Serial2.read ();
        Serial.println (lu);
        vide ();
        if (lu == '4')
        {
            etat = 4;
            message = (String) score;
            for (int _ = 0; _  < 5 - message.length (); _ ++)
            {
              message = "0" + message;
            }
            Serial.print ("message : ");
            Serial.println (message);
            envoi (message);
        }
      }
      break;

    case 3:
      break;

    case 4:
      if (Serial2.available () > 0)
      {
        lu = (char) Serial2.read ();
        vide ();
        if (lu == '1')
        {
            etat = 1;
            score = 0;
            tm1637.displayStr ("   0");
        }
      }
      break;

  }
}
