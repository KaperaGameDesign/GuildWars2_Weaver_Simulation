#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <vector>
#include <iostream>

enum class FinisherType {
    blast,
    leap,
    projectile,
    whirl
};

enum class Condition {
    bleeding,
    burning,
    confusion,
    poisoned,
    torment,
    blinded,
    chilled,
    crippled,
    fear,
    immobilized,
    slow,
    taunt,
    weakness,
    vulnerability
};

struct DamageInstance {
    std::string ID;
    float time;
    float damageCoefficient;
    bool fireFieldDamage;
    Condition condition;
	int condiStacks;
	float condiDurations;
    FinisherType finisher;
};

class Skill {
public:
    std::string ID;
    std::string skillName;
    std::string skillType;
    std::string skillWeapon;
    bool createsField;
    float fieldTime;
    std::string fieldType;
    float animationTime;
    std::string mainAttunement;
    std::string secondaryAttunement;
	float cooldown;
    
    std::vector<DamageInstance> damageInstances;

    // Default constructor with default values for each parameter
    Skill();

    // Constructor
    Skill(const std::string& ID, const std::string& name, const std::string& type, const std::string& weapon, bool createsField, float fieldTime,
          const std::string& fieldType, float animationTime, const std::string& mainAttunement,
          const std::string& secondaryAttunement, float cooldown, const std::vector<DamageInstance>& damageInstances);


    void display() {
        std::cout << "ID: " << ID << std::endl;
        std::cout << "Skill: " << skillName << std::endl;
        std::cout << "Type: " << skillType << std::endl;
        std::cout << "Weapon: " << skillWeapon << std::endl;
        std::cout << "Creates Field: " << createsField << std::endl;
        std::cout << "Field Time: " << fieldTime << std::endl;
        std::cout << "Field Type: " << fieldType << std::endl;
        std::cout << "Animation Time: " << animationTime << std::endl;
        std::cout << "Main Attunement: " << mainAttunement << std::endl;
        std::cout << "Secondary Attunement: " << secondaryAttunement << std::endl;
        std::cout << "Cooldown: " << cooldown << std::endl;

        std::cout << "Damage Instances:" << std::endl;
        for (const auto& damageInstance : damageInstances) {
            std::cout << "  Damage Instance ID: " << damageInstance.ID << std::endl;
            std::cout << "  Time: " << damageInstance.time << std::endl;
            std::cout << "  Damage Coefficient: " << damageInstance.damageCoefficient << std::endl;
            std::cout << "  Fire Field Damage: " << damageInstance.fireFieldDamage << std::endl;
            // Print other members of DamageInstance as needed
        }
    }

    std::string getMainAttunement() const {
        return mainAttunement;
    }

    std::string getSecondaryAttunement() const {
        return secondaryAttunement;
    }

    std::string getSkillType() const {
        return skillType;
    }

    std::string getID() const {
        return ID;
    }

    double getAnimationTime() const {
        return animationTime;
    }

    double getCooldown() const {
        return cooldown;
    }

};

#endif

