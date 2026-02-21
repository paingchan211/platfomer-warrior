#include "SaveGameManager.h"
#include "Player.h"
#include "SkillTree.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path) // Define MKDIR for Windows
#else
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755) // Define MKDIR for Unix/Linux
#endif

// Constructor initializes the saves directory
SaveGameManager::SaveGameManager()
    : savesDirectory_("saves")
{
    ensureSavesDirectoryExists(); // Ensure the save directory exists
}

// Save the game data to a file
bool SaveGameManager::saveGame(const Player &player, const std::string &saveName)
{
    std::string filepath = getSaveFilePath(saveName); // Get full file path

    std::ofstream file(filepath); // Open file for writing
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for saving: " << filepath << std::endl;
        return false; // Return false if file cannot be opened
    }

    // Write save file version
    file << "[SAVE_FILE_VERSION]\n";
    file << "1\n\n";

    // Write player stats section
    file << "[PLAYER_STATS]\n";
    savePlayerStats(player, file);
    file << "\n";

    // Write skill tree section
    file << "[SKILL_TREE]\n";
    saveSkillTree(player.getSkillTree(), file);
    file << "\n";

    // Write player HP section
    file << "[PLAYER_HP]\n";
    file << player.getHp() << "\n";
    file << player.getMaxHp() << "\n\n";

    // Write player damage
    file << "[PLAYER_DAMAGE]\n";
    file << player.getDamage() << "\n\n";

    // Write inventory section
    file << "[PLAYER_INVENTORY]\n";
    file << "HpPotions=" << player.getHpPotions() << "\n\n";

    file.close(); // Close file
    std::cout << "Game saved successfully to: " << filepath << std::endl;
    return true;
}

// Load the game data from a file
bool SaveGameManager::loadGame(Player &player, const std::string &saveName)
{
    std::string filepath = getSaveFilePath(saveName); // Get full file path

    std::ifstream file(filepath); // Open file for reading
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for loading: " << filepath << std::endl;
        return false; // Return false if file not found
    }

    std::string line;
    // Read file line by line
    while (std::getline(file, line))
    {
        // Parse each section header
        if (line == "[SAVE_FILE_VERSION]")
        {
            int version;
            file >> version;
            std::getline(file, line); // Consume the newline
        }
        else if (line == "[PLAYER_STATS]")
        {
            if (!loadPlayerStats(player, file))
            {
                std::cerr << "Failed to load player stats" << std::endl;
                file.close();
                return false;
            }
        }
        else if (line == "[SKILL_TREE]")
        {
            if (!loadSkillTree(player.getSkillTree(), file))
            {
                std::cerr << "Failed to load skill tree" << std::endl;
                file.close();
                return false;
            }
        }
        else if (line == "[PLAYER_HP]")
        {
            int currentHP, maxHP;
            file >> currentHP >> maxHP;
            std::getline(file, line); // Consume newline
            player.setMaxHp(maxHP);
            player.setHp(currentHP);
        }
        else if (line == "[PLAYER_DAMAGE]")
        {
            int damage;
            file >> damage;
            std::getline(file, line);
            player.setDamage(damage);
        }
        else if (line == "[PLAYER_INVENTORY]")
        {
            std::string invLine;
            // Read inventory items until new section starts
            while (std::getline(file, invLine))
            {
                if (invLine.empty() || invLine[0] == '[')
                {
                    break; // Exit on section header or empty line
                }

                size_t pos = invLine.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = invLine.substr(0, pos);
                    std::string value = invLine.substr(pos + 1);

                    if (key == "HpPotions")
                    {
                        int potionCount = std::stoi(value);
                        player.setHpPotions(potionCount);
                    }
                }
            }

            // If we stopped at a new section, process it
            if (!invLine.empty() && invLine[0] == '[')
            {
                if (invLine == "[SKILL_TREE]")
                {
                    if (!loadSkillTree(player.getSkillTree(), file))
                    {
                        std::cerr << "Failed to load skill tree" << std::endl;
                        file.close();
                        return false;
                    }
                }
                else if (invLine == "[PLAYER_HP]")
                {
                    int currentHP, maxHP;
                    file >> currentHP >> maxHP;
                    std::getline(file, line);
                    player.setMaxHp(maxHP);
                    player.setHp(currentHP);
                }
                else if (invLine == "[PLAYER_DAMAGE]")
                {
                    int damage;
                    file >> damage;
                    std::getline(file, line);
                    player.setDamage(damage);
                }
            }
        }
    }

    file.close();
    std::cout << "Game loaded successfully from: " << filepath << std::endl;
    return true;
}

