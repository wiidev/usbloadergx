#include "RiivolutionPatcher.hpp"
#include "xml/pugixml.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

// Helper: Loads a file to memory and returns its contents and size
static bool LoadFileToMem(const std::string& filepath, std::vector<char>& outBuffer)
{
    // 1. Get file size using C-style sys/stat.h
    struct stat fileStat;
    if (stat(filepath.c_str(), &fileStat) != 0) {
        // stat failed (e.g., file doesn't exist, no permission)
        return false;
    }
    size_t size = fileStat.st_size;

    // 2. Open file in binary mode
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        // Failed to open file
        return false;
    }

    // 3. Resize buffer to fit the file
    outBuffer.resize(size);

    // 4. Read the entire file into the buffer
    file.read(outBuffer.data(), size);

    // 5. Check if the read operation was successful and read the full size
    //    file.gcount() returns the number of characters actually read.
    if (file.fail() || static_cast<size_t>(file.gcount()) != size) {
        // Read failed or didn't read the expected number of bytes
        outBuffer.clear(); // Clear buffer on failure
        return false;
    }

    // 6. Close the file (optional explicitly, destructor will handle it)
    file.close();

    return true; // Success
}

// Helper: Find XML files in a directory
static void FindXMLFiles(const std::string& dir, std::vector<std::string>& xmlFiles) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;
    struct dirent* entry;
    while ((entry = readdir(d))) {
        std::string name(entry->d_name);
        if (name.size() > 4 && name.substr(name.size() - 4) == ".xml")
            xmlFiles.push_back(dir + "/" + name);
    }
    closedir(d);
}

