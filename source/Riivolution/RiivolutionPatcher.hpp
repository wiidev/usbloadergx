#pragma once

#include "rawksd/riivolution.h"
#include "rawksd/riivolution_config.h"
#include <string>
#include <vector>

// This function parses all Riivolution XMLs for the game, loads user config, and prepares a combined RiiDisc.
bool Riivolution_ParseForGame(const std::string& gameId, RiiDisc& outDisc);