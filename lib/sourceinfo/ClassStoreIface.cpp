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
    return m_classStore->getSortedClassNameList();
}
