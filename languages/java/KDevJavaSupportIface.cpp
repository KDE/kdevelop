
#include "KDevJavaSupportIface.h"
#include "javasupportpart.h"

KDevJavaSupportIface::KDevJavaSupportIface( JavaSupportPart* javaSupport )
    : QObject( javaSupport ), DCOPObject( "KDevJavaSupport" ), m_javaSupport( javaSupport )
{
}

KDevJavaSupportIface::~KDevJavaSupportIface()
{
}

void KDevJavaSupportIface::addClass()
{
    m_javaSupport->slotNewClass();
}

void KDevJavaSupportIface::parseProject()
{
    m_javaSupport->parseProject();
}

#include "KDevJavaSupportIface.moc"
