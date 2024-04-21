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
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>
#include <deque>
#include <unordered_map>

#include "Skill.h"
#include "Player.h"
#include "loadSkillFromFile.h"

using namespace std;

double addMods(const vector<double>& vec) {
    double sum = 1.0;
    for (double val : vec) {
        sum += val;
    }
    return sum;
}

double multiMods(const vector<double>& vec) {
    double product = 1.0;
    for (double val : vec) {
        product *= (1 + val);
    }
    return product;
}

vector<tuple<double, string, double>> mergeAndSort(const vector<tuple<double, string, double>>& vec1, const vector<tuple<double, string, double>>& vec2) {
    vector<tuple<double, string, double>> merged;
    merged.reserve(vec1.size() + vec2.size());
    merged.insert(merged.end(), vec1.begin(), vec1.end());
    merged.insert(merged.end(), vec2.begin(), vec2.end());
    sort(merged.begin(), merged.end());
    return merged;
}

void writeToCSV(const vector<double>& timeVec, const vector<string>& nameVec, const vector<double>& damageVec, const vector<double>& DPS) {
    ofstream file("DamageInstances.csv");
    file << "Time,DamageInstance,Damage\n";
    for (size_t i = 0; i < timeVec.size(); i++) {
        file << timeVec[i] << "," << nameVec[i] << "," << damageVec[i] << "," << DPS[i] << "\n";
    }
    file.close();
}

void calculateDPS(const vector<double>& timeVec, const vector<double>& damageVec, vector<double>& DPS) {
    DPS.assign(timeVec.size(), 0.0);
    double totalDamage = 0.0;
    for (size_t j = 0; j < timeVec.size(); ++j) {
        totalDamage += damageVec[j];
        if (timeVec[j] > 0) {
            DPS[j] = totalDamage / timeVec[j];
        }
    }
}

