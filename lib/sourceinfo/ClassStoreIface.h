#ifndef _KDEVCLASSSTOREIFACE_H_
#define _KDEVCLASSSTOREIFACE_H_

#include <dcopobject.h>
#include <qstringlist.h>

class ClassStore;


class ClassStoreIface : public DCOPObject
{
    K_DCOP

public:

    ClassStoreIface( ClassStore *classStore );
    ~ClassStoreIface();

k_dcop:
    QStringList getSortedClassNameList();

private:
    ClassStore *m_classStore;
};

#endif
