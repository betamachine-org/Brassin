/*************************************************
 *************************************************

  Gestion d'un brassin thermostaté en chaud et froid

  Sketch Brassin.ino
  Copyright 2020 Betamachine   wwww.betamacune.org - right reserved.



  This file is part of Brassin.

    Brassin is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Brassin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with betaEvents.  If not, see <https://www.gnu.org/licenses/lglp.txt>.



 *************************************************/

#define APP_NAME "Brassin V1.0"

// Constantes modifiable pour les durées en millisecondes
const long delaiLectureTemp = 60L * 1000;

// Numéro des pin hardware pour le bouton
const int pinBouton1 = 5;
const int pinOneWire = 8;



/* Evenements du Manager (voir EventsManager.h)
  evNill = 0,      // No event  about 1 every milisecond but do not use them for delay Use pushDelayEvent(delay,event)
  ev100Hz,         // tick 100HZ    non cumulative (see betaEvent.h)
  ev10Hz,          // tick 10HZ     non cumulative (see betaEvent.h)
  ev1Hz,           // un tick 1HZ   cumulative (see betaEvent.h)
  ev24H,           // 24H when timestamp pass over 24H
  evInit,
  evInChar,
  evInString,
*/

// Liste des evenements specifique a ce projet
enum tUserEventCode {
  // evenement utilisateurs
  evBP0 = 100,        //pousoir D5  (passage en mode clignotant)
  evLed0,             //Led de vie clignotante (LED_BUILTIN)
  evDs18x20,         // evenements internes du evHandelerDS18x20
};


//  betaEvent.h est une aide pour construire les elements de base d'une programation evenementiel

//  une instance "Events" avec un poussoir "BP0" une LED "Led0" un clavier "Keyboard"
//  BP0 genere un evenement evBP0 a chaque pression le poussoir connecté sur BP0_PIN
//  Led0 genere un evenement evLed0 a chaque clignotement de la led precablée sur la platine LED_BUILTIN
//  Keyboard genere un evenement evChar a char caractere recu et un evenement evString a chaque ligne recue
//  Debug permet sur reception d'un "T" sur l'entrée Serial d'afficher les infos de charge du CPU

//#define BP0_PIN   5                //   Par defaut BP0 est sur D5
//#define Led0_PIN  LED_BUILTIN      //   Par defaut Led0 est sur LED_BUILTIN
//#define SERIAL_SPEED 115200        //   Default at 115200
//#define NO_SERIAL                  //   Par defaut Serial est actif : enlevez le commentaire si vous n'en souhaitez pas (economie de memoire)
//#define NO_DEBUG                   //   Par defaut EventDebug est actif : enlevez le commentaire si vous n'en souhaitez pas (economie de memoire)

#include <BetaEvents.h>

// Sondes temperatures : DS18B20
// On utilise la lib OneWire.h et DS18B20.h
// La lib DS18B20 lit en mode bloquant 750ms a chaque lecture donc 1,5 sec pour deux sondes
// TODO: utiliser l'exemple dans la lib OneWire pour avoir une lecture non bloquante ?


//#include <DS18B20.h>
//instance du bus OneWire dedié aux DS18B20
//DS18B20 ds(pinOneWire);
#include "evHandlerDS18x20.h"
evHandlerDS18x20 ds(pinOneWire,10L*1000);


void setup() {

  // Start instance`
  // will setup Serial speed at 115200 by default
  Events.begin();
  Serial.println(F("\r\n\n" APP_NAME));

  Serial.print("Nombre de sonde temperature trouvée : ");
  //  Serial.println(ds.getNumberOfDevices());


  Serial.println("Bonjour ....");
}

bool sleepOk = true;

void loop() {

  Events.get(sleepOk);  // generation du prochain evenement
  Events.handle();      // passage de l'evenement au systeme
  switch (Events.code)  // gestion de l'evenement
  {
    case evInit: {
        Serial.println("Init");
        //        Events.delayedPush(5000L, evLectureTemp); // 1er sequence de lecture dans 5 secondes
      }
      break;

    case evDs18x20: {
        if (Events.ext == evxDsRead) {
          D_println(ds.current);
          D_println(ds.celsius);
        }
      }
      break;


    
    // Evenement pousoir
    case evBP0:
      switch (Events.ext) {
        case evxBPDown:                           // push button 0 went down
          Led0.setMillisec(500, 50);              // set led fast blink
          Serial.println(F("BP0 Down"));          // warn user on console
          break;

        case evxBPUp:                             // push button 0 went up
          Led0.setMillisec(1000, 10);             // set led slow blink
          Serial.println(F("BP0 Up"));            // warn user
          break;


      }
      break;


    case evInString:
      if (Debug.trackTime < 2) {
        D_println(Keyboard.inputString);
      }
      if (Keyboard.inputString.equals("S")) {

        sleepOk = !sleepOk;
        D_println(sleepOk);
      }

      break;

  }
}
