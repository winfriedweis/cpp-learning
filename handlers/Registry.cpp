#include "Registry.h"
#include "TwoSumHandler.h"
#include "AddTwoNumbersHandler.h"
#include "RayCasterHandler.h"

std::vector<AlgoEntry> getAllEntries() {
    return {
        { "  Two Sum  (#1)",         handleTwoSum         },
        { "  Add Two Numbers  (#2)", handleAddTwoNumbers  },
        { "  RayCast - Game",        handleRayCaster      },
    };
}
