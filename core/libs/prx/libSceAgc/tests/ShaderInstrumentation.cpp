#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libc/include/Shutdown.hpp"

#include <cstdint>
#include <cstdio>
#include <stdexcept>

extern "C" int APS5_VABI sceAgcSetShaderInstrumentation(std::uint32_t flags);
extern "C" std::uint32_t APS5_VABI sceAgcGetShaderInstrumentation();

namespace {

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void testRoundTrip() {
    check(sceAgcGetShaderInstrumentation() == 0, "the instrumentation flags do not start clear");
    check(sceAgcSetShaderInstrumentation(0x5a5a5a5au) == 0, "setting the instrumentation flags failed");
    check(sceAgcGetShaderInstrumentation() == 0x5a5a5a5au, "the instrumentation flags did not round-trip");
    check(sceAgcSetShaderInstrumentation(0xffffffffu) == 0 && sceAgcGetShaderInstrumentation() == 0xffffffffu, "the instrumentation flags dropped a bit");
    check(sceAgcSetShaderInstrumentation(0) == 0, "clearing the instrumentation flags failed");
    check(sceAgcGetShaderInstrumentation() == 0, "the instrumentation flags did not clear");
}

}

int main() {
    try {
        testRoundTrip();
        LibcRunShutdown_nid_postfix();
        std::puts("AGC shader instrumentation tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        try { LibcRunShutdown_nid_postfix(); }
        catch (const std::exception& shutdown) { std::fprintf(stderr, "shutdown: %s\n", shutdown.what()); }
        return 1;
    }
}
