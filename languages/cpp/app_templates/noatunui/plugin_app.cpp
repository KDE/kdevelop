
#include "plugin_%{APPNAMELC}_impl.h"

#include <kglobal.h>
#include <klocale.h>

extern "C"
{
    Plugin *create_plugin()
    {
	KGlobal::locale()->insertCatalog("%{APPNAMELC}");
        return new %{APPNAME}UI;
    }
}

