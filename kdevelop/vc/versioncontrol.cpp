#include <assert.h>
#include "versioncontrol.h"
#include "cvs.h"


void VersionControl::getSupportedSystems(QStrList *list)
{
    list->append("CVS");
}


VersionControl *VersionControl::getVersionControl(const char *name)
{
    if (qstrcmp(name, "CVS") == 0)
	return new CVS();
    else
	return 0;
}
