#pragma once
#include <string>

extern "C" {
#include "../libs/sha256/sha256.h"
}

std::string sha256(const std::string& str);
