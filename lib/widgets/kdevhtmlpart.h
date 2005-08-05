#ifndef __KDEVHTMLPART_H__
#define __KDEVHTMLPART_H__

#include <stdlib.h>

#include <qdatetime.h>

#include <khtml_part.h>

/**
@file kdevhtmlpart.h
Customized KHTML part for KDevelop.
*/

class KAction;
class KToolBarPopupAction;
class KParts::ReadOnlyPart;

struct DocumentationHistoryEntry {
    KURL url;
    int id;
    
    DocumentationHistoryEntry() {}
    DocumentationHistoryEntry( const KURL& u ): url( u ) 
    {
        id = abs( QTime::currentTime().msecsTo( QTime() ) );	// nasty, but should provide a reasonably unique number
    }
};

/**
Customized KHTML part for KDevelop.
Used as HTML documentation and file viewer. 

Represents customized BrowserViewGUI mode of KHTMLPart. Provides also actions for:
- reload;
- stop;
- duplicate;
- print;
- copy text;
- back;
- forward.
.
It has it's own popup menu and font/zoom settings.
*/
class KDevHTMLPart : public KHTMLPart
{
    Q_OBJECT

public:
    
    enum Options { CanDuplicate=1, CanOpenInNewWindow=2 };

    KDevHTMLPart();
    
    void setContext(const QString &context);
    QString context() const;
    virtual bool openURL(const KURL &url);
    static QString resolveEnvVarsInURL(const QString& url);
    
    void setOptions(int options) { m_options = options; }

signals:
    void fileNameChanged(KParts::ReadOnlyPart *part);

protected slots:

    void slotStarted(KIO::Job *);
    void slotCompleted();
    void slotCancelled(const QString &errMsg);
    
    void openURLRequest(const KURL &url);
    void popup( const QString & url, const QPoint & p );
    
    void slotReload();
    void slotStop();
    virtual void slotDuplicate() = 0;
    virtual void slotOpenInNewWindow(const KURL &url) = 0;
    void slotPrint();
    void slotCopy();
    void slotSelectionChanged();

    void slotBack();
    void slotForward();
    void slotBackAboutToShow();
    void slotForwardAboutToShow();

    void slotPopupActivated( int id );
    void addHistoryEntry();
  
  
private:

    QValueList< DocumentationHistoryEntry > m_history;
    QValueList< DocumentationHistoryEntry >::Iterator m_Current;

    KToolBarPopupAction* m_backAction;
    KToolBarPopupAction* m_forwardAction;
    
    bool m_restoring;

    QString m_context;
    KAction *stopAction;
    KAction *reloadAction;
    KAction *duplicateAction;
    KAction *printAction;
    KAction *copyAction;
    
    int m_options;
};

#endif
