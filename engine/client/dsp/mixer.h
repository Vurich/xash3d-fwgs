#include "common.h"
#include "client.h"
#include "sound.h"

#define DSP_MIXER_CHANNELS 3

void mixer_init(void);
void mixer_process(portable_samplepair_t *interleaved_buffers[DSP_MIXER_CHANNELS], portable_samplepair_t *interleaved_output, int samplecount, int samplerate);
void mixer_setpreset(int preset);