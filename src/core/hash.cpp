#include "hash.h"
#include <sstream>
#include <iomanip>

std::string sha256(const std::string& input) {
    uint8_t hash[32];
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
    sha256_final(&ctx, hash);

    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }

    return ss.str();
}