// Check if a save file exists
bool SaveGameManager::saveExists(const std::string &saveName) const
{
    std::string filepath = getSaveFilePath(saveName);
    std::ifstream file(filepath);
    return file.good(); // Returns true if file can be opened
}

// Write player stats to file
void SaveGameManager::savePlayerStats(const Player &player, std::ofstream &file)
{
    file << "Level=" << player.getLevel() << "\n";
    file << "Experience=" << player.getExperience() << "\n";
    file << "ExpToNextLevel=" << player.getExpToNextLevel() << "\n";
}

// Write skill tree data to file
void SaveGameManager::saveSkillTree(const SkillTree &tree, std::ofstream &file)
{
    file << "AvailableSkillPoints=" << tree.getSkillPoints() << "\n";

    // Define all skill types to save
    const SkillType skillTypes[] = {
        SkillType::SPECIAL_ATTACK,
        SkillType::FIRE_PROJECTILE,
        SkillType::ICE_PROJECTILE,
        SkillType::FIRE_DAMAGE_BOOST,
        SkillType::FIRE_CHARGE_BOOST,
        SkillType::FIRE_STACK_BOOST,
        SkillType::ICE_DAMAGE_BOOST,
        SkillType::ICE_CHARGE_BOOST,
        SkillType::ICE_STACK_BOOST};

    // Loop through each skill type and write its data
    for (const auto &type : skillTypes)
    {
        auto node = tree.getSkillNode(type);
        if (node && !node->isEmpty())
        {
            const auto &skill = node->getKey();
            file << "Skill=" << static_cast<int>(type)
                 << "," << skill.currentLevel
                 << "," << (skill.isLocked ? 1 : 0) << "\n";
        }
    }
}

// Read player stats from file
bool SaveGameManager::loadPlayerStats(Player &player, std::ifstream &file)
{
    std::string line;
    int level = 0, experience = 0, expToNextLevel = 0;

    // Read lines until section end
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[')
        {
            break;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "Level")
                level = std::stoi(value);
            else if (key == "Experience")
                experience = std::stoi(value);
            else if (key == "ExpToNextLevel")
                expToNextLevel = std::stoi(value);
        }
    }

    player.setProgression(level, experience, expToNextLevel);

    return true;
}

// Read skill tree data from file
bool SaveGameManager::loadSkillTree(SkillTree &tree, std::ifstream &file)
{
    std::string line;
    int availablePoints = 0;

    tree.reset(); // Reset skill tree before loading

    // Read skill data
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[')
        {
            break;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "AvailableSkillPoints")
            {
                availablePoints = std::stoi(value);
            }
            else if (key == "Skill")
            {
                size_t comma1 = value.find(',');
                size_t comma2 = value.find(',', comma1 + 1);

                if (comma1 != std::string::npos && comma2 != std::string::npos)
                {
                    int typeInt = std::stoi(value.substr(0, comma1));
                    int currentLevel = std::stoi(value.substr(comma1 + 1, comma2 - comma1 - 1));
                    int isLocked = std::stoi(value.substr(comma2 + 1));

                    SkillType type = static_cast<SkillType>(typeInt);
                    auto node = tree.getSkillNode(type);

                    if (node && !node->isEmpty())
                    {
                        node->getKey().currentLevel = currentLevel;
                        node->getKey().isLocked = (isLocked == 1);
                    }
                }
            }
        }
    }

    // Apply available skill points
    tree.addSkillPoints(availablePoints);

    return true;
}

// Generate save file path string
std::string SaveGameManager::getSaveFilePath(const std::string &saveName) const
{
    return savesDirectory_ + "/" + saveName + ".txt";
}

// Create saves directory if it does not exist
void SaveGameManager::ensureSavesDirectoryExists()
{
    struct stat info;
    if (stat(savesDirectory_.c_str(), &info) != 0)
    {
        MKDIR(savesDirectory_.c_str());
        std::cout << "Created saves directory: " << savesDirectory_ << std::endl;
    }
}
