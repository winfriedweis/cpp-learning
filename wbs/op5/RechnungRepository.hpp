//
//  RechnungRepository.hpp
//  cpp-learning
//
//  Created by Winfried Weis on 17.03.26.
//
#pragma once
#include "RechnungData.h"
#include <fstream>

class RechnungRepository {
public:
  void speichereInDatei( const RechnungData& data ) const {
    std::ofstream file("rechnung_ " + data.getEmpfaenger() +".txt");
                       if (file.is_open()) {
      file << data.getEmpfaenger() << ": " << data.getBetrag();
      std::cout << "Rechnung gespeichert" << std::endl;
    }
  }
};


