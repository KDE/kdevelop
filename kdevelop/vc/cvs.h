#ifndef _CVS_H_
#define _CVS_H_

#include "versioncontrol.h"


class CVS : public VersionControl
{
public:
    virtual void add(const char *filename, const char *text);
    virtual void remove(const char *filename, const char *text);
    virtual bool isRegistered(const char *filename);
};

#endif
