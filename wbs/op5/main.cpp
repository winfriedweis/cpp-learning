//
//  main.cpp
//  cpp-learning
//
//  Created by Winfried Weis on 17.03.26.
//

#include "RechnungData.h"
#include "RechnungService.hpp"
#include "RechnungRepository.hpp"
#include "EmailService.hpp"

int main() {
  RechnungData rechnung("Mike Huebner", 12.99);
  
  RechnungService service;
  RechnungRepository repository;
  EmailService emailService;
  
  service.berechnePreis(rechnung);
  repository.speichereInDatei(rechnung);
  emailService.sendeEmail(rechnung);
  
  return 0;
}
