#ifndef _KDEVELOPIFACE_H_
#define _KDEVELOPIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>
#include "ckdevelop.h"


class KDevelopIface : virtual public DCOPObject
{
    K_DCOP
    
k_dcop:
    virtual void openDocument(const QString &filename) = 0;
};
    
#endif
