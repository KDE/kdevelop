/**
 * The interface to an editor
 */

#ifndef _KDEVEDITORMANAGERIFACE_H_
#define _KDEVEDITORMANAGERIFACE_H_

#include <dcopobject.h>

class KDevEditorManager;


class KDevEditorManagerIface : public DCOPObject
{
    K_DCOP
    
public:
    
    KDevEditorManagerIface( KDevEditorManager *editorManager );
    ~KDevEditorManagerIface();

k_dcop:
    void gotoSourceFile(const QString& url, int lineNum);
    void gotoDocumentationFile(const QString& url);

private:
    KDevEditorManager *m_editorManager;
};

#endif
