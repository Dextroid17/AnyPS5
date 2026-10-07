#include "prx/libSceAgcDriver/Submit/include/UserData.hpp"

#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

extern "C" {

std::uint32_t APS5_VABI sceAgcDriverUserDataGetPacketSize(std::uint32_t size_in_bytes) {
    if (size_in_bytes == 0) return 3;
    const auto payloadDwords = static_cast<std::uint32_t>((static_cast<std::uint64_t>(size_in_bytes) + 3u) >> 2);
    if (payloadDwords == 1) return 4;
    return payloadDwords + 7;
}

}
