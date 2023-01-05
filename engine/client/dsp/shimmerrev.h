#include "common.h"
#include "client.h"
#include "sound.h"

void shimmerrev_init(void);
void shimmerrev_process(portable_samplepair_t *interleaved, int samplecount, int samplerate);
void shimmerrev_setpreset(int preset);