%{CPP_TEMPLATE}

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include "SDL.h"

using namespace std;

int main(int argc, char *argv[])
{
 cout <<"Initializing SDL." << endl;
  /* Initializes Audio and the CDROM, add SDL_INIT_VIDEO for Video */
  if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_CDROM)< 0) {
    cout <<"Could not initialize SDL:" << SDL_GetError() << endl;
       SDL_Quit();
  } else {
    cout << "Audio & CDROM initialized correctly" << endl;;
    /* Trying to read number of CD devices on system */
	cout << "Drives available :" << SDL_CDNumDrives() << endl;
    for(int i=0; i < SDL_CDNumDrives(); ++i) {
      cout << "Drive " << i << "\"" << SDL_CDName(i) << "\"";
    }
  }
  SDL_Quit();
}
