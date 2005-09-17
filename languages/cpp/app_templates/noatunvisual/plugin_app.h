 
/** Note: You only have to change plugin_%{APPNAMELC}_impl.[h,cpp]  */

#ifndef _PLUGIN_%{APPNAMEUC}_H_
#define _PLUGIN_%{APPNAMEUC}_H_

#include <noatun/plugin.h>
#include <string.h>

extern "C"
{
    #include <SDL.h>
    #include <fcntl.h>
    #include <unistd.h>
}

class %{APPNAME}Scope : public MonoScope, public Plugin
{
NOATUNPLUGIND

public:
    %{APPNAME}Scope();
    virtual ~%{APPNAME}Scope();

    /** @short create a new child process (using fork) with %{APPNAME}View  */
    void init();

protected:
    virtual void scopeEvent(float *data, int bands);

private:
    int mOutFd;
};

#endif // _PLUGIN_%{APPNAMEUC}_H_

