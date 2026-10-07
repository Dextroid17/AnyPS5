#include "prx/libSceAgc/Misc/include/ShaderInstrumentation.hpp"

#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

std::uint32_t ShaderInstrumentationFlags = 0;

}

extern "C" {

int APS5_VABI sceAgcSetShaderInstrumentation(std::uint32_t flags) {
    ShaderInstrumentationFlags = flags;
    return 0;
}

std::uint32_t APS5_VABI sceAgcGetShaderInstrumentation() {
    return ShaderInstrumentationFlags;
}

}
