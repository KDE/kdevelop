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


#if 0
void VersionControl::add(const char *filename)
{
    cerr << "Internal error" << endl;
    return false;
}

void VersionControl::remove(const char *filename)
{
    cerr << "Internal error" << endl;
    return false;
}


void isRegistered(const char *filename)
{
    cerr << "Internal error" << endl;
    return false;
}
#endif
