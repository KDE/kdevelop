#include "ClassStoreIface.h"
#include "classstore.h"


ClassStoreIface::ClassStoreIface(ClassStore *classStore)
    : DCOPObject("ClassStore")
{
    m_classStore = classStore;
}


ClassStoreIface::~ClassStoreIface()
{}


QStringList ClassStoreIface::getSortedClassNameList()
{
    QStringList res;
    
    QStrList *list = m_classStore->getSortedClassNameList();
    QStrListIterator it(*list);
    for (; it.current(); ++it)
        res += it.current();

    delete list;
    
    return res;
}
