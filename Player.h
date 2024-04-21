#ifndef PLAYER_H
#define PLAYER_H

#include <map>
#include <string>

class Player {
public:
    std::map<std::string, float> attributes;
    std::map<std::string, float> conditionDurations;

    Player();

    void initializeAttributes();
    double getAttribute(const std::string& attributeName) const;
    double getConditionDuration(const std::string& conditionName) const;
};

#endif