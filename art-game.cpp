// Using SDL and standard IO
#include <SDL.h>
#include <SDL_audio.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int zero_clamp(int num) {
  return num < 1 ? 1 : num;
}

const int AMPLITUDE = 28000;
const int SAMPLE_RATE = 44100;

int clarity = 255;
int amplitude = 2;
float toned = 1;
float toned_freq = 261.63; 

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
    Sint16 *buffer = (Sint16*)raw_buffer;
    int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
    int &sample_nr(*(int*)user_data);

    for(int i = 0; i < length; i++, sample_nr++)
    {
        double time = (double)sample_nr / (double)SAMPLE_RATE;
        float saw = (float)i / ((float)SAMPLE_RATE / toned_freq);
        saw = saw - floorf(saw); 
        float noise = (float)((rand() % ((clarity / 2) * amplitude))) / clarity;
        buffer[i] = (Sint16)(AMPLITUDE * (noise + saw * toned)); // render 441 HZ sine wave
    }
}

int main(int argc, char *args[]) {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer;
  SDL_Surface *screenSurface = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf(
      "SDL could not initialize! SDL_Error: %s\n", 
      SDL_GetError()
    );
  } 
  else {
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    
    if (window == NULL) {
      printf(
        "Window could not be created! SDL_Error: %s\n", 
        SDL_GetError()
      );
    } 
    else {
      
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);

      SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
      
      Uint32* pixels = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];

      // Hack to get window to stay up
      SDL_Event e;
      bool quit = false;

      int redMin = 0;
      int redMax = 0;
      int greenMin = 0;
      int greenMax = 0;
      int blueMin = 0;
      int blueMax = 0;

      int sample_nr = 0;

      SDL_AudioSpec want;
      want.freq = SAMPLE_RATE; // number of samples per second
      want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
      want.channels = 1; // only one channel
      want.samples = 2048; // buffer-size
      want.callback = audio_callback; // function SDL calls periodically to refill the buffer
      want.userdata = &sample_nr; // counter, keeping track of current sample number

      SDL_AudioSpec have;
      if(SDL_OpenAudio(&want, &have) != 0) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
      if(want.format != have.format) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");


      SDL_PauseAudio(0); 
      while (quit == false) {
        while (SDL_PollEvent(&e)) {
          if (e.type == SDL_QUIT)
            quit = true;
          if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_j) {
              redMin = rand() % 20;
              redMax = rand() % 20;
            }
            if (e.key.keysym.sym == SDLK_r) {
              greenMin = rand() % 20;
              greenMax = rand() % 20;
            }
            if (e.key.keysym.sym == SDLK_v) {
              blueMin = rand() % 20;
              blueMax = rand() % 20;
            }
            if (e.key.keysym.sym == SDLK_d) {
              clarity = rand() % 1000;
            }
            if (e.key.keysym.sym == SDLK_q) {
              amplitude = (rand() % 4) + 1;
            }
            if (e.key.keysym.sym == SDLK_t) {
              toned = (float)(rand() % 15) / 5.0;
              toned_freq = (float)(rand() % 1000) / (float)(rand() % 200);
            }
          }
        }
              
        for (int x = 0; x < SCREEN_WIDTH; x++) {
          for (int y = 0; y < SCREEN_HEIGHT; y++) {
            Uint32 red = rand() % zero_clamp(256 - redMin - redMax) + redMin;
            Uint32 green = rand() % zero_clamp(256 - greenMin - greenMax) + greenMin;
            Uint32 blue = rand() % zero_clamp(256 - blueMin - blueMax) + blueMin;
            Uint32 color = 0xFF000000 | red << 16 | green << 8 | blue;
            pixels[x + y * SCREEN_WIDTH] = color;
          }
        }

        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
        
        SDL_Rect destination = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, texture, NULL, &destination);
        SDL_RenderPresent(renderer);
        
      }

      SDL_PauseAudio(1); // stop playing sound

      SDL_CloseAudio();
          
      delete[] pixels;
      SDL_DestroyTexture(texture);
    }
  }


  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();
  return 0;
}
