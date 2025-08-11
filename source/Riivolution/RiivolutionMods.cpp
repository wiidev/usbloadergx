#include "RiivolutionMods.hpp"
#include "FileOperations/fileops.h"
#include "usbloader/disc.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include "xml/pugixml.hpp"

// --- Directory scan helper ---
static void scanXmlsIn(const std::string& dir, std::vector<std::string>& out) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;
    struct dirent* de;
    while ((de = readdir(d))) {
        std::string fn = de->d_name;
        if (fn.size() > 4 && fn.substr(fn.size() - 4) == ".xml")
            out.push_back(dir + fn);
    }
    closedir(d);
}

// --- XML scan logic for all game ID variants ---
std::vector<RiivolutionModXml> RiivolutionMods::ScanForXmls(const std::string& discHdr) {
    std::string regionalID = discHdr.substr(0, 4); // e.g. RZTE
    std::string shortID = discHdr.substr(0, 3);  // RZT

    std::vector<std::string> paths;

    // Scan sd:/riivolution/(regional)/, sd:/riivolution/(full)/, sd:/mods/(regional)/, sd:/mods/(full)/, fallback: sd:/riivolution/
    std::vector<std::string> baseDirs = { "sd:/riivolution/", "sd:/mods/" };
    std::vector<std::string> ids = { regionalID, discHdr, shortID };

    for (const auto& base : baseDirs) {
        for (const auto& id : ids) {
            scanXmlsIn(base + id + "/", paths);
        }
    }
    scanXmlsIn("sd:/riivolution/", paths);  // Remains as a separate call

    std::vector<RiivolutionModXml> mods;
    for (auto& path : paths) {
        RiivolutionModXml m;
        std::string regid;
        if (ParseXml(path, m, regid)) {
            m.filename = path;
            m.regionalId = regid;
            mods.push_back(m);
        }
    }
    return mods;
}

// --- Parse a single Riivolution XML (pugixml) ---
bool RiivolutionMods::ParseXml(const std::string& path, RiivolutionModXml& outMod, std::string& outRegionId) {
    outMod.filename = path;
    outMod.options.clear();
    outMod.sectionTitle.clear();
    outRegionId.clear();

    pugi::xml_document doc;
    if (!doc.load_file(path.c_str())) return false;

    auto root = doc.child("wiidisc");
    if (!root) return false;

    auto idNode = root.child("id");
    auto regionNode = root.child("region");
    if (!idNode || !regionNode) return false;

    std::string game = idNode.attribute("game").as_string();
    std::string region = regionNode.attribute("type").as_string();
    outRegionId = game + region;
    outMod.regionalId = outRegionId;
    outMod.patches.clear();

    for (auto patch : root.children("patch")) {
        Rii p;
        p.id = patch.attribute("id").as_string();
        p.node = patch;
        outMod.patches.push_back(p);
    }

    auto section = root.child("options").child("section");
    if (!section) return false;
    outMod.sectionTitle = section.attribute("name").as_string();

    for (auto option : section.children("option")) {
        RiivoOption opt;
        opt.name = option.attribute("name").as_string();
        opt.choices.push_back({"Disabled", ""});
        for (auto choice : option.children("choice")) {
            auto patch = choice.child("patch");
            std::string patchId = patch ? patch.attribute("id").as_string() : "";
            opt.choices.push_back({choice.attribute("name").as_string(), patchId});
        }
        opt.selected = 0;
        outMod.options.push_back(opt);
    }
    return true;
}

// --- Config path logic ---
std::string RiivolutionMods::GetConfigPath(const std::string& regionalId) {
    std::string dir = "sd:/riivolution/" + regionalId;
    mkdir(dir.c_str(), 0777);
    return dir + "/config";
}

bool RiivolutionMods::LoadConfig(const std::string& regionalId, RiivolutionModXml& mod) {
    pugi::xml_document doc;
    if (!doc.load_file(GetConfigPath(regionalId).c_str())) return false;
    auto root = doc.child("riivolution");
    if (!root) return false;
    for (auto optNode : root.children("option")) {
        std::string id = optNode.attribute("id").as_string();
        int def = optNode.attribute("default").as_int(0);
        for (auto& opt : mod.options)
            if (opt.name == id) opt.selected = def;
    }
    return true;
}

bool RiivolutionMods::SaveConfig(const std::string& regionalId, const RiivolutionModXml& mod) {
    pugi::xml_document doc;
    auto root = doc.append_child("riivolution");
    root.append_attribute("version") = 2;
    for (const auto& opt : mod.options) {
        auto node = root.append_child("option");
        node.append_attribute("id") = opt.name.c_str();
        node.append_attribute("default") = opt.selected;
    }
    return doc.save_file(GetConfigPath(regionalId).c_str());
}

// --- File patching stub (to be implemented with WBFS/ISO logic) ---
bool RiivolutionMods::FileReplaceAndPatch(const RiivolutionModXml& mod, const std::string& gameIsoPath) {
    std::vector<std::string> enabledPatchIds;
    for (const RiivoOption& opt : mod.options) {
        if (opt.selected > 0 && (size_t)opt.selected < opt.choices.size())
            enabledPatchIds.push_back(opt.choices[opt.selected].patchId);
    }
    for (const std::string& patchId : enabledPatchIds) {
        auto p = std::find_if(mod.patches.begin(), mod.patches.end(),
            [&](const Rii& p) { return p.id == patchId; });
        if (p == mod.patches.end()) continue;
        for (auto file : p->node.children("file")) {
            const char* disc = file.attribute("disc").as_string();
            const char* ext = file.attribute("external").as_string();
            // TODO: Use WBFS/ISO patch logic
            printf("Would replace '%s' in ISO '%s' with '%s'\n", disc, gameIsoPath.c_str(), ext);
        }
        for (auto folder : p->node.children("folder")) {
            // TODO: Implement folder patching
        }
    }
    return true;
}

// --- Memory patching stub ---
void RiivolutionMods::ApplyMemoryPatches(const RiivolutionModXml& mod) {
    std::vector<std::string> enabledPatchIds;
    for (const RiivoOption& opt : mod.options) {
        if (opt.selected > 0 && (size_t)opt.selected < opt.choices.size())
            enabledPatchIds.push_back(opt.choices[opt.selected].patchId);
    }
    for (const std::string& patchId : enabledPatchIds) {
        auto p = std::find_if(mod.patches.begin(), mod.patches.end(),
            [&](const Rii& p) { return p.id == patchId; });
        if (p == mod.patches.end()) continue;
        for (auto mem : p->node.children("memory")) {
            std::string value = mem.attribute("value").as_string();
            std::string offsetStr = mem.attribute("offset").as_string();
            unsigned int offset = 0;
            sscanf(offsetStr.c_str(), "%x", &offset);
            if (!value.empty() && offset != 0) {
                size_t len = value.length() / 2;
                std::vector<uint8_t> bytes(len);
                for (size_t i = 0; i < len; ++i)
                    sscanf(value.substr(i * 2, 2).c_str(), "%2hhx", &bytes[i]);
                memcpy((void*)offset, bytes.data(), bytes.size());
            }
        }
    }
}

// --- Utility: WBFS/ISO search stub ---
std::string RiivolutionMods::GetIsoPathForGame(const char* gameId) {
    // TODO: Implement WBFS/ISO search by gameId
    // Return the path to the loaded game image.
    return "";
}