/*
 * Copyright (C) $YEAR$ $AUTHOR$ <$EMAIL$>
 */

#ifndef $APPNAMEUC$_IMPL_H
#define $APPNAMEUC$_IMPL_H

struct SDL_Surface;

template<class Pixel> class Bitmap
{
public:
    int width, height, extra;
    Pixel *data;

    Bitmap(int e=0) : extra(e), data(0) { }
    ~Bitmap() { delete[] data; }

    inline void addPixel(int x, int y, int bright1, int bright2);
    void addVertLine(int x, int y, int y2, int br1, int br2);
    
    void fadeStar();
    
    void size(int w,int h)
    {
        delete[] data;
        width = w;
        height = h;
        data = new Pixel[w*h+extra];
        clear();
    }

    void clear()
    {
        memset(data,0,sizeof(Pixel)*(width*height+extra));
    }
};


class $APPNAME$View
{
public:
    $APPNAME$View(int in);
    ~$APPNAME$View();
    
protected:
    void startVideo();
    void checkInput();
    void setupPalette(double dummy=0.0);

    void repaint();

private:
    int mFd;

    SDL_Surface *surface;
    Bitmap<unsigned short> outputBmp;
    
    bool fullscreen;
    int width;
    int height;
};

#endif // $APPNAMEUC$_IMPL_H

