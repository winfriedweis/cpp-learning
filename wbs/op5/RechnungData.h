//
//  RechnungData.h
//  cpp-learning
//
//  Created by Winfried Weis on 17.03.26.
//

#pragma once // Wird nur einmal geladen / ausgeführt
#include <string> // Wir nutzen die Klasse / Bibliothek String

class RechnungData { // Wir legen die Klasse an
  
private:  // Sichtbarkeitsmodifikator
  
  std::string emfaenger; // Legen eine String-Variable an
  double betrag;         // Eine Variable vom Typ double wird im Speicher allokiert - Hier findet eine deklaration statt
  
public: //
  
  RechnungData(std::string emp, double bet) : // Konsturktor für RechnungData-Objekt
  emfaenger(std::move(emp)), betrag(bet) {}
  
  std::string getEmpfaenger()   const { return empfaenger; } // Getter für Empfaenger
  double getBetrag()            const { return betrag ; } // Getter für Betrag
}


// Es wird ein Objekt von Rechnungsdata erstellt, in welchem Empänger und Betrag gespeichert werden und abrufbar sind. 
