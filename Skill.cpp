#include "Skill.h"
#include <string>
#include <vector>

Skill::Skill(const std::string& ID, const std::string& name, const std::string& type, const std::string& weapon, bool createsField,
             float fieldTime, const std::string& fieldType, float animationTime, const std::string& mainAttunement,
             const std::string& secondaryAttunement, float cooldown, const std::vector<DamageInstance>& damageInstances)
    : ID(ID), skillName(name), skillType(type), skillWeapon(weapon), createsField(createsField), fieldTime(fieldTime),
      fieldType(fieldType), animationTime(animationTime), mainAttunement(mainAttunement),
      secondaryAttunement(secondaryAttunement), cooldown(cooldown), damageInstances(damageInstances) {}

Skill::Skill() {}