#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C" {
int APS5_VABI sceShareCaptureScreenshotExtended(const void* extended_param, std::int32_t* req_id);
int APS5_VABI sceShareCaptureVideoClipExtended();
int APS5_VABI sceShareGetRunningStatus();
int APS5_VABI sceShareSetContentParamForApplicationTitle();
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

template <typename TCall>
bool NotImplemented(TCall call) {
    try {
        call();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

}

int main() {
    constexpr std::int32_t notSupported = static_cast<std::int32_t>(0x81960007);
    std::uint8_t param[64]{};

    std::int32_t reqId = 7;
    Require(sceShareCaptureScreenshotExtended(param, &reqId) == notSupported);
    Require(reqId == -1);

    reqId = 7;
    Require(sceShareCaptureScreenshotExtended(nullptr, &reqId) == notSupported);
    Require(reqId == -1);

    Require(sceShareCaptureScreenshotExtended(param, nullptr) == notSupported);
    Require(sceShareCaptureScreenshotExtended(nullptr, nullptr) == notSupported);

    Require(NotImplemented([] { sceShareCaptureVideoClipExtended(); }));
    Require(NotImplemented([] { sceShareGetRunningStatus(); }));
    Require(NotImplemented([] { sceShareSetContentParamForApplicationTitle(); }));
    return 0;
}