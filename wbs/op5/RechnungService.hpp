//
//  RechnungService.hpp
//  cpp-learning
//
//  Created by Winfried Weis on 17.03.26.
//
// Preisberechnung

#pragma once
#include "RechnungData.h"
#include <iostream>

class RechnungService {
public:
  void berechnePreis( const RechnungData& data ) const {
    std::cout << "Preis berechnet für: " << data.getEmpfaenger() << " | Betrag: " << data.getBetrag() << std::endl;
    
  }
};
