
#include "plugin_%{APPNAMELC}_impl.h"

#include <string.h>
#include <stdlib.h>

extern "C"
{
    #include <SDL.h>
    #include <fcntl.h>
    #include <unistd.h>
}

#if 0
#define TEST() cout << "At line "  << __LINE__ << endl
#else
#define TEST() if(0)
#endif

#define output ((unsigned char*)data)    
template <class Type>
inline void Bitmap<Type>::addPixel(int x, int y, int br1, int br2)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    register unsigned char *p = output+x*2+y*width*2;
    if (p[0] < 255-br1) p[0] += br1; else p[0] = 255;
    if (p[1] < 255-br2) p[1] += br2; else p[1] = 255;
}

template <class Type>
void Bitmap<Type>::addVertLine(int x, int y1, int y2, int br1, int br2)
{
    if(y1 < y2)
    {
        for(int y = y1; y <= y2; y++)
            addPixel(x,y, br1, br2);
    }
    else if(y2 < y1)
    {
        for(int y = y2; y <= y1; y++)
            addPixel(x, y, br1, br2);
    }
    else
    {
        addPixel(x, y1, br1, br2);
    }
}

template <class Type>
void Bitmap<Type>::fadeStar()
{
    register unsigned long *ptr = (unsigned long*)output;
    int i = width*height*2/4;
    do
    {
        if (*ptr)
            *(ptr++) -= ((*ptr & 0xf0f0f0f0ul) >> 4) + ((*ptr & 0xe0e0e0e0ul) >> 5);
        else 
            ptr++;
    } while(--i > 0);
}

%{APPNAME}View::%{APPNAME}View(int in) : mFd(in), outputBmp(0), fullscreen(false)
{
    surface = 0;
    width = 320;
    height = width*6/8;
    outputBmp.size(width, height);
    TEST();
    
    TEST();
    fcntl(mFd, F_SETFL, fcntl(mFd, F_GETFL) & ~O_NONBLOCK);
    TEST();


    float *data = new float[width];
    TEST();

    startVideo();
    setupPalette();
    TEST();

    while(true)
    {
        checkInput();

        if(!surface) exit(0);

        int bytestoread = width * sizeof(float);
        int pos = 0;
        while(pos < bytestoread)
        {
            char *d = (char *)data;
            int r = read(mFd, d + pos, bytestoread - pos);
            if(r > 0) pos += r;
            else if (r==0) exit(0);
        }

        outputBmp.fadeStar();
        outputBmp.fadeStar();

        float *d = data;
        float *end = data + width;
        float heightHalf = height / 4.0;
        int y = height / 2;
        int x=0;
        int oldy=(int)(*d * heightHalf);
        while(d <= end)
        {
            int amp = (int)(*d * heightHalf);
            amp+=y;
            
            outputBmp.addVertLine(x, oldy, amp, 255, 255);
            oldy=amp;
            
            d++;
            x++;
        }
        repaint();
    }
}

static SDL_Surface *CreateScreen(Uint16 w, Uint16 h, Uint8 bpp, Uint32 flags)
{
    SDL_Surface *screen;

    // Set the video mode
    screen = SDL_SetVideoMode(w, h, bpp, flags);
    return screen;
}

void %{APPNAME}View::startVideo()
{
    if(surface)
    {
        SDL_FreeSurface(surface);
        surface = 0;
    }

    Uint32 videoflags;

    /* Initialize SDL */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
        exit(0);

    SDL_WM_SetCaption("%{APPNAME}","%{APPNAMELC}");

    /* See if we try to get a hardware colormap */
    videoflags = SDL_SWSURFACE | (fullscreen?SDL_FULLSCREEN:0);

    surface = CreateScreen(width, height, 8, videoflags);
    if (!surface)
        exit(0);
    
    SDL_ShowCursor(0);

    if(!surface)
        SDL_Quit();

    SDL_WM_SetCaption("%{APPNAME}", 0);
    SDL_ShowCursor(0);

}

void %{APPNAME}View::setupPalette(double)
{
#define BOUND(x) ((x) > 255 ? 255 : (x))
#define PEAKIFY(x) int(BOUND((x) - (x)*(255-(x))/255/2))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

    int redMax=136;
    int greenMax=136;
    int blueMax=255;
    
    SDL_Color sdlPalette[256];

    for(int i=0;i<256;i++)
    {
        // i
        // 255 136
        sdlPalette[i].r = i*redMax/255;
        sdlPalette[i].g = i*greenMax/255;
        sdlPalette[i].b = i*blueMax/255;
    }

    SDL_SetColors(surface, sdlPalette, 0, 256);

#undef BOUND
#undef PEAKIFY
#undef MAX
}

void %{APPNAME}View::checkInput()
{
    SDL_Event myEvent;

    while(SDL_PollEvent(&myEvent))
    {
        switch(myEvent.type)
        {
        case SDL_KEYDOWN:
            switch(myEvent.key.keysym.sym)
            {
            case SDLK_SPACE:
//                fullscreen ^= true;
//                startVideo();
                break;
            default:
                break;
            }
            break;

        case SDL_QUIT:
            exit(0);
            break;
        }
    }
}

#define output2 ((unsigned char*)outputBmp.data)

void %{APPNAME}View::repaint()
{
    SDL_LockSurface(surface);
    TEST();

    register unsigned long *ptr2 = (unsigned long*)output2;
    unsigned long *ptr1 = (unsigned long*)( surface->pixels );
    int i = width*height/4;
    TEST();

    do {
        // Asger Alstrup Nielsen's (alstrup@diku.dk)
        // optimized 32 bit screen loop
        register unsigned int const r1 = *(ptr2++);
        register unsigned int const r2 = *(ptr2++);

        //if (r1 || r2) {
#ifdef LITTLEENDIAN
        register unsigned int const v =
            ((r1 & 0x000000f0ul) >> 4)
            | ((r1 & 0x0000f000ul) >> 8)
            | ((r1 & 0x00f00000ul) >> 12)
            | ((r1 & 0xf0000000ul) >> 16);
        *(ptr1++) = v |
            ( ((r2 & 0x000000f0ul) << 12)
              | ((r2 & 0x0000f000ul) << 8)
              | ((r2 & 0x00f00000ul) << 4)
              | ((r2 & 0xf0000000ul)));
#else
        register unsigned int const v =
            ((r2 & 0x000000f0ul) >> 4)
            | ((r2 & 0x0000f000ul) >> 8)
            | ((r2 & 0x00f00000ul) >> 12)
            | ((r2 & 0xf0000000ul) >> 16);
        *(ptr1++) = v |
            ( ((r1 & 0x000000f0ul) << 12)
              | ((r1 & 0x0000f000ul) << 8)
              | ((r1 & 0x00f00000ul) << 4)
              | ((r1 & 0xf0000000ul)));
#endif
        //} else ptr1++;
    } while (--i);
    TEST();

    SDL_UnlockSurface(surface);
    SDL_UpdateRect(surface, 0, 0, 0, 0);
    TEST();

}

%{APPNAME}View::~%{APPNAME}View()
{
    ::close(mFd);
}

