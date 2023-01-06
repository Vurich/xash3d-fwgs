#include "RNBO.h"
#include "common.h"
#include "client.h"
#include "sound.h"
#include "clamp_cast.hpp"

#define DSP_MIXER_CHANNELS 3

template <typename T>
constexpr double normalize(T value)
{
    return value < 0
               ? -static_cast<double>(value) / std::numeric_limits<T>::min()
               : static_cast<double>(value) / std::numeric_limits<T>::max();
}

template <typename T>
constexpr T denormalize(double value)
{
    return clamp_cast<T>(value < 0
                             ? -value * std::numeric_limits<T>::min()
                             : value * std::numeric_limits<T>::max());
}

std::string mixer_presets_json =
#include "mixer_presets.json.h"
    ;
RNBO::CoreObject mixer_obj;
RNBO::PresetList mixer_presets("[]");
std::vector<double> mixer_inputs;
std::vector<double> mixer_outputs;

extern "C"
{
    RNBO::PatcherFactoryFunctionPtr MixerFactoryFunction(RNBO::PlatformInterface *platformInterface);

    void mixer_setpreset(int preset_index)
    {
        return;
        auto preset = mixer_presets.presetAtIndex(preset_index);

        if (preset)
        {
            mixer_obj.setPreset(std::move(preset));
        }
    }

    void mixer_init()
    {
        auto mixerPatcher = MixerFactoryFunction(RNBO::Platform::get())();

        mixer_obj.setPatcher((RNBO::UniquePtr<RNBO::PatcherInterface>(mixerPatcher)));

        mixer_presets = RNBO::PresetList(mixer_presets_json);

        // mixer_setpreset(0);
    }

    void mixer_process(portable_samplepair_t *interleaved_buffers[DSP_MIXER_CHANNELS], portable_samplepair_t *interleaved_output, int samplecount, int samplerate)
    {
        mixer_obj.prepareToProcess(samplerate, samplecount);
        mixer_inputs.resize(samplecount * 2 * DSP_MIXER_CHANNELS);
        mixer_outputs.resize(samplecount * 2);

        double *inputs[2 * DSP_MIXER_CHANNELS];
        double *outputs[2] = {mixer_outputs.data(), mixer_outputs.data() + samplecount};

        for (int buf = 0; buf < DSP_MIXER_CHANNELS; buf++)
        {
            for (int i = 0; i < samplecount; i++)
            {
                mixer_inputs[i + samplecount * buf * 2] = normalize(interleaved_buffers[buf][i].left);
                mixer_inputs[i + samplecount * (buf * 2 + 1)] = normalize(interleaved_buffers[buf][i].right);
            }

            inputs[buf * 2] = mixer_inputs.data() + samplecount * buf * 2;
            inputs[buf * 2 + 1] = mixer_inputs.data() + samplecount * (buf * 2 + 1);
        }

        mixer_obj.process(inputs, 2 * DSP_MIXER_CHANNELS, outputs, 2, samplecount);

        for (int i = 0; i < samplecount; i++)
        {
            interleaved_output[i].left = denormalize<int>(mixer_outputs[i]);
            interleaved_output[i].right = denormalize<int>(mixer_outputs[i + samplecount]);
        }
    }
}