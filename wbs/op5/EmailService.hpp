//
//  EmailService.hpp
//  cpp-learning
//
//  Created by Winfried Weis on 17.03.26.
//

#pragma once
#include "RechnungData.hpp"
#include <iostream>

class EmailService {
public:
  void sendeEmail(const RechnungData& data) const {
    std::cout << "Email gesendet! An: " << data.getEmpfaenger(); << std::endl;
  }
};
