#include <assert.h>
#include "versioncontrol.h"
#include "cvs.h"


QStringList VersionControl::getSupportedSystems()
{
    QStringList list;
    list.append("CVS");
    return list;
}


VersionControl *VersionControl::getVersionControl(const char *name)
{
    if (qstrcmp(name, "CVS") == 0)
	return new CVS();
    else
	return 0;
}
