#ifndef _KDEVELOPIFACEIMPL_H_
#define _KDEVELOPIFACEIMPL_H_

#include "KDevelopIface.h"


class KDevelopIfaceImpl : public KDevelopIface
{
public:
    KDevelopIfaceImpl(CKDevelop *core);
    virtual void openDocument(const QString &filename);

private:
    CKDevelop *core;
};

   
#endif
