/**
 * The interface to an editor
 */

#ifndef _KDEVEDITORMANAGER_H_
#define _KDEVEDITORMANAGER_H_

#include <qstringlist.h>
#include "kdevcomponent.h"


class KDevEditorManager : public KDevComponent
{
    Q_OBJECT
    
public:
    
    KDevEditorManager( QObject *parent=0, const char *name=0 );
    ~KDevEditorManager();

    /**
     * Loads a file into the editor and jump to a line number.
     */
    virtual void gotoSourceFile(const KURL& url, int lineNum) = 0;
    /**
     * Loads a file into the HTML viewer.
     */
    virtual void gotoDocumentationFile(const KURL& url) = 0;

signals:
    /**
     * Emitted when a file has been saved.
     */
    void sigSavedFile(const QString &fileName);
};

#endif
