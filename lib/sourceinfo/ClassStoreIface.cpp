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
    
    QStringList *list = m_classStore->getSortedClassNameList();
    res = *list;
    delete list;
    
    return res;
}
