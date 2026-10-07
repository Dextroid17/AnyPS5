#include "prx/libSceAgc/Misc/include/Semaphore.hpp"
#include "prx/libSceAgc/Command/include/Packet.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

void* AmmSemaphoreMemory = nullptr;
std::uint64_t AmmSemaphoreBytes = 0;

}

extern "C" {

int APS5_VABI sceAgcSetAmmSemaphoreMemory(void* memory, std::uint64_t size_in_bytes) {
    Agc::Command::Require(AmmSemaphoreMemory == nullptr, __func__, "the AMM semaphore memory is already registered");
    Agc::Command::Require(memory != nullptr, __func__, "missing AMM semaphore memory");
    Agc::Command::Require(size_in_bytes != 0, __func__, "empty AMM semaphore memory");
    Agc::Command::Require((reinterpret_cast<std::uintptr_t>(memory) & 0x3fffu) == 0, __func__, "misaligned AMM semaphore memory");
    Agc::Command::Require((size_in_bytes & 0x3fffu) == 0, __func__, "misaligned AMM semaphore memory size");
    std::memset(memory, 0, static_cast<std::size_t>(size_in_bytes));
    AmmSemaphoreMemory = memory;
    AmmSemaphoreBytes = size_in_bytes;
    return 0;
}

int APS5_VABI sceAgcGetSemaphoreLabel(std::uint32_t index, void** label) {
    Agc::Command::Require(AmmSemaphoreMemory != nullptr, __func__, "the AMM semaphore memory is not registered");
    Agc::Command::Require(label != nullptr, __func__, "missing semaphore label destination");
    Agc::Command::Require((static_cast<std::uint64_t>(index) + 1u) * 32u <= AmmSemaphoreBytes, __func__, "the semaphore label lies outside the registered memory");
    *label = static_cast<std::uint8_t*>(AmmSemaphoreMemory) + static_cast<std::uint64_t>(index) * 32u;
    return 0;
}

}
