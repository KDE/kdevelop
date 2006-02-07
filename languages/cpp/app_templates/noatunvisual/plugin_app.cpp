
#include <sys/wait.h>
#include <sys/types.h>

#include "plugin_%{APPNAMELC}.h"
#include "plugin_%{APPNAMELC}_impl.h"

#include <kglobal.h>
#include <klocale.h>

extern "C"
{
    Plugin *create_plugin()
    {
	KGlobal::locale()->insertCatalog("%{APPNAMELC}");
        return new %{APPNAME}Scope();
    }
}

%{APPNAME}Scope::%{APPNAME}Scope()
    : MonoScope(50), Plugin()
{
    setSamples(320);
}

%{APPNAME}Scope::~%{APPNAME}Scope()
{
    ::close(mOutFd);
    wait(0);
}

void %{APPNAME}Scope::init()
{
    int pipes[2];
    ::pipe(pipes);
    mOutFd=pipes[1];
    if (!fork())
    {
        ::close(pipes[1]);
        new %{APPNAME}View(pipes[0]);
        exit(0);
    }
    else
    {
        fcntl(mOutFd, F_SETFL, fcntl(mOutFd, F_GETFL) & ~O_NONBLOCK);
        ::close(pipes[0]);
        MonoScope::start();
    }    
}

void %{APPNAME}Scope::scopeEvent(float *d, int size)
{
    if (::write(mOutFd, (char *)d, size * sizeof(float))==-1)
    {
        MonoScope::stop();
        unload();
    }    
}