// Parse a single Riivolution XML file using pugixml and add it to the discs vector
static bool ParseRiivolutionXML(const std::string& xmlPath, std::vector<RiiDisc>& discs, const std::string& rootfs, int fs) {
    std::vector<char> buffer;
    if (!LoadFileToMem(xmlPath, buffer)) return false;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());
    if (!result) return false;
    pugi::xml_node wiidisc = doc.child("wiidisc");
    if (!wiidisc) return false;

    RiiDisc disc;
    // root and shiftfiles parsing
    std::string xmlroot = xmlPath.substr(0, xmlPath.find_last_of("/"));
    bool shiftfiles = false;
    if (wiidisc.attribute("root")) xmlroot = wiidisc.attribute("root").value();
    if (wiidisc.attribute("shiftfiles")) shiftfiles = std::string(wiidisc.attribute("shiftfiles").value()) == "true";
    if (shiftfiles) RVL_SetAlwaysShift(true);

    // id check
    pugi::xml_node idNode = wiidisc.child("id");
    if (idNode) {
        if (idNode.attribute("game")) {
            std::string gameId = idNode.attribute("game").value();
            if (gameId != rootfs.substr(rootfs.size() - 6)) return false;
        }
    }

    // Parse options/sections/options/choices/patches
    pugi::xml_node optionsNode = wiidisc.child("options");
    for (pugi::xml_node sectionNode : optionsNode.children("section")) {
        RiiSection section;
        section.Name = sectionNode.attribute("name") ? sectionNode.attribute("name").value() : "";
        section.ID = sectionNode.attribute("id") ? sectionNode.attribute("id").value() : section.Name;
        for (pugi::xml_node optionNode : sectionNode.children("option")) {
            RiiOption option;
            option.Name = optionNode.attribute("name") ? optionNode.attribute("name").value() : "";
            option.ID = optionNode.attribute("id") ? optionNode.attribute("id").value() : section.ID + option.Name;
            option.Default = optionNode.attribute("default") ? std::stoi(optionNode.attribute("default").value()) : 0;
            for (pugi::xml_node choiceNode : optionNode.children("choice")) {
                RiiChoice choice;
                choice.Name = choiceNode.attribute("name") ? choiceNode.attribute("name").value() : "";
                choice.ID = choiceNode.attribute("id") ? choiceNode.attribute("id").value() : xmlroot + option.ID + choice.Name;
                choice.Filesystem = fs;
                for (pugi::xml_node patchNode : choiceNode.children("patch")) {
                    RiiChoice::Patch patch;
                    patch.ID = patchNode.attribute("id") ? patchNode.attribute("id").value() : "";
                    for (pugi::xml_node paramNode : patchNode.children("param")) {
                        patch.Params[paramNode.attribute("name").value()] = paramNode.attribute("value").value();
                    }
                    choice.Patches.push_back(patch);
                }
                for (pugi::xml_node paramNode : choiceNode.children("param")) {
                    choice.Params[paramNode.attribute("name").value()] = paramNode.attribute("value").value();
                }
                option.Choices.push_back(choice);
            }
            for (pugi::xml_node paramNode : optionNode.children("param")) {
                option.Params[paramNode.attribute("name").value()] = paramNode.attribute("value").value();
            }
            section.Options.push_back(option);
        }
        disc.Sections.push_back(section);
    }

    // Parse patches
    for (pugi::xml_node patchNode : wiidisc.children("patch")) {
        RiiPatch patch;
        std::string id = patchNode.attribute("id") ? patchNode.attribute("id").value() : "";
        for (pugi::xml_node fileNode : patchNode.children("file")) {
            RiiFilePatch file;
            file.Resize = fileNode.attribute("resize") ? std::string(fileNode.attribute("resize").value()) == "true" : true;
            file.Create = fileNode.attribute("create") ? std::string(fileNode.attribute("create").value()) == "true" : false;
            file.Disc = fileNode.attribute("disc") ? fileNode.attribute("disc").value() : "";
            file.Offset = fileNode.attribute("offset") ? std::stoi(fileNode.attribute("offset").value()) : 0;
            file.External = fileNode.attribute("external") ? fileNode.attribute("external").value() : "";
            file.FileOffset = fileNode.attribute("fileoffset") ? std::stoi(fileNode.attribute("fileoffset").value()) : 0;
            file.Length = fileNode.attribute("length") ? std::stoi(fileNode.attribute("length").value()) : 0;
            patch.Files.push_back(file);
        }
        for (pugi::xml_node folderNode : patchNode.children("folder")) {
            RiiFolderPatch folder;
            folder.Create = folderNode.attribute("create") ? std::string(folderNode.attribute("create").value()) == "true" : false;
            folder.Resize = folderNode.attribute("resize") ? std::string(folderNode.attribute("resize").value()) == "true" : true;
            folder.Recursive = folderNode.attribute("recursive") ? std::string(folderNode.attribute("recursive").value()) == "true" : true;
            folder.Length = folderNode.attribute("length") ? std::stoi(folderNode.attribute("length").value()) : 0;
            folder.Disc = folderNode.attribute("disc") ? folderNode.attribute("disc").value() : "";
            folder.External = folderNode.attribute("external") ? folderNode.attribute("external").value() : "";
            patch.Folders.push_back(folder);
        }
        for (pugi::xml_node shiftNode : patchNode.children("shift")) {
            RiiShiftPatch shift;
            shift.Source = shiftNode.attribute("source") ? shiftNode.attribute("source").value() : "";
            shift.Destination = shiftNode.attribute("destination") ? shiftNode.attribute("destination").value() : "";
            patch.Shifts.push_back(shift);
        }
        for (pugi::xml_node savegameNode : patchNode.children("savegame")) {
            RiiSavegamePatch save;
            save.External = savegameNode.attribute("external") ? savegameNode.attribute("external").value() : "";
            save.Clone = savegameNode.attribute("clone") ? std::string(savegameNode.attribute("clone").value()) == "true" : 1;
            patch.Savegame = save;
        }
        for (pugi::xml_node dlcNode : patchNode.children("dlc")) {
            RiiDLCPatch dlc;
            dlc.External = dlcNode.attribute("external") ? dlcNode.attribute("external").value() : "";
            patch.DLC = dlc;
        }
        for (pugi::xml_node memoryNode : patchNode.children("memory")) {
            RiiMemoryPatch memory;
            memory.Offset = memoryNode.attribute("offset") ? std::stoi(memoryNode.attribute("offset").value()) : 0;
            memory.Search = memoryNode.attribute("search") ? std::string(memoryNode.attribute("search").value()) == "true" : false;
            memory.Ocarina = memoryNode.attribute("ocarina") ? std::string(memoryNode.attribute("ocarina").value()) == "true" : false;
            memory.Align = memoryNode.attribute("align") ? std::stoi(memoryNode.attribute("align").value()) : 1;
            memory.ValueFile = memoryNode.attribute("valuefile") ? memoryNode.attribute("valuefile").value() : "";
            if (memoryNode.attribute("value")) {
                std::string valueHex = memoryNode.attribute("value").value();
                // Convert hex string to bytes
                if (valueHex.substr(0, 2) == "0x") valueHex = valueHex.substr(2);
                size_t len = valueHex.length() / 2;
                memory.Value = new u8[len];
                memory.Length = len;
                for (size_t i = 0; i < len; ++i)
                    memory.Value[i] = std::stoi(valueHex.substr(2 * i, 2), nullptr, 16);
            }
            if (memoryNode.attribute("original")) {
                std::string origHex = memoryNode.attribute("original").value();
                if (origHex.substr(0, 2) == "0x") origHex = origHex.substr(2);
                size_t len = origHex.length() / 2;
                memory.Original = new u8[len];
                if (!memory.Length)
                    memory.Length = len;
                for (size_t i = 0; i < len; ++i)
                    memory.Original[i] = std::stoi(origHex.substr(2 * i, 2), nullptr, 16);
            }
            patch.Memory.push_back(memory);
        }
        disc.Patches[id] = patch;
    }

    discs.push_back(disc);
    return true;
}

// Main entry: parse all XMLs for this game and create a combined disc with config applied
bool Riivolution_ParseForGame(const std::string& gameId, RiiDisc& outDisc) {
    std::vector<RiiDisc> discs;
    std::vector<std::string> xmlFiles;
    // Scan both standard Riivolution directories
    FindXMLFiles("/sd/riivolution", xmlFiles);
    FindXMLFiles("/sd/apps/riivolution", xmlFiles);
    for (const std::string& xmlPath : xmlFiles) {
        ParseRiivolutionXML(xmlPath, discs, "/sd/", 0);
    }
    if (discs.empty()) return false;
    outDisc = CombineDiscs(&discs);
    ParseConfigXMLs(&outDisc);
    return true;
}