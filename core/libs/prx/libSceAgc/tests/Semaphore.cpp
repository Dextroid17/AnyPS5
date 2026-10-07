#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libc/include/Shutdown.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

extern "C" int APS5_VABI sceAgcSetAmmSemaphoreMemory(void* memory, std::uint64_t size_in_bytes);
extern "C" int APS5_VABI sceAgcGetSemaphoreLabel(std::uint32_t index, void** label);

namespace {

constexpr std::uint64_t LabelBytes = 32u;
constexpr std::uint64_t MemoryAlignment = 0x4000u;
constexpr std::uint64_t MemoryBytes = 0x8000u;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename TAction>
void expectFailure(TAction action) {
    try {
        action();
    } catch (const std::runtime_error& error) {
        check(error.what()[0] != '\0', "empty exception message");
        return;
    }
    throw std::runtime_error("expected an exception");
}

alignas(0x1000) std::array<std::uint8_t, MemoryBytes + MemoryAlignment> storage{};

std::uint8_t* memory() {
    const auto base = reinterpret_cast<std::uintptr_t>(storage.data());
    return reinterpret_cast<std::uint8_t*>((base + MemoryAlignment - 1u) & ~static_cast<std::uintptr_t>(MemoryAlignment - 1u));
}

std::uint64_t labelCount() { return MemoryBytes / LabelBytes; }

void testUnregistered() {
    void* label = reinterpret_cast<void*>(0x1);
    expectFailure([&] { sceAgcGetSemaphoreLabel(0, &label); });
    check(label == reinterpret_cast<void*>(0x1), "a rejected semaphore label changed the output");
}

void testRegistrationRejections() {
    expectFailure([] { sceAgcSetAmmSemaphoreMemory(nullptr, MemoryBytes); });
    expectFailure([] { sceAgcSetAmmSemaphoreMemory(memory(), 0); });
    expectFailure([] { sceAgcSetAmmSemaphoreMemory(memory() + 1, MemoryBytes); });
    expectFailure([] { sceAgcSetAmmSemaphoreMemory(memory(), MemoryBytes - 1); });
}

void testRegistration() {
    for (std::uint64_t byte = 0; byte < MemoryBytes; ++byte) memory()[byte] = 0x5a;
    check(sceAgcSetAmmSemaphoreMemory(memory(), MemoryBytes) == 0, "registering the AMM semaphore memory failed");
    for (std::uint64_t byte = 0; byte < MemoryBytes; ++byte) check(memory()[byte] == 0, "registering the AMM semaphore memory did not clear it");
}

void testLabels() {
    void* label = nullptr;
    check(sceAgcGetSemaphoreLabel(0, &label) == 0 && label == memory(), "incorrect first semaphore label");
    check(sceAgcGetSemaphoreLabel(1, &label) == 0 && label == memory() + LabelBytes, "incorrect second semaphore label");
    check(sceAgcGetSemaphoreLabel(static_cast<std::uint32_t>(labelCount() - 1), &label) == 0 && label == memory() + MemoryBytes - LabelBytes, "incorrect last semaphore label");
}

void testLabelRejections() {
    void* label = reinterpret_cast<void*>(0x1);
    expectFailure([&] { sceAgcGetSemaphoreLabel(static_cast<std::uint32_t>(labelCount()), &label); });
    expectFailure([&] { sceAgcGetSemaphoreLabel(0xffffffffu, &label); });
    expectFailure([&] { sceAgcGetSemaphoreLabel(0, nullptr); });
    check(label == reinterpret_cast<void*>(0x1), "a rejected semaphore label changed the output");
}

void testRepeatedRegistration() {
    expectFailure([] { sceAgcSetAmmSemaphoreMemory(memory(), MemoryBytes); });
}

}

int main() {
    try {
        testUnregistered();
        testRegistrationRejections();
        testRegistration();
        testLabels();
        testLabelRejections();
        testRepeatedRegistration();
        LibcRunShutdown_nid_postfix();
        std::puts("AGC semaphore tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        try { LibcRunShutdown_nid_postfix(); }
        catch (const std::exception& shutdown) { std::fprintf(stderr, "shutdown: %s\n", shutdown.what()); }
        return 1;
    }
}
