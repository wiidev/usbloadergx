#pragma once
#include <string>
#include <vector>
#include <map>
#include "xml/pugixml.hpp"

// --- Data structures ---
struct RiivoChoice {
    std::string name;
    std::string patchId;
};
struct RiivoOption {
    std::string name;
    std::vector<RiivoChoice> choices;
    int selected = 0; // 0: Disabled
};
struct Rii {
    std::string id;
    pugi::xml_node node;
};
struct RiivolutionModXml {
    std::string filename;
    std::string sectionTitle;
    std::vector<RiivoOption> options;
    std::string regionalId; // e.g. RZTP
    std::vector<Rii> patches; // parsed <patch>
};

class RiivolutionMods {
public:
    // Scanning, parsing, config
    static std::vector<RiivolutionModXml> ScanForXmls(const std::string& shortId);
    static bool ParseXml(const std::string& path, RiivolutionModXml& outMod, std::string& outRegionId);
    static bool LoadConfig(const std::string& regionalId, RiivolutionModXml& mod);
    static bool SaveConfig(const std::string& regionalId, const RiivolutionModXml& mod);

    // Patching
    static bool FileReplaceAndPatch(const RiivolutionModXml& mod, const std::string& gameIsoPath);
    static void ApplyMemoryPatches(const RiivolutionModXml& mod);

    // Utility
    static std::string GetConfigPath(const std::string& regionalId);
    static std::string GetIsoPathForGame(const char* gameId); // implement with WBFS/ISO search logic
};