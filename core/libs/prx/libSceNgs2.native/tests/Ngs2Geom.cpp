#include "Ngs2Test.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>

static constexpr float SoundSpeed = 343.0f;

template <typename TCall>
static bool InvalidArgument(TCall call) {
    try {
        call();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

static void TestLayouts() {
    Require(sizeof(Ngs2GeomVector) == 12);
    Require(sizeof(Ngs2GeomCone) == 16);
    Require(sizeof(Ngs2GeomRolloff) == 16);
    Require(sizeof(Ngs2GeomListenerParam) == 60);
    Require(sizeof(Ngs2GeomListenerWork) == 96);
    Require(sizeof(Ngs2GeomSourceParam) == 108);
    Require(sizeof(Ngs2GeomA3dAttribute) == 32);
    Require(sizeof(Ngs2GeomAttribute) == 308);
}

static void TestResetListenerParam() {
    Ngs2GeomListenerParam param;
    std::memset(&param, 0x5a, sizeof(param));
    Require(sceNgs2GeomResetListenerParam(&param) == SCE_NGS2_OK);
    Require(param.position.x == 0.0f && param.position.y == 0.0f && param.position.z == 0.0f);
    Require(param.velocity.x == 0.0f && param.velocity.y == 0.0f && param.velocity.z == 0.0f);
    Require(param.orient_front.x == 0.0f && param.orient_front.y == 0.0f && param.orient_front.z == 1.0f);
    Require(param.orient_up.x == 0.0f && param.orient_up.y == 1.0f && param.orient_up.z == 0.0f);
    Require(param.sound_speed == SoundSpeed);
    Require(param.reserved[0] == 0u && param.reserved[1] == 0u);
    Require(sceNgs2GeomResetListenerParam(nullptr) == SCE_NGS2_ERROR_INVALID_OUT_ADDRESS);
}

static void TestResetSourceParam() {
    Ngs2GeomSourceParam param;
    std::memset(&param, 0x5a, sizeof(param));
    Require(sceNgs2GeomResetSourceParam(&param) == SCE_NGS2_OK);
    Require(param.position.x == 0.0f && param.position.y == 0.0f && param.position.z == 0.0f);
    Require(param.velocity.x == 0.0f && param.velocity.y == 0.0f && param.velocity.z == 0.0f);
    Require(param.direction.x == 0.0f && param.direction.y == 0.0f && param.direction.z == 1.0f);
    Require(param.cone.inner_level == 1.0f && param.cone.inner_angle == 360.0f);
    Require(param.cone.outer_level == 1.0f && param.cone.outer_angle == 360.0f);
    Require(param.rolloff.model == 0u);
    Require(param.rolloff.max_distance == 1000000.0f);
    Require(param.rolloff.rolloff_factor == 1.0f);
    Require(param.rolloff.reference_distance == 1.0f);
    Require(param.doppler_factor == 1.0f && param.fbw_level == 1.0f && param.lfe_level == 1.0f);
    Require(param.max_level == 1.0f && param.min_level == 0.0f && param.radius == 0.0f);
    Require(param.num_speakers == 2u && param.matrix_format == 2u);
    Require(param.reserved[0] == 0u && param.reserved[1] == 0u);
    Require(sceNgs2GeomResetSourceParam(nullptr) == SCE_NGS2_ERROR_INVALID_OUT_ADDRESS);
}

static void TestCalcListener() {
    Ngs2GeomListenerParam param{};
    param.velocity = {1.0f, 2.0f, 3.0f};
    param.sound_speed = 1500.0f;

    Ngs2GeomListenerWork work;
    std::memset(&work, 0x5a, sizeof(work));
    Require(sceNgs2GeomCalcListener(&param, &work, 0) == SCE_NGS2_OK);
    Require(work.velocity.x == 1.0f && work.velocity.y == 2.0f && work.velocity.z == 3.0f);
    Require(work.sound_speed == 1500.0f);
    Require(work.coordinate == 0u);
    for (std::uint32_t row = 0; row < 4; row++) {
        for (std::uint32_t column = 0; column < 4; column++) {
            Require(work.matrix[row][column] == (row == column ? 1.0f : 0.0f));
        }
    }
    Require(work.reserved[0] == 0u && work.reserved[2] == 0u);

    param.sound_speed = 0.0f;
    Require(sceNgs2GeomCalcListener(&param, &work, 0) == SCE_NGS2_OK);
    Require(work.sound_speed == SoundSpeed);

    Require(sceNgs2GeomCalcListener(&param, &work, 0xfffffffeu) == SCE_NGS2_OK);
    Require(work.coordinate == 0u);
    Require(sceNgs2GeomCalcListener(&param, &work, 0xffffffffu) == SCE_NGS2_OK);
    Require(work.coordinate == 1u);

    Require(InvalidArgument([&] { sceNgs2GeomCalcListener(nullptr, &work, 0); }));
    Require(sceNgs2GeomCalcListener(&param, nullptr, 0) == SCE_NGS2_ERROR_INVALID_OUT_ADDRESS);
}

static void TestApply() {
    Ngs2GeomListenerWork listener{};
    Ngs2GeomSourceParam source{};
    Require(sceNgs2GeomResetSourceParam(&source) == SCE_NGS2_OK);
    source.position = {4.0f, 5.0f, 6.0f};

    Ngs2GeomAttribute attrib;
    std::memset(&attrib, 0x5a, sizeof(attrib));
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.pitch_ratio == 1.0f);
    Require(attrib.a3d_attrib.position.x == 4.0f && attrib.a3d_attrib.position.y == 5.0f && attrib.a3d_attrib.position.z == 6.0f);
    Require(attrib.a3d_attrib.volume == 1.0f);
    Require(attrib.a3d_attrib.reserved[0] == 0u && attrib.reserved[0] == 0u);
    Require(attrib.level[0] == 1.0f && attrib.level[9] == 1.0f);
    Require(attrib.level[18] == 0.0f && attrib.level[63] == 0.0f);

    source.max_level = 0.5f;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.level[0] == 0.5f && attrib.level[9] == 0.5f);
    source.max_level = 0.0f;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.level[0] == 1.0f);

    source.min_level = 0.25f;
    source.max_level = 0.5f;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.a3d_attrib.volume == 0.5f);

    source.min_level = 0.0f;
    source.max_level = 1.0f;
    source.matrix_format = 8;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    for (std::uint32_t channel = 0; channel < 8; channel++) Require(attrib.level[channel * 8 + channel] == 1.0f);

    source.matrix_format = 99;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.level[63] == 1.0f);
    Require(attrib.a3d_attrib.position.x == 4.0f && attrib.a3d_attrib.volume == 1.0f);
    Require(attrib.reserved[0] == 0u);

    source.matrix_format = 0;
    Require(sceNgs2GeomApply(&listener, &source, &attrib, 0) == SCE_NGS2_OK);
    Require(attrib.level[0] == 1.0f && attrib.level[9] == 1.0f && attrib.level[18] == 0.0f);

    Require(InvalidArgument([&] { sceNgs2GeomApply(nullptr, &source, &attrib, 0); }));
    Require(InvalidArgument([&] { sceNgs2GeomApply(&listener, nullptr, &attrib, 0); }));
    Require(sceNgs2GeomApply(&listener, &source, nullptr, 0) == SCE_NGS2_ERROR_INVALID_OUT_ADDRESS);
}

int main() {
    TestLayouts();
    TestResetListenerParam();
    TestResetSourceParam();
    TestCalcListener();
    TestApply();
    return 0;
}
