
#include "plugin_%{APPNAMELC}_impl.h"

extern "C"
{
    Plugin *create_plugin()
    {
        return new %{APPNAME}UI;
    }
}

