
#ifndef %{APPNAMEUC}_IMPL_H
#define %{APPNAMEUC}_IMPL_H

struct SDL_Surface;

/**
 * @short This class is used for painting and supports some effects.
 * Note: This is used in Blurscope. Feel free to remove this and
 *       implement your own drawing routines!
 */
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

/**
 * @short This class does:
 * o set up view
 * o drawing routines.
 */
class %{APPNAME}View
{
public:
    %{APPNAME}View(int in);
    ~%{APPNAME}View();
    
protected:
    /** Screen initialization with SDL. Note, that you can initialize OpenGL with SDL!  */
    void startVideo();
    /** SDL event queue  */
    void checkInput();
    /** Used in Blurscope. Feel free to implement your own drawing routines!  */
    void setupPalette(double dummy=0.0);

    /** Draw everything.  */
    void repaint();

private:
    /** used for pipelining  */
    int mFd;

    /** SDL screen surface  */
    SDL_Surface *surface;
    Bitmap<unsigned short> outputBmp;
    
    bool fullscreen;
    int width;
    int height;
};

#endif // %{APPNAMEUC}_IMPL_H

