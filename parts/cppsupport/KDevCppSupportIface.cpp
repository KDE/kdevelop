
#include "KDevCppSupportIface.h"
#include "cppsupportpart.h"

KDevCppSupportIface::KDevCppSupportIface( CppSupportPart* cppSupport )
    : QObject( cppSupport ), DCOPObject( "KDevCppSupport" ), m_cppSupport( cppSupport )
{
}

KDevCppSupportIface::~KDevCppSupportIface()
{
}

void KDevCppSupportIface::addClass()
{
    m_cppSupport->slotNewClass();
}

void KDevCppSupportIface::parseProject()
{
    m_cppSupport->parseProject();
}

#include "KDevCppSupportIface.moc"
