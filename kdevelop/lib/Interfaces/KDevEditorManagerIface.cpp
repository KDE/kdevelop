#include <kurl.h>
#include "KDevEditorManagerIface.h"
#include "kdeveditormanager.h"


KDevEditorManagerIface::KDevEditorManagerIface(KDevEditorManager *editorManager)
    : DCOPObject("KDevEditorManager")
{
    m_editorManager = editorManager;
}


KDevEditorManagerIface::~KDevEditorManagerIface()
{}


void KDevEditorManagerIface::gotoSourceFile(const QString& url, int lineNum)
{
    m_editorManager->gotoSourceFile(KURL(url), lineNum);
}


void KDevEditorManagerIface::gotoDocumentationFile(const QString& url)
{
    m_editorManager->gotoDocumentationFile(KURL(url));
}
