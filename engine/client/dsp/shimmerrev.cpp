#include "RNBO.h"
#include "common.h"
#include "client.h"
#include "sound.h"

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
    return static_cast<T>(std::max(std::min(value < 0
                                                ? -value * std::numeric_limits<T>::min()
                                                : value * std::numeric_limits<T>::max(),
                                            static_cast<double>(std::numeric_limits<T>::max())),
                                   static_cast<double>(std::numeric_limits<T>::min())));
}

extern "C"
{
    std::string shimmerrev_presets_json =
#include "shimmerrev_presets.json.h"
        ;

    RNBO::PatcherFactoryFunctionPtr ShimmerRevFactoryFunction(RNBO::PlatformInterface *platformInterface);

    RNBO::CoreObject shimmerrev_obj;
    RNBO::PresetList shimmerrev_presets(shimmerrev_presets_json);
    std::vector<double> shimmerrev_inputs;
    std::vector<double> shimmerrev_outputs;

    void shimmerrev_setpreset(int preset_index)
    {
        auto preset = shimmerrev_presets.presetAtIndex(preset_index);

        if (preset)
        {
            shimmerrev_obj.setPreset(std::move(preset));
        }
    }

    void shimmerrev_init()
    {
        auto shimmerrevPatcher = ShimmerRevFactoryFunction(RNBO::Platform::get())();

        shimmerrev_obj.setPatcher((RNBO::UniquePtr<RNBO::PatcherInterface>(shimmerrevPatcher)));

        shimmerrev_setpreset(0);
    }

    void shimmerrev_process(portable_samplepair_t *interleaved, int samplecount, int samplerate)
    {
        shimmerrev_obj.prepareToProcess(samplerate, samplecount);
        shimmerrev_inputs.resize(samplecount * 2);
        shimmerrev_outputs.resize(samplecount * 2);

        for (int i = 0; i < samplecount; i++)
        {
            shimmerrev_inputs[i] = normalize(interleaved[i].left);
            shimmerrev_inputs[i + samplecount] = normalize(interleaved[i].right);
        }

        double *inputs[2] = {shimmerrev_inputs.data(), shimmerrev_inputs.data() + samplecount};
        double *outputs[2] = {shimmerrev_outputs.data(), shimmerrev_outputs.data() + samplecount};

        shimmerrev_obj.process(inputs, 2, outputs, 2, samplecount);

        for (int i = 0; i < samplecount; i++)
        {
            interleaved[i].left = denormalize<int>(shimmerrev_outputs[i]);
            interleaved[i].right = denormalize<int>(shimmerrev_outputs[i + samplecount]);
        }
    }
}