#include <algorithm>
#include <cstdint>

#include "prx/libc/include/General.hpp"
#include "Ngs2Internal.hpp"

namespace {

constexpr float DefaultSoundSpeed = 343.0f;

}

#pragma GCC visibility push(default)

extern "C" {

int APS5_VABI sceNgs2GeomResetListenerParam(Ngs2GeomListenerParam* out_listener_param) {
    if (out_listener_param == nullptr) return SCE_NGS2_ERROR_INVALID_OUT_ADDRESS;
    *out_listener_param = {};
    out_listener_param->orient_front.z = 1.0f;
    out_listener_param->orient_up.y = 1.0f;
    out_listener_param->sound_speed = DefaultSoundSpeed;
    return SCE_NGS2_OK;
}

int APS5_VABI sceNgs2GeomResetSourceParam(Ngs2GeomSourceParam* out_source_param) {
    if (out_source_param == nullptr) return SCE_NGS2_ERROR_INVALID_OUT_ADDRESS;
    *out_source_param = {};
    out_source_param->direction.z = 1.0f;
    out_source_param->cone.inner_level = 1.0f;
    out_source_param->cone.inner_angle = 360.0f;
    out_source_param->cone.outer_level = 1.0f;
    out_source_param->cone.outer_angle = 360.0f;
    out_source_param->rolloff.max_distance = 1000000.0f;
    out_source_param->rolloff.rolloff_factor = 1.0f;
    out_source_param->rolloff.reference_distance = 1.0f;
    out_source_param->doppler_factor = 1.0f;
    out_source_param->fbw_level = 1.0f;
    out_source_param->lfe_level = 1.0f;
    out_source_param->max_level = 1.0f;
    out_source_param->num_speakers = 2;
    out_source_param->matrix_format = 2;
    return SCE_NGS2_OK;
}

int APS5_VABI sceNgs2GeomCalcListener(const Ngs2GeomListenerParam* param, Ngs2GeomListenerWork* out_work, std::uint32_t flags) {
    if (param == nullptr) APS5_INVALID_ARG_EX;
    if (out_work == nullptr) return SCE_NGS2_ERROR_INVALID_OUT_ADDRESS;
    *out_work = {};
    for (std::uint32_t index = 0; index < 4; index++) out_work->matrix[index][index] = 1.0f;
    out_work->velocity = param->velocity;
    out_work->sound_speed = param->sound_speed > 0.0f ? param->sound_speed : DefaultSoundSpeed;
    out_work->coordinate = flags & 1u;
    return SCE_NGS2_OK;
}

int APS5_VABI sceNgs2GeomApply(const Ngs2GeomListenerWork* listener, const Ngs2GeomSourceParam* source, Ngs2GeomAttribute* out_attrib, std::uint32_t flags) {
    (void)flags;
    if (listener == nullptr || source == nullptr) APS5_INVALID_ARG_EX;
    if (out_attrib == nullptr) return SCE_NGS2_ERROR_INVALID_OUT_ADDRESS;
    const std::uint32_t channels = std::min(source->matrix_format == 0 ? 2u : source->matrix_format, NGS2_MAX_CHANNELS);
    const float level = source->max_level > 0.0f ? source->max_level : 1.0f;
    *out_attrib = {};
    out_attrib->pitch_ratio = 1.0f;
    out_attrib->a3d_attrib.position = source->position;
    out_attrib->a3d_attrib.volume = std::max(source->min_level, source->max_level);
    for (std::uint32_t channel = 0; channel < channels; channel++) {
        out_attrib->level[channel * NGS2_MAX_CHANNELS + channel] = level;
    }
    return SCE_NGS2_OK;
}

}

#pragma GCC visibility pop
