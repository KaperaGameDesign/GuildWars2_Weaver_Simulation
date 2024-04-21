#include <iostream>
#include <vector>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <string>
#include <cmath>
#include <utility>
#include <map>
#include <fstream>
#include <sstream>

#include "loadSkillFromFile.h"
#include "Skill.h"

using namespace std;

void loadSkillFromFile(vector<Skill>& skills) {
	ifstream SkillData;
	SkillData.open("C:\\Users\\SnappyJoe\\Desktop\\Projects\\WeaverSimulation\\SkillData.csv");
	
	string line{};
	getline(SkillData, line);
	line = "";
	while(getline(SkillData, line)) {
		string ID{};
		string skillName{};
		string skillType{};
		string skillWeapon{};
		bool createsField{};
		float fieldTime{};
		string fieldType{};
		float animationTime{};
		string mainAttunement{};
		string secondaryAttunement{};
		float cooldown{};
		string tempString="";

		stringstream inputString(line);

		getline(inputString, ID, ',');
		getline(inputString, skillName, ',');
		getline(inputString, skillType, ',');
		getline(inputString, skillWeapon, ',');
		getline(inputString, tempString, ',');
		createsField = (tempString == "True");
		tempString = "";
		getline(inputString, tempString, ',');
		fieldTime = stof(tempString);
		tempString = "";
		getline(inputString, fieldType, ',');
		getline(inputString, tempString, ',');
		animationTime = stof(tempString);
		tempString = "";
		getline(inputString, mainAttunement, ',');
		getline(inputString, secondaryAttunement, ',');
		getline(inputString, tempString, ',');
		cooldown = stof(tempString);
		tempString = "";
		line = "";

		ifstream DamageData;
		DamageData.open("C:\\Users\\SnappyJoe\\Desktop\\Projects\\WeaverSimulation\\SkillDamageInstance.csv");
		string damageLine{};
		getline(DamageData, damageLine);
		damageLine = "";
		vector<DamageInstance> tempVector;
		while (getline(DamageData, damageLine)) {
			if (damageLine.find(ID) == 0) {
				string ID2{};
				float time{};
				float damageCoefficient{};
				bool fireFieldDamage{};
				Condition condition{};
				int condiStacks{};
				float condiDurations{};
				FinisherType finisher{};

				string tempString = "";

				stringstream inputString(damageLine);

				getline(inputString, ID2, ',');
				getline(inputString, tempString, ',');
				time = stof(tempString);
				tempString = "";
				getline(inputString, tempString, ',');
				if (tempString != "") {
					damageCoefficient = stof(tempString);
				}
				tempString = "";
				getline(inputString, tempString, ',');
				fireFieldDamage = (tempString == "True");
				tempString = "";
				getline(inputString, tempString, ',');
				if (tempString != "") {
					if (tempString == "bleeding") {
						condition = Condition::bleeding;
					} else if (tempString == "burning") {
						condition = Condition::burning;
					} else if (tempString == "confusion") {
						condition = Condition::confusion;
					} else if (tempString == "poisoned") {
						condition = Condition::poisoned;
					} else if (tempString == "torment") {
						condition = Condition::torment;
					} else if (tempString == "blinded") {
						condition = Condition::blinded;
					} else if (tempString == "chilled") {
						condition = Condition::chilled;
					} else if (tempString == "crippled") {
						condition = Condition::crippled;
					} else if (tempString == "fear") {
						condition = Condition::fear;
					} else if (tempString == "immobilized") {
						condition = Condition::immobilized;
					} else if (tempString == "slow") {
						condition = Condition::slow;
					} else if (tempString == "taunt") {
						condition = Condition::taunt;
					} else if (tempString == "weakness") {
						condition = Condition::weakness;
					} else if (tempString == "vulnerability") {
						condition = Condition::vulnerability;
					}
				}
				tempString = "";
				getline(inputString, tempString, ',');
				if (tempString != "") {
					condiStacks = stoi(tempString);
				}
				tempString = "";
				getline(inputString, tempString, ',');
				if (tempString != "") {
					condiDurations = stof(tempString);
				}
				tempString = "";
				getline(inputString, tempString, ',');
				if (tempString != "") {
					if (tempString == "blast") {
						finisher = FinisherType::blast;
					} else if (tempString == "leap") {
						finisher = FinisherType::leap;
					} else if (tempString == "projectile") {
						finisher = FinisherType::projectile;
					} else if (tempString == "whirl") {
						finisher = FinisherType::whirl;
					}
				}
				tempString = "";
				damageLine = "";

				tempVector.push_back({ID2, time, damageCoefficient, fireFieldDamage, condition, condiStacks, condiDurations, finisher});
			}
		}

		Skill skill(ID, skillName, skillType, skillWeapon, createsField, fieldTime, fieldType, animationTime, mainAttunement, secondaryAttunement, cooldown, tempVector);
		skills.push_back(skill);
	}
}