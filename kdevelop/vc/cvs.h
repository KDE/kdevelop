#ifndef _CVS_H_
#define _CVS_H_

#include "versioncontrol.h"


class CVS : public VersionControl
{
public:
    virtual void add(const char *filename);
    virtual void remove(const char *filename);
    virtual void update(const char *filename);
    virtual void commit(const char *filename);
    virtual State registeredState(const char *filename);
};

#endif
