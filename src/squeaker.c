#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SAMPLE_RATE 44100
#define TAU 6.28318530717958647692528676655900576839433879875 /* 2*PI */

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
    /* sine wave fitted into Uint8 */
    stream[i]=round((sin(tone->phase)+1)*127); 
    /* Time step */
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
  audio.format = AUDIO_U8; /* Unsigned 8-bit int */
  audio.channels = 1; /* Mono */
  audio.samples = 1024;
  audio.callback = fill_audio;
  audio.userdata = &tone;
  if ( SDL_OpenAudio(&audio, NULL) < 0 ) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return(-1);
  }
  SDL_PauseAudio(0); // Start playing
  /* Continuously read new floats from STDIN,
   * quit if something other than float is read */
  while(scanf("%lf",&(tone.frequency)));
  /* scanf didn't detect float, quitting */
  SDL_CloseAudio();
  return(0);
}
