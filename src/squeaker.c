#include <stdio.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

#define TAU 6.28318530717958647692528676655900576839433879875 /* 2*PI */
#define SAMPLE_RATE 44100
#define INPUT_LOWPASS 1e-2
#define MIN_FREQ 1e-4

typedef struct
{
  double freq;
  double new_freq;
  double phase;
  double input_lowpass;
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
      Sint32 sample = lround(sin(tone->phase)*SDL_MAX_SINT32);
      /* written byte by byte */
      for(int j=0; j<4; j++) stream[i+j] = ((Uint8*) &sample)[j];
      /* Update frequency */
      if(tone->input_lowpass >= MIN_FREQ)
      {
        /* Slide from current frequency to new frequency.
         * This implements simple P controller, with asymptotic behaviour.
         * Do calculation in logarithmic scape.
         */
        double new = log(tone->new_freq);
        double old = log(tone->freq);
        old+=(new-old)/(SAMPLE_RATE*tone->input_lowpass);
        tone->freq = exp(old);
      }
      else
      {
        /* If input lowpass is 0, jump straight to new frequency */
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
    .freq = MIN_FREQ,
    .new_freq = MIN_FREQ,
    .phase = 0.0,
    .input_lowpass = INPUT_LOWPASS,
    .mtx = mtx
  };
  if(argc>1)
  {
    if(!sscanf(argv[1],"%lf",&(tone.input_lowpass)))
    {
      fprintf(stderr,"Usage: %s [lowpass=0.01]\n\n",argv[0]);
      fputs("lowpass specifies how fast the sound reacts to input\n", stderr);
      fputs("        Higher values mean slower reactions\n", stderr);
      fputs("        0 means instantaneous transitions\n", stderr);
      return(-1);
    }
  }
  SDL_AudioSpec audio;
  audio.freq = SAMPLE_RATE;
  audio.format = AUDIO_S32SYS; /* 32-bit signed int */
  audio.channels = 1; /* Mono */
  audio.samples = 512;
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
  double new_freq;
  while(scanf("%lf",&new_freq))
  {
    /* Clamp input, to prevent aliasing and -inf in log scale */
    new_freq = fmax( MIN_FREQ , fmin( new_freq , SAMPLE_RATE/2 ));
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
