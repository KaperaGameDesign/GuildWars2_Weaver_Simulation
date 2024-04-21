#include "Player.h"
#include <vector>

Player::Player(){
    // No need to initialize attributes here, it will be done in initializeAttributes method
}

double Player::getAttribute(const std::string& attributeName) const {
    auto it = attributes.find(attributeName);
    if (it != attributes.end()) {
        return it->second;
    } else {
        return 0.0;
    }
}

double Player::getConditionDuration(const std::string& conditionName) const {
    auto it = conditionDurations.find(conditionName);
    if (it != conditionDurations.end()) {
        return it->second;
    } else {
        return 0.0; // Return a default value if the condition duration is not found
    }
}

void Player::initializeAttributes() {
    attributes["power"] = 2810+750+150;
    attributes["toughness"] = 1000;
    attributes["vitality"] = 1000;
    attributes["precision"] = 2091;
    attributes["ferocity"] = 1618;
    attributes["conditionDamage"] = 750;
    attributes["expertise"] = 0;
    attributes["concentration"] = 0;
    attributes["health"] = 12145;
    attributes["healingPower"] = 0;

    conditionDurations["bleeding"] = 0.0;
    conditionDurations["burning"] = 0.2;
    conditionDurations["confusion"] = 0.0;
    conditionDurations["poisoned"] = 0.0;
    conditionDurations["torment"] = 0.0;
    conditionDurations["blinded"] = 0.0;
    conditionDurations["chilled"] = 0.0;
    conditionDurations["crippled"] = 0.0;
    conditionDurations["fear"] = 0.0;
    conditionDurations["immobilized"] = 0.0;
    conditionDurations["slow"] = 0.0;
    conditionDurations["taunt"] = 0.0;
    conditionDurations["weakness"] = 0.0;
    conditionDurations["vulnerability"] = 0.0;
}