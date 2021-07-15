#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL2/SDL.h>


#define SAMPLE_RATE 44100
#define TAU 6.28318530717958647692528676655900576839433879875
typedef struct
{
  double frequency;
  double phase;
} tone;


void fill_audio(void *udata, Uint8 *stream, int len)
{
  tone *tone = udata;
  for(int i=0; i<len; i++)
  {
    stream[i]=round((sin(tone->phase)+1)*127);
    tone->phase += (TAU/SAMPLE_RATE) * tone->frequency;
  }
  /* Wrap phase to stop it from growing uncontrollably */
  tone->phase = fmod(tone->phase,TAU);
}

int main()
{
  tone tone = {.frequency = 0.0, .phase = 0.0};
  SDL_AudioSpec audio;
  audio.freq = SAMPLE_RATE;
  audio.format = AUDIO_U8;
  audio.channels = 1;
  audio.samples = 1024;
  audio.callback = fill_audio;
  audio.userdata = &tone;
  if ( SDL_OpenAudio(&audio, NULL) < 0 ) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return(-1);
  }
  SDL_PauseAudio(0);
  while(scanf("%lf",&(tone.frequency)));
  SDL_CloseAudio();
  return(0);
}
