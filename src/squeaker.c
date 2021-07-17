#include <stdio.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

#define TAU 6.28318530717958647692528676655900576839433879875 /* 2*PI */
#define SAMPLE_RATE 44100
#define PORTAMENTO 0.1

typedef struct
{
  double freq;
  double new_freq;
  double phase;
  double portamento;
  SDL_mutex *mtx;
} tone;

void fill_audio(void *udata, Uint8 *stream, int len)
{
  tone *tone = udata;
  for(int i=0; i<len; i+=4)
  {
    /* Time step */
    SDL_LockMutex(tone->mtx);
    {
      /* sine wave */
      float sample = sin(tone->phase);
      /* written byte by byte */
      Uint8* sample_byte = (Uint8*) &sample;
      for(int j=0; j<4; j++) stream[i+j] = sample_byte[j];
      /* Update frequency */
      if(tone->portamento)
      {
        /* Slide from current frequency to new frequency.
         * This implements simple P controller, with asymptotic behaviour.
         */
        tone->freq+=(tone->new_freq-tone->freq)/(SAMPLE_RATE*tone->portamento);
      }
      else
      {
        /* If portamento is 0, jump straight to new frequency */
        tone->freq = tone->new_freq;
      }
      /* Update phase */
      tone->phase += (TAU/SAMPLE_RATE) * tone->freq;
    }
    SDL_UnlockMutex(tone->mtx);
  }
  /* Wrap phase to stop it from growing uncontrollably */
  tone->phase = fmod(tone->phase,TAU);
}

int main(int argc, const char *argv[])
{
  SDL_SetMainReady();
  if(SDL_Init(SDL_INIT_AUDIO) != 0)
  {
    fprintf(stderr,"Unable to initialize SDL: %s", SDL_GetError());
    return(-1);
  }
  SDL_mutex *mtx;
  if(!(mtx = SDL_CreateMutex()))
  {
    fprintf(stderr, "Couldn't initialise mutex\n");
    return(-1);
  }
  tone tone =
  {
    .freq = 0.0,
    .new_freq = 0.0,
    .phase = 0.0,
    .portamento = PORTAMENTO,
    .mtx = mtx
  };
  if(argc==2)
  {
    sscanf(argv[1],"%lf",&(tone.portamento));
  }
  SDL_AudioSpec audio;
  audio.freq = SAMPLE_RATE;
  audio.format = AUDIO_F32SYS; /* 32-bit float */
  audio.channels = 1; /* Mono */
  audio.samples = 1024;
  audio.callback = fill_audio;
  audio.userdata = &tone;
  if(SDL_OpenAudio(&audio, NULL) != 0)
  {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return(-1);
  }
  SDL_PauseAudio(0); // Start playing
  /* Continuously read new floats from STDIN,
   * quit if something other than float is read */
  int continuing = 1;
  double new_freq;
  while(continuing)
  {
    if(!scanf("%lf",&new_freq)){
      continuing = 0;
    };
    if(tone.new_freq != new_freq)
    {
      SDL_LockMutex(mtx);
      {
        tone.new_freq = new_freq;
      }
      SDL_UnlockMutex(mtx);
    }
  }
  /* scanf didn't detect float, quitting */
  SDL_CloseAudio();
  SDL_DestroyMutex(mtx);
  SDL_Quit();
  return(0);
}