string weightedRandomSelection(const vector<string>& availableSkills, const map<string, double>& skillWeights) {
    map<string, double> filteredSkillWeights;
    for (const auto& skill : availableSkills) {
        if (skillWeights.find(skill) != skillWeights.end()) {
            filteredSkillWeights[skill] = skillWeights.at(skill);
        }
    }

    double totalWeight = 0.0;
    for (const auto& entry : filteredSkillWeights) {
        totalWeight += entry.second;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> distribution(0.0, totalWeight);
    double randomWeight = distribution(gen);

    double cumulativeWeight = 0.0;
    for (const auto& entry : filteredSkillWeights) {
        cumulativeWeight += entry.second;
        if (randomWeight <= cumulativeWeight) {
            return entry.first;
        }
    }
    return "";
}

int main() {
	auto start = chrono::high_resolution_clock::now();
	int numIterations{50000};
	double bestFinalDPS{0.0};
	vector<double> bestTimeOfDamageInstances{};
	vector<double> bestDamageOfDamageInstances{};
	vector<string> bestNameOfDamageInstances{};
	vector<double> bestDPS{};
	vector<string> bestPrimaryAttunement;
	vector<string> bestSecondaryAttunement;
	vector<double> bestAttunementTime;
	vector<string> bestRotation;
	vector<double> bestRotationTime;

	for (int iter = 0 ; iter < numIterations; iter++) {
		// Initialization of player stats and skills
		Player player;
		player.initializeAttributes();
		vector<Skill> skills;
		loadSkillFromFile(skills);
		double maxRotationTime{62.0};
		double targetArmor{2597.0};

		double wovenAirStart{};
		double wovenAirFinish{};
		double wovenFireStart{};
		double wovenFireFinish{};
		vector<double> elementsOfRageStarts;
		vector<double> elementsOfRageFinishes;
		vector<double> fireworksStart;
		vector<double> fireworksFinish;

		map<string, double> originalCooldowns;
		for (auto& skill : skills) {
			originalCooldowns[skill.getID()] = skill.getCooldown();
		}

		// Adjust cooldown by Alacrity and CDR traits
		for (auto& skill : skills) {
			skill.cooldown /= 1.25;
			if (skill.skillType != "Glyph") {
				if ((skill.mainAttunement == "Fire" && skill.secondaryAttunement == "") ||
					(skill.mainAttunement == "" && skill.secondaryAttunement == "Fire") ||
					(skill.mainAttunement == "Fire" && skill.secondaryAttunement == "Fire")) {
					skill.cooldown /= 1.25;
				}
			}
			if (skill.skillType != "Glyph") {
				if ((skill.mainAttunement == "Air" && skill.secondaryAttunement == "") ||
					(skill.mainAttunement == "" && skill.secondaryAttunement == "Air") ||
					(skill.mainAttunement == "Air" && skill.secondaryAttunement == "Air")) {
					skill.cooldown /= 1.25;
				}
			}
		}

		// Setup variables for attunement rotation
		const vector<string> attunements{"Fire", "Air", "Water", "Earth"};
		vector<string> primaryAttunement{};
		vector<string> secondaryAttunement{};
		vector<double> attunementTime{};
		vector<string> availableAttunements = attunements;

		// Generate random starting attunements
		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dist(0, attunements.size() - 1);
		int randomIndex1 = dist(gen);
		int randomIndex2 = dist(gen);

		primaryAttunement.push_back(attunements[randomIndex1]);
		secondaryAttunement.push_back(attunements[randomIndex2]);
		attunementTime.push_back(0.0);

		vector<string> visitedAttunements{};
		bool endWeaveSelf = false;
		double randomSwapFireWS = 2.7;
		double randomSwapWaterWS = 1.75;
		double randomSwapAirWS = 2.2;
		double randomSwapEarthWS = 1.75;
		double chosenRandomSwapWS{0.0};

		visitedAttunements.push_back(primaryAttunement[0]);

		// Generate Weave self random attunements
		while (!endWeaveSelf) {
			availableAttunements = attunements;
			if (visitedAttunements.size() == 3 && attunementTime.back() < 17.0) {
				for (const string& attunement :attunements) {
					if (find(visitedAttunements.begin(), visitedAttunements.end(), attunement) == visitedAttunements.end()) {
						availableAttunements.erase(remove(availableAttunements.begin(), availableAttunements.end(), attunement), availableAttunements.end());
					}
				}
			}
			if (primaryAttunement.back() == secondaryAttunement.back()) {
				availableAttunements.erase(remove(availableAttunements.begin(), availableAttunements.end(), primaryAttunement.back()), availableAttunements.end());
			}
			random_device rd;
			mt19937 gen(rd());
			uniform_int_distribution<int> dist(0, availableAttunements.size() - 1);

			secondaryAttunement.push_back(primaryAttunement.back());
			int randomIndex = dist(gen);
			if (attunementTime.back() > 17.0) {
				string newPrimaryAttunement = "";
				for (const string& attunement : attunements) {
					if (find(visitedAttunements.begin(), visitedAttunements.end(), attunement) == visitedAttunements.end()) {
						newPrimaryAttunement = attunement;
						break;
					}
				}
				primaryAttunement.push_back(newPrimaryAttunement);
			} else {
				primaryAttunement.push_back(availableAttunements[randomIndex]);
			}
		
			visitedAttunements.push_back(primaryAttunement.back());
			sort(visitedAttunements.begin(), visitedAttunements.end());
			visitedAttunements.erase(unique(visitedAttunements.begin(), visitedAttunements.end()), visitedAttunements.end());

			if (secondaryAttunement.back() == "Fire") {
				chosenRandomSwapWS = randomSwapFireWS;
			} else if (secondaryAttunement.back() == "Water") {
				chosenRandomSwapWS = randomSwapWaterWS;
			} else if (secondaryAttunement.back() == "Air") {
				chosenRandomSwapWS = randomSwapAirWS;
			} else if (secondaryAttunement.back() == "Earth") {
				chosenRandomSwapWS = randomSwapEarthWS;
			}

			double randomIncrement = 1.6 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (chosenRandomSwapWS - 1.6)));
			attunementTime.push_back(attunementTime.empty() ? 0.0 : attunementTime.back() + randomIncrement);

			if (attunementTime.back() > 20.6 || visitedAttunements.size() == 4) {
				wovenAirFinish = attunementTime.back()+10;
				wovenFireFinish = attunementTime.back() + 10;
				endWeaveSelf = true;
			}
		}

		// Small fix for Weave Self attunemnets
		availableAttunements = attunements;
		if (primaryAttunement.back() == secondaryAttunement.back()) {
			availableAttunements.erase(remove(availableAttunements.begin(), availableAttunements.end(), primaryAttunement.back()), availableAttunements.end());
		}

		secondaryAttunement.push_back(primaryAttunement.back());
		int randomIndex = dist(gen);
		primaryAttunement.push_back(availableAttunements[randomIndex]);

		if (secondaryAttunement.back() == "Fire") {
			chosenRandomSwapWS = randomSwapFireWS;
		} else if (secondaryAttunement.back() == "Water") {
			chosenRandomSwapWS = randomSwapWaterWS;
		} else if (secondaryAttunement.back() == "Air") {
			chosenRandomSwapWS = randomSwapAirWS;
		} else if (secondaryAttunement.back() == "Earth") {
			chosenRandomSwapWS = randomSwapEarthWS;
		}

		double randomIncrement = 1.6 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (chosenRandomSwapWS - 1.6)));
		attunementTime.push_back(attunementTime.empty() ? 0.0 : attunementTime.back() + randomIncrement);

		double randomSwapFire = 5.2;
		double randomSwapWater = 3.2;
		double randomSwapAir = 4.0;
		double randomSwapEarth = 3.2;
		double chosenRandomSwap{0.0};

		// Generate attunement rotation outside of Weave Self
		while (attunementTime.back() < maxRotationTime) {
			availableAttunements = attunements;
			if (primaryAttunement.back() == secondaryAttunement.back()) {
				availableAttunements.erase(remove(availableAttunements.begin(), availableAttunements.end(), primaryAttunement.back()), availableAttunements.end());
			}
			random_device rd;
			mt19937 gen(rd());
			uniform_int_distribution<int> dist(0, availableAttunements.size() - 1);

			secondaryAttunement.push_back(primaryAttunement.back());
			int randomIndex = dist(gen);
			primaryAttunement.push_back(availableAttunements[randomIndex]);

			if (secondaryAttunement.back() == "Fire") {
				chosenRandomSwap = randomSwapFire;
			} else if (secondaryAttunement.back() == "Water") {
				chosenRandomSwap = randomSwapWater;
			} else if (secondaryAttunement.back() == "Air") {
				chosenRandomSwap = randomSwapAir;
			} else if (secondaryAttunement.back() == "Earth") {
				chosenRandomSwap = randomSwapEarth;
			}

			double randomIncrement = 3.2 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (chosenRandomSwap - 3.2)));
			attunementTime.push_back(attunementTime.empty() ? 0.0 : attunementTime.back() + randomIncrement);
		}
		
		primaryAttunement.clear();
		secondaryAttunement.clear();
		attunementTime.clear();

		// Manual declaration of attunement order and time - OPTIONAL
		/*
		primaryAttunement = {"Air", "Air", "Fire", "Fire", "Earth", "Fire", "Air", "Air", "Fire", "Fire", "Air", "Water", "Fire", "Fire", "Air", "Air", "Fire", "Fire", "Air", "Air", "Fire", "Fire", "Air", "Fire"};
		secondaryAttunement = {"Fire", "Air", "Air", "Fire", "Fire", "Earth", "Fire", "Air", "Air", "Fire", "Fire", "Air", "Water", "Fire", "Fire", "Air", "Air", "Fire", "Fire", "Air", "Air", "Fire", "Fire", "Air"};
		attunementTime = {0.0, 1.3, 3.0, 5.1, 6.8, 8.8, 10.5, 12.3, 14.3, 16.2, 18.0, 19.7, 21.5, 24.76, 28.6, 32.28, 35.92, 39.2, 42.7, 46.78, 50.28, 53.56, 57.0, 60.64};
		wovenAirFinish = 29.678;
		wovenFireFinish = 29.678;
		*/
		
		// Find correct times of wovenAir/Fire and elementsOfRage
		for (size_t i = 0; i < primaryAttunement.size(); ++i) {
			if (primaryAttunement[i] == "Air") {
				wovenAirStart = attunementTime[i];
				break;
			}
		}

		for (size_t i = 0; i < primaryAttunement.size(); ++i) {
			if (primaryAttunement[i] == "Fire") {
				wovenFireStart = attunementTime[i];
				break;
			}
		}

		for (size_t i = 0; i < primaryAttunement.size(); ++i) {
			if (primaryAttunement[i] == secondaryAttunement[i]) {
				elementsOfRageStarts.push_back(attunementTime[i]);
				elementsOfRageFinishes.push_back(attunementTime[i] + 8);
			}
		}

	////////////////////////////////////////////////////////////////// SKILL ROTATION //////////////////////////////////////////////////////////////////

		// Setup variables for skill rotation
		vector<string> rotation;
		vector<double> rotationTime{0.1};
		double currentRotationTime{0.1};
		vector<string> availableSkills;

		// Generate available skills before rotation starts
		for (const auto& skill : skills) {
			if (primaryAttunement[0] == skill.mainAttunement && skill.secondaryAttunement.empty()) {
				availableSkills.push_back(skill.ID);
			}
			if (secondaryAttunement[0] == skill.secondaryAttunement && skill.mainAttunement.empty()) {
				availableSkills.push_back(skill.ID);
			}
			if (primaryAttunement[0] == skill.mainAttunement && secondaryAttunement[0] == skill.secondaryAttunement) {
				availableSkills.push_back(skill.ID);
			}
			if (skill.skillType == "Dual Attack" && ((primaryAttunement[0] == skill.mainAttunement && secondaryAttunement[0] == skill.secondaryAttunement) || (primaryAttunement[0] == skill.secondaryAttunement && secondaryAttunement[0] == skill.mainAttunement))) {
				availableSkills.push_back(skill.ID);
			}
		}

		availableSkills.erase(
			remove_if(availableSkills.begin(), availableSkills.end(),
				[&](const string& skillID) {
					return ((primaryAttunement[0] == "Fire" && (skillID == "FireSwipe" || skillID == "SearingSlash")) ||
							(primaryAttunement[0] == "Water" && (skillID == "Clapotis" || skillID == "BreakingWave")) ||
							(primaryAttunement[0] == "Air" && (skillID == "PolaricSlash" || skillID == "CallLightning")) ||
							(primaryAttunement[0] == "Earth" && (skillID == "CrystallineStrike" || skillID == "CrystallineSunder")));
				}),
			availableSkills.end());

		sort(availableSkills.begin(), availableSkills.end());
		availableSkills.erase(unique(availableSkills.begin(), availableSkills.end()), availableSkills.end());

		////////////////////////////////////////// Generate random skill rotation based on attunement rotation //////////////////////////////////////////
		int i = 1;
		Skill selectedSkill{};
		rotation.push_back("WeaveSelf");
		map<string, double> cooldownTimes;

		while (currentRotationTime <= attunementTime.back()){
			while (currentRotationTime <= attunementTime[i]) {

				map<string, double> skillWeights = {
				{"FireStrike", 1.0},
				{"FireSwipe", 10.0},
				{"SearingSlash", 10.0},
				{"Seiche", 1.0},
				{"Clapotis", 5.0},
				{"BreakingWave", 5.0},
				{"ChargedStrike", 10.0},
				{"PolaricSlash", 20.0},
				{"CallLightning", 50.0},
				{"CrystalSlash", 1.0},
				{"CrystallineStrike", 5.0},
				{"CrystallineSunder", 5.0},
				{"FlameUprising", 30.00},
				{"CauterizingStrike", 20.0},
				{"Wildfire", 30.0},
				{"TidalSurge", 20.0},
				{"QuantumStrike", 10.0},
				{"Cyclone", 10.0},
				{"LightningOrb", 30.0},
				{"EarthenVortex", 10.0},
				{"RustFrenzy", 10.0},
				{"DustStorm", 30.0},
				{"TwinStrike", 20.0},
				{"PyroVortex", 10.0},
				{"LavaSkin", 10.0},
				{"ShearingEdge", 10.0},
				{"NaturalFrenzy", 10.0},
				{"GaleStrike", 10.0},
				{"Firestorm", 30.0},
				{"LightningStorm", 30.0}
				};

				if (primaryAttunement[i-1] == "Air" && secondaryAttunement[i-1] == "Fire") {
					skillWeights["PyroVortex"] = 1.0;
				}

				if (availableSkills.size() == 0) {
					if (primaryAttunement[i-1] == "Fire") {
						availableSkills = {"FireStrike"};
					} else if (primaryAttunement[i-1] == "Water") {
						availableSkills = {"Seiche"};
					} else if (primaryAttunement[i-1] == "Air") {
						availableSkills = {"ChargedStrike"};
					} else if (primaryAttunement[i-1] == "Earth") {
						availableSkills = {"CrystalSlash"};
					}
				}

				string randomSkillID = weightedRandomSelection(availableSkills, skillWeights);

				for (const auto& skill : skills) {
					if (skill.getID() == randomSkillID) {
						selectedSkill = skill;
						break;
					}
				}

				availableSkills.clear();			

				for (const auto& skill : skills) {
					if (primaryAttunement[i-1] == skill.mainAttunement && skill.secondaryAttunement.empty()) {
						availableSkills.push_back(skill.ID);
					}
					if (secondaryAttunement[i-1] == skill.secondaryAttunement && skill.mainAttunement.empty()) {
						availableSkills.push_back(skill.ID);
					}
					if (primaryAttunement[i-1] == skill.mainAttunement && secondaryAttunement[i-1] == skill.secondaryAttunement) {
						availableSkills.push_back(skill.ID);
					}
					if (skill.skillType == "Dual Attack" && ((primaryAttunement[i-1] == skill.mainAttunement && secondaryAttunement[i-1] == skill.secondaryAttunement) || (primaryAttunement[i-1] == skill.secondaryAttunement && secondaryAttunement[i-1] == skill.mainAttunement))) {
						availableSkills.push_back(skill.ID);
					}
				}

				availableSkills.erase(
					remove_if(availableSkills.begin(), availableSkills.end(),
							[&](const string& skillID) {
								for (const auto& skill : skills) {
									if (skill.getID() == skillID && skill.getSkillType() == "Basic Attack") {
										return true;
									}
								}
								return false;
							}),
					availableSkills.end());

				if (selectedSkill.getID() == "FireStrike") {
					availableSkills.push_back("FireSwipe");
				} else if (selectedSkill.getID() == "FireSwipe") {
					availableSkills.push_back("SearingSlash");
				} else if (primaryAttunement[i-1] == "Fire" && (selectedSkill.getID() == "SearingSlash" || selectedSkill.getSkillType() != "Basic Attack")) {
					availableSkills.push_back("FireStrike");
				}

				if (selectedSkill.getID() == "Seiche") {
					availableSkills.push_back("Clapotis");
				} else if (selectedSkill.getID() == "Clapotis") {
					availableSkills.push_back("BreakingWave");
				} else if (primaryAttunement[i-1] == "Water" && (selectedSkill.getID() == "BreakingWave" || selectedSkill.getSkillType() != "Basic Attack")) {
					availableSkills.push_back("Seiche");
				}

				if (selectedSkill.getID() == "ChargedStrike") {
					availableSkills.push_back("PolaricSlash");
				} else if (selectedSkill.getID() == "PolaricSlash") {
					availableSkills.push_back("CallLightning");
				} else if (primaryAttunement[i-1] == "Air" && (selectedSkill.getID() == "CallLightning" || selectedSkill.getSkillType() != "Basic Attack")) {
					availableSkills.push_back("ChargedStrike");
				}

				if (selectedSkill.getID() == "CrystalSlash") {
					availableSkills.push_back("CrystallineStrike");
				} else if (selectedSkill.getID() == "CrystallineStrike") {
					availableSkills.push_back("CrystallineSunder");
				} else if (primaryAttunement[i-1] == "Earth" && (selectedSkill.getID() == "CrystallineSunder" || selectedSkill.getSkillType() != "Basic Attack")) {
					availableSkills.push_back("CrystalSlash");
				}

				sort(availableSkills.begin(), availableSkills.end());
				availableSkills.erase(unique(availableSkills.begin(), availableSkills.end()), availableSkills.end());

				if (selectedSkill.ID == "Firestorm") {
					cooldownTimes["Firestorm"] = currentRotationTime + selectedSkill.cooldown;
					cooldownTimes["LightningStorm"] = currentRotationTime + selectedSkill.cooldown;
				} else if (selectedSkill.ID == "LightningStorm") {
					cooldownTimes["Firestorm"] = currentRotationTime + selectedSkill.cooldown;
					cooldownTimes["LightningStorm"] = currentRotationTime + selectedSkill.cooldown;
				} else {
					cooldownTimes[selectedSkill.ID] = currentRotationTime + selectedSkill.cooldown;
				}

				for (const auto& skill : skills) {
					if (cooldownTimes[skill.ID] > currentRotationTime) {
						// Skill is on cooldown, remove it from availableSkills
						auto it = remove(availableSkills.begin(), availableSkills.end(), skill.ID);
						availableSkills.erase(it, availableSkills.end());
					}
				}

				currentRotationTime += selectedSkill.getAnimationTime();
				rotation.push_back(selectedSkill.getID());
				rotationTime.push_back(currentRotationTime);			
			} // End of loop through available skills in given attunement
			// Update attunements
			i += 1;

			availableSkills.clear();

			for (const auto& skill : skills) {
				if (primaryAttunement[i-1] == skill.mainAttunement && skill.secondaryAttunement.empty()) {
					availableSkills.push_back(skill.ID);
				}
				if (secondaryAttunement[i-1] == skill.secondaryAttunement && skill.mainAttunement.empty()) {
					availableSkills.push_back(skill.ID);
				}
				if (primaryAttunement[i-1] == skill.mainAttunement && secondaryAttunement[i-1] == skill.secondaryAttunement) {
					availableSkills.push_back(skill.ID);
				}
				if (skill.skillType == "Dual Attack" && ((primaryAttunement[i-1] == skill.mainAttunement && secondaryAttunement[i-1] == skill.secondaryAttunement) || (primaryAttunement[i-1] == skill.secondaryAttunement && secondaryAttunement[i-1] == skill.mainAttunement))) {
					availableSkills.push_back(skill.ID);
				}
			}

			availableSkills.erase(
				remove_if(availableSkills.begin(), availableSkills.end(),
						[&](const string& skillID) {
							for (const auto& skill : skills) {
								if (skill.getID() == skillID && skill.getSkillType() == "Basic Attack") {
									return true;
								}
							}
							return false;
						}),
				availableSkills.end());
			
			if (selectedSkill.getID() == "FireStrike") {
				availableSkills.push_back("FireSwipe");
			} else if (selectedSkill.getID() == "FireSwipe") {
				availableSkills.push_back("SearingSlash");
			} else if (selectedSkill.getID() == "Seiche") {
				availableSkills.push_back("Clapotis");
			} else if (selectedSkill.getID() == "Clapotis") {
				availableSkills.push_back("BreakingWave");
			} else if (selectedSkill.getID() == "ChargedStrike") {
				availableSkills.push_back("PolaricSlash");
			} else if (selectedSkill.getID() == "PolaricSlash") {
				availableSkills.push_back("CallLightning");
			} else if (selectedSkill.getID() == "CrystalSlash") {
				availableSkills.push_back("CrystallineStrike");
			} else if (selectedSkill.getID() == "CrystallineStrike") {
				availableSkills.push_back("CrystallineSunder");
			}

			if (primaryAttunement[i-1] == "Fire" && selectedSkill.getID() != "FireStrike") {
				availableSkills.push_back("FireStrike");
			} else if (primaryAttunement[i-1] == "Water" && selectedSkill.getID() != "Seiche") {
				availableSkills.push_back("Seiche");
			} else if (primaryAttunement[i-1] == "Air" && selectedSkill.getID() != "ChargedStrike") {
				availableSkills.push_back("ChargedStrike");
			} else if (primaryAttunement[i-1] == "Earth" && selectedSkill.getID() != "CrystalSlash") {
				availableSkills.push_back("CrystalSlash");
			}

			sort(availableSkills.begin(), availableSkills.end());
			availableSkills.erase(unique(availableSkills.begin(), availableSkills.end()), availableSkills.end());

			for (const auto& skill : skills) {
				if (cooldownTimes[skill.ID] > currentRotationTime) {
					// Skill is on cooldown, remove it from availableSkills
					auto it = remove(availableSkills.begin(), availableSkills.end(), skill.ID);
					availableSkills.erase(it, availableSkills.end());
				}
			}
		}

		// Adding attunements to rotation (skills)
		for (size_t i = 1 ; i < primaryAttunement.size() ; i++) {
			if (primaryAttunement[i] == "Fire") {
				rotation.push_back("FireAttunement");
			} else if (primaryAttunement[i] == "Water") {
				rotation.push_back("WaterAttunement");
			} else if (primaryAttunement[i] == "Air") {
				rotation.push_back("AirAttunement");
			} else if (primaryAttunement[i] == "Earth") {
				rotation.push_back("EarthAttunement");
			}
			rotationTime.push_back(attunementTime[i]);
		}

		vector<size_t> indices(rotationTime.size());
		iota(indices.begin(), indices.end(), 0);
		sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
			return rotationTime[a] < rotationTime[b];
		});
		vector<string> sortedRotation(rotation.size());
		vector<double> sortedRotationTime(rotationTime.size());
		for (size_t i = 0; i < indices.size(); ++i) {
			sortedRotation[i] = rotation[indices[i]];
			sortedRotationTime[i] = rotationTime[indices[i]];
		}
		rotation = sortedRotation;
		rotationTime = sortedRotationTime;

		///////////////////////////////////////////////////////////// DPS CALCULATIONS BELOW /////////////////////////////////////////

		// Initialize necessary variables
		vector<double> timeOfDamageInstances{};
		vector<double> damageOfDamageInstances{};
		vector<string> nameOfDamageInstances{};
		vector<double> timeOfCondiDamageInstance{};
		vector<double> damageOfCondiDamageInstance{};
		vector<string> nameOfCondiDamageInstance{};
		double forceSigil{0.05};
		double impactSigil{0.03};
		double pyromancersTraining{0.1};
		double stormsoul{0.1};
		double swiftRevenge{0.1};
		double vulnerablity{0.25};
		double btth{0.1};
		vector<double> permaAdditiveMods{swiftRevenge, forceSigil, impactSigil};
		vector<double> permaMultiplicativeMods{pyromancersTraining, stormsoul, vulnerablity, btth};
		vector<double> permaAdditiveCondiMods{};
		vector<double> permaMultiplicativeCondiMods{vulnerablity};
		vector<double> additiveCondiMods{};
		vector<double> multiplicativeCondiMods{};
		vector<double> additiveMods{};
		vector<double> multiplicativeMods{};
		deque<pair<double, double>> persistingFlamesStacks;
		double burningPrecisionCooldown{0.0};


		for (size_t i = 0 ; i < rotation.size() ; i++) {
			// Find THE skill
			const string& skillID = rotation[i];
			const Skill* skill = nullptr;
			for (const auto& s : skills) {
				if (s.getID() == skillID) {
					skill = &s;
					break;
				}
			}
			if (skill && originalCooldowns[skill->getID()] >= 20 && (skill->skillWeapon == "Sword" || skill->skillWeapon == "Warhorn")) {
				fireworksStart.push_back(rotationTime[i]);
				fireworksFinish.push_back(rotationTime[i] + 6);
			}

			// Get weapon strength
			double weaponStrength{0.0};
			if (skill->skillWeapon == "Sword") {
				weaponStrength = 1000.0;
			} else if (skill->skillWeapon == "Warhorn") {
				weaponStrength = 900.0;
			} else {
				weaponStrength = 690.5;
			}

			size_t attunementIndex{0};
			////////////////////////////////////////////////////////////////////// DAMAGE INSTANCE LOOP /////////////////////////////////////////////////////////////////
			for (const auto& damageInstance : skill->damageInstances) {
				double adjustedTime = rotationTime[i] - skill->animationTime + damageInstance.time;

				// Find attunements at time of damage instance to uppdate attributes
				for (size_t j = 0 ; j < attunementTime.size() ; ++j) {
					if (attunementTime[j] <= adjustedTime) {
						attunementIndex = j;
					} else {
						break;
					}
				}
				string primaryAttunementInstance = primaryAttunement[attunementIndex];
				string secondaryAttunementInstance = secondaryAttunement[attunementIndex];

				double power = player.getAttribute("power");
				double precision = player.getAttribute("precision");
				double ferocity = player.getAttribute("ferocity");

				// Handle attributes from attunements
				if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Fire") {
					power += 420;
				} else if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Air") {
					power += 420;
					ferocity += 120;
				} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Fire") {
					power += 120;
					ferocity += 270;
				} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Air") {
					ferocity += 270;
				} else if (primaryAttunementInstance == "Fire" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
					power += 420;
				} else if (primaryAttunementInstance == "Air" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
					ferocity += 270;
				} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Fire") {
					power += 120;
				} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Air") {
					ferocity += 120;
				}

				double critChance = (precision - 895) / 2100 + 0.05 + 0.15 + 0.25;
				if (critChance > 1.0) {
					critChance = 1.0;
				}
				double critDamage = ferocity / 1500 + 1.5;

				timeOfDamageInstances.push_back(adjustedTime);
				nameOfDamageInstances.push_back(damageInstance.ID);
				
				double damage{0.0};
				if (damageInstance.ID == "FireAttunement") {
					damage = weaponStrength * power * damageInstance.damageCoefficient / targetArmor;
				} else if (damageInstance.ID == "AirAttunement") {
					damage = (critChance * (critDamage+1) + (1-critChance)) * weaponStrength * power * damageInstance.damageCoefficient / targetArmor;
				} else damage = (critChance * critDamage + (1-critChance)) * weaponStrength * power * damageInstance.damageCoefficient / targetArmor;

				damageOfDamageInstances.push_back(damage);
				
				if (damageInstance.fireFieldDamage) {
					double persistingFlamesStart = adjustedTime;
					double persistingFlamesEnd = adjustedTime + 15.0;
					persistingFlamesStacks.push_back({persistingFlamesStart, persistingFlamesEnd});
				}

				/////////////////////////////////////////////// CONDI INSTANCES HERE ///////////////////////////////////////////

				double burningDuration = player.getConditionDuration("burning");
				double bleedingDuration = player.getConditionDuration("bleeding");
				double condiDamage = player.getAttribute("conditionDamage");
				double damage_condi{0.0};

				if (damageInstance.condition == Condition::burning) {
					// Calculate the adjusted condition duration
					double adjustedConditionDuration = damageInstance.condiDurations * (1 + burningDuration);

					// Calculate the number of times to push back the adjusted time
					int ceilAdjustedConditionDuration = ceil(adjustedConditionDuration);

					// Push back adjusted times to timeOfCondiDamageInstance
					for (int j = 1; j < (ceilAdjustedConditionDuration+1); ++j) {
						timeOfCondiDamageInstance.push_back(j + adjustedTime);
						if (j == ceilAdjustedConditionDuration && ceilAdjustedConditionDuration-adjustedConditionDuration > 0) {
							damage_condi = damageInstance.condiStacks * (131 + (condiDamage * 0.155)) * (ceilAdjustedConditionDuration - adjustedConditionDuration);
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back(damageInstance.ID + " - burning");
						} else {
							damage_condi = damageInstance.condiStacks * (131 + (condiDamage * 0.155));
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back(damageInstance.ID + " - burning");
						}
					}
				}
				
				// Burning Precision
				if (damageInstance.damageCoefficient != 0.0 && adjustedTime > burningPrecisionCooldown) {
					// Check if Burning Precision procs (33% chance)
					if (rand() % 3 == 0) {
						double adjustedConditionDuration = 3 * (1 + burningDuration);
						int ceilAdjustedConditionDuration = ceil(adjustedConditionDuration);
						for (int j = 1; j < (ceilAdjustedConditionDuration+1); ++j) {
							timeOfCondiDamageInstance.push_back(j + adjustedTime);
							if (j == ceilAdjustedConditionDuration && ceilAdjustedConditionDuration-adjustedConditionDuration > 0) {
								damage_condi = (131 + (condiDamage * 0.155)) * (ceilAdjustedConditionDuration - adjustedConditionDuration);
								damageOfCondiDamageInstance.push_back(damage_condi);
								nameOfCondiDamageInstance.push_back("BurningPrecision");
							} else {
								damage_condi = (131 + (condiDamage * 0.155));
								damageOfCondiDamageInstance.push_back(damage_condi);
								nameOfCondiDamageInstance.push_back("BurningPrecision");
							}
						}
						burningPrecisionCooldown = adjustedTime + 5;
					}
				}

				if (damageInstance.condition == Condition::bleeding) {
					// Calculate the adjusted condition duration
					double adjustedConditionDuration = damageInstance.condiDurations * (1 + bleedingDuration);

					// Calculate the number of times to push back the adjusted time
					int ceilAdjustedConditionDuration = ceil(adjustedConditionDuration);

					// Push back adjusted times to timeOfCondiDamageInstance
					for (int j = 1; j < (ceilAdjustedConditionDuration+1); ++j) {
						timeOfCondiDamageInstance.push_back(j + adjustedTime);
						if (j == ceilAdjustedConditionDuration && ceilAdjustedConditionDuration-adjustedConditionDuration > 0) {
							damage_condi = damageInstance.condiStacks * (22 + (condiDamage * 0.06)) * (ceilAdjustedConditionDuration - adjustedConditionDuration);
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back(damageInstance.ID + " - bleeding");
						} else {
							damage_condi = damageInstance.condiStacks * (22 + (condiDamage * 0.06));
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back(damageInstance.ID + " - bleeding");
						}
					}
				}
			}
		}
		// PRIMORDIAL STANCE - manual declaration of times
		vector<double> primordialStanceTiming{4.0, 11.9, 22.5, 36.6, 50.9};

		for (const auto& primordialStanceTime : primordialStanceTiming) {
			for (int i = 0; i < 5; ++i) {
				double currentTime = primordialStanceTime + i + 1;
				size_t attunementIndex = 0;
				for (size_t j = 0; j < attunementTime.size(); ++j) {
					if (attunementTime[j] <= currentTime) {
						attunementIndex = j;
					} else {
						break;
					}
				}
				string primaryAttunementInstance = primaryAttunement[attunementIndex];
				string secondaryAttunementInstance = secondaryAttunement[attunementIndex];

				double power = player.getAttribute("power");
				double precision = player.getAttribute("precision");
				double ferocity = player.getAttribute("ferocity");

				// Handle attributes from attunements
				if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Fire") {
					power += 420;
				} else if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Air") {
					power += 420;
					ferocity += 120;
				} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Fire") {
					power += 120;
					ferocity += 270;
				} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Air") {
					ferocity += 270;
				} else if (primaryAttunementInstance == "Fire" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
					power += 420;
				} else if (primaryAttunementInstance == "Air" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
					ferocity += 270;
				} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Fire") {
					power += 120;
				} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Air") {
					ferocity += 120;
				}

				double critChance = (precision - 895) / 2100 + 0.05 + 0.15 + 0.25;
				if (critChance > 1.0) {
					critChance = 1.0;
				}
				double critDamage = ferocity / 1500 + 1.5;

				double damage{0.0};
				damage = (critChance * critDamage + (1-critChance)) * 690.5 * power * 0.33 / targetArmor;
				
				timeOfDamageInstances.push_back(currentTime);
				nameOfDamageInstances.push_back("PrimordialStance " + primaryAttunementInstance + " / " + secondaryAttunementInstance);
				damageOfDamageInstances.push_back(damage);

				double burningDuration = player.getConditionDuration("burning");
				double bleedingDuration = player.getConditionDuration("bleeding");
				double condiDamage = player.getAttribute("conditionDamage");
				double damage_condi{0.0};

				if (primaryAttunementInstance == "Fire" || secondaryAttunementInstance == "Fire") {
					double stackMultiplier = (primaryAttunementInstance == secondaryAttunementInstance) ? 2 : 1;
					double adjustedConditionDuration = 2 * (1 + burningDuration);

					// Calculate the number of times to push back the adjusted time
					int ceilAdjustedConditionDuration = ceil(adjustedConditionDuration);

					// Push back adjusted times to timeOfCondiDamageInstance
					for (int j = 1; j < (ceilAdjustedConditionDuration+1); ++j) {
						timeOfCondiDamageInstance.push_back(j + currentTime);
						if (j == ceilAdjustedConditionDuration && ceilAdjustedConditionDuration-adjustedConditionDuration > 0) {
							damage_condi = stackMultiplier * (131 + (condiDamage * 0.155)) * (ceilAdjustedConditionDuration - adjustedConditionDuration);
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back("PrimordialStance - burning");
						} else {
							damage_condi = stackMultiplier * (131 + (condiDamage * 0.155));
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back("PrimordialStance - burning");
						}
					}
				} else if (primaryAttunementInstance == "Earth" || secondaryAttunementInstance == "Earth") {
					double stackMultiplier = (primaryAttunementInstance == secondaryAttunementInstance) ? 2 : 1;
					double adjustedConditionDuration = 6 * (1 + bleedingDuration);

					// Calculate the number of times to push back the adjusted time
					int ceilAdjustedConditionDuration = ceil(adjustedConditionDuration);

					// Push back adjusted times to timeOfCondiDamageInstance
					for (int j = 1; j < (ceilAdjustedConditionDuration+1); ++j) {
						timeOfCondiDamageInstance.push_back(j + currentTime);
						if (j == ceilAdjustedConditionDuration && ceilAdjustedConditionDuration-adjustedConditionDuration > 0) {
							damage_condi = stackMultiplier * 2 * (22 + (condiDamage * 0.06)) * (ceilAdjustedConditionDuration - adjustedConditionDuration);
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back("PrimordialStance - bleeding");
						} else {
							damage_condi = stackMultiplier * 2 * (22 + (condiDamage * 0.06));
							damageOfCondiDamageInstance.push_back(damage_condi);
							nameOfCondiDamageInstance.push_back("PrimordialStance - bleeding");
						}
					}
				}


			}
		}
		// ARCANE BLAST - manual declaration of times
		vector<double> arcaneBlastTimings{4.0, 5.0, 15.0, 37.0, 38.0, 52.0};

		for (const auto& arcaneBlastTime : arcaneBlastTimings) {
			size_t attunementIndex = 0;
			for (size_t j = 0 ; j < attunementTime.size() ; ++j) {
				if (attunementTime[j] <= arcaneBlastTime) {
					attunementIndex = j;
				} else {
					break;
				}
			}
			string primaryAttunementInstance = primaryAttunement[attunementIndex];
			string secondaryAttunementInstance = secondaryAttunement[attunementIndex];

			double power = player.getAttribute("power");
			double precision = player.getAttribute("precision");
			double ferocity = player.getAttribute("ferocity");

			// Handle attributes from attunements
			if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Fire") {
				power += 420;
			} else if (primaryAttunementInstance == "Fire" && secondaryAttunementInstance == "Air") {
				power += 420;
				ferocity += 120;
			} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Fire") {
				power += 120;
				ferocity += 270;
			} else if (primaryAttunementInstance == "Air" && secondaryAttunementInstance == "Air") {
				ferocity += 270;
			} else if (primaryAttunementInstance == "Fire" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
				power += 420;
			} else if (primaryAttunementInstance == "Air" && (secondaryAttunementInstance == "Water" || secondaryAttunementInstance == "Earth")) {
				ferocity += 270;
			} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Fire") {
				power += 120;
			} else if ((primaryAttunementInstance == "Water" || primaryAttunementInstance == "Earth") && secondaryAttunementInstance == "Air") {
				ferocity += 120;
			}

			double critChance = (precision - 895) / 2100 + 0.05 + 0.15 + 0.25;
			if (critChance > 1.0) {
				critChance = 1.0;
			}
			double critDamage = ferocity / 1500 + 1.5;

			timeOfDamageInstances.push_back(arcaneBlastTime);
			nameOfDamageInstances.push_back("Arcane Blast");
				
			double damage{0.0};
			damage = (critChance * critDamage + (1-critChance)) * 690.5 * power * 1.4 / targetArmor;
			damageOfDamageInstances.push_back(damage);
		}
		// MERGING, SORTING AND OUTPUTTING DAMAGE INSTANCES
		vector<tuple<double, string, double>> zipped;
		for (size_t i = 0; i < timeOfDamageInstances.size(); ++i) {
			zipped.push_back(make_tuple(timeOfDamageInstances[i], nameOfDamageInstances[i], damageOfDamageInstances[i]));
		}
		sort(zipped.begin(), zipped.end());

		vector<tuple<double, string, double>> zippedCondi;
		for (size_t i = 0; i < timeOfCondiDamageInstance.size(); ++i) {
			zippedCondi.push_back(make_tuple(timeOfCondiDamageInstance[i], nameOfCondiDamageInstance[i], damageOfCondiDamageInstance[i]));
		}
		sort(zippedCondi.begin(), zippedCondi.end());

		// Clear the original vectors
		timeOfDamageInstances.clear();
		nameOfDamageInstances.clear();
		damageOfDamageInstances.clear();

		timeOfCondiDamageInstance.clear();
		nameOfCondiDamageInstance.clear();
		damageOfCondiDamageInstance.clear();

		// Unzip the sorted vector back into the original vectors
		for (const auto& instance : zipped) {
			double time;
			string name;
			double damage;
			tie(time, name, damage) = instance;
			timeOfDamageInstances.push_back(time);
			nameOfDamageInstances.push_back(name);
			damageOfDamageInstances.push_back(damage);
		}

		for (const auto& instance : zippedCondi) {
			double time;
			string name;
			double damage;
			tie(time, name, damage) = instance;
			timeOfCondiDamageInstance.push_back(time);
			nameOfCondiDamageInstance.push_back(name);
			damageOfCondiDamageInstance.push_back(damage);
		}

		// LOOP THAT UPDATES DAMAGE AND HANDLES MODS
		for (size_t i = 0 ; i < damageOfCondiDamageInstance.size() ; ++i) {
			additiveCondiMods.clear();
			multiplicativeCondiMods.clear();
			double currentTime = timeOfCondiDamageInstance[i];

			if (currentTime >= wovenFireStart && currentTime <= wovenFireFinish) {
				additiveCondiMods.push_back(0.2);
			}

			for (size_t j = 0; j < elementsOfRageStarts.size(); ++j) {
				if (currentTime >= elementsOfRageStarts[j] && currentTime <= elementsOfRageFinishes[j]) {
					additiveCondiMods.push_back(0.07);
				}
			}

			for (double mod : permaAdditiveCondiMods) {
				additiveCondiMods.push_back(mod);
			}
			for (double mod : permaMultiplicativeCondiMods) {
				multiplicativeCondiMods.push_back(mod);
			}

			double condiMod{0.0};
			condiMod = addMods(additiveCondiMods) * multiMods(multiplicativeCondiMods);

			damageOfCondiDamageInstance[i] *= condiMod;
		}

		for (size_t i = 0 ; i < damageOfDamageInstances.size() ; ++i) {
			additiveMods.clear();
			multiplicativeMods.clear();
			double currentTime = timeOfDamageInstances[i];
			int activeStacks = 0;

			for (const auto& stack : persistingFlamesStacks) {
				double stackStart = stack.first;
				double stackEnd = stack.second;

				// If the current time is within the duration of the persisting flames stack, increment the activeStacks count
				if (currentTime >= stackStart && currentTime <= stackEnd) {
					activeStacks++;
				}
			}
			if (activeStacks > 10) {
				activeStacks = 10;
			}

			for (int j = 0; j < activeStacks; ++j) {
				additiveMods.push_back(0.01);
			}

			if (currentTime >= wovenAirStart && currentTime <= wovenAirFinish) {
				additiveMods.push_back(0.1);
			}

			for (size_t j = 0; j < elementsOfRageStarts.size(); ++j) {
				if (currentTime >= elementsOfRageStarts[j] && currentTime <= elementsOfRageFinishes[j]) {
					additiveMods.push_back(0.07);
				}
			}

			for (size_t j = 0; j < fireworksStart.size(); ++j) {
				if (currentTime >= fireworksStart[j] && currentTime <= fireworksFinish[j]) {
					multiplicativeMods.push_back(0.07);
				}
			}

			for (double mod : permaAdditiveMods) {
				additiveMods.push_back(mod);
			}
			for (double mod : permaMultiplicativeMods) {
				multiplicativeMods.push_back(mod);
			}

			double powerMod{0.0};
			powerMod = addMods(additiveMods) * multiMods(multiplicativeMods);
			damageOfDamageInstances[i] *= powerMod;
		}

		vector<tuple<double, string, double>> mergedDamage;
		for (size_t i = 0; i < timeOfDamageInstances.size(); ++i) {
			mergedDamage.push_back(make_tuple(timeOfDamageInstances[i], nameOfDamageInstances[i], damageOfDamageInstances[i]));
		}
		for (size_t i = 0; i < timeOfCondiDamageInstance.size(); ++i) {
			mergedDamage.push_back(make_tuple(timeOfCondiDamageInstance[i], nameOfCondiDamageInstance[i], damageOfCondiDamageInstance[i]));
		}
		sort(mergedDamage.begin(), mergedDamage.end());

		vector<double> timeOfDamageInstancesFinal;
		vector<string> nameOfDamageInstanceFinal;
		vector<double> damageOfDamageInstancesFinal;

		for (const auto& instance : mergedDamage) {
			double time;
			string name;
			double damage;
			tie(time, name, damage) = instance;
			timeOfDamageInstancesFinal.push_back(time);
			nameOfDamageInstanceFinal.push_back(name);
			damageOfDamageInstancesFinal.push_back(damage);
		}

		// Trim values for time > 60
		for (size_t i = 0; i < timeOfDamageInstancesFinal.size(); ++i) {
			if (timeOfDamageInstancesFinal[i] > 60.0) {
				// Remove the corresponding elements from all vectors
				timeOfDamageInstancesFinal.erase(timeOfDamageInstancesFinal.begin() + i);
				nameOfDamageInstanceFinal.erase(nameOfDamageInstanceFinal.begin() + i);
				damageOfDamageInstancesFinal.erase(damageOfDamageInstancesFinal.begin() + i);

				// Since we erased an element, decrement i to stay at the same index
				--i;
			}
		}

		vector<double> DPS(timeOfDamageInstancesFinal.size(), 0.0);
		calculateDPS(timeOfDamageInstancesFinal, damageOfDamageInstancesFinal, DPS);
		double finalDPS = DPS.back();

		cout << "Final DPS: " << finalDPS << endl;

		// If current rotation/iteration is better, overwrite old one
		if (finalDPS > bestFinalDPS) {
			bestDamageOfDamageInstances.assign(damageOfDamageInstancesFinal.begin(), damageOfDamageInstancesFinal.end());
			bestNameOfDamageInstances.assign(nameOfDamageInstanceFinal.begin(), nameOfDamageInstanceFinal.end());
			bestTimeOfDamageInstances.assign(timeOfDamageInstancesFinal.begin(), timeOfDamageInstancesFinal.end());
			bestDPS.assign(DPS.begin(), DPS.end());
			bestFinalDPS = finalDPS;
	 		bestPrimaryAttunement = primaryAttunement;
			bestSecondaryAttunement = secondaryAttunement;
			bestAttunementTime = attunementTime;
			bestRotation = rotation;
			bestRotationTime = rotationTime;
		}
	}

	cout << "Best DPS: " << bestFinalDPS << endl;
	writeToCSV(bestTimeOfDamageInstances, bestNameOfDamageInstances, bestDamageOfDamageInstances, bestDPS);

	ofstream file("outputRotation.csv");
	file << "Rotation \t Rotation Time \n";
	for (size_t i = 0; i < bestRotation.size(); i++) {
		file << bestRotation[i] << "\t" << bestRotationTime[i] << "\n";
	}
	file.close();

	// Print bestRotation
	cout << "Best Rotation :" << endl;
	for (size_t i = 0; i < bestRotation.size(); ++i) {
		cout << bestRotationTime[i] << "\t" << bestRotation[i] << "\n";
	}
	cout << endl;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Runtime: " << duration.count() << " seconds" << endl;

	return 0;
}