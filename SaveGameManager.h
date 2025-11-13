#pragma once

#include <string>

// Forward declarations for Player and SkillTree classes
class Player;
class SkillTree;

// Class responsible for saving and loading player game data
class SaveGameManager
{
public:
    // Constructor initializes save directory
    SaveGameManager();

    // Saves the player's game data to file
    bool saveGame(const Player &player, const std::string &saveName);

    // Loads the player's game data from file
    bool loadGame(Player &player, const std::string &saveName);

    // Checks if a save file with the given name exists
    bool saveExists(const std::string &saveName) const;

private:
    // Directory path for save files
    std::string savesDirectory_;

    // Writes player stats to file
    void savePlayerStats(const Player &player, std::ofstream &file);

    // Writes player's skill tree data
    void saveSkillTree(const SkillTree &tree, std::ofstream &file);

    // Reads player stats from file
    bool loadPlayerStats(Player &player, std::ifstream &file);

    // Reads player's skill tree data from file
    bool loadSkillTree(SkillTree &tree, std::ifstream &file);

    // Returns full save file path
    std::string getSaveFilePath(const std::string &saveName) const;

    // Ensures the saves directory is created
    void ensureSavesDirectoryExists();
};
