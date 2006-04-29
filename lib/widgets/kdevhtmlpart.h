#ifndef __KDEVHTMLPART_H__
#define __KDEVHTMLPART_H__

#include <stdlib.h>

#include <qdatetime.h>
#include <QLinkedList>

#include <khtml_part.h>
#include "kdevexport.h"

/**
@file kdevhtmlpart.h
Customized KHTML part for KDevelop.
*/

class KAction;
class KToolBarPopupAction;
class KParts::ReadOnlyPart;
class KDevDocument;

struct DocumentationHistoryEntry {
    KUrl url;
    int id;

    DocumentationHistoryEntry() {}
    DocumentationHistoryEntry( const KUrl& u ): url( u )
    {
        id = abs( QTime::currentTime().msecsTo( QTime() ) );    // nasty, but should provide a reasonably unique number
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
class KDEVWIDGETS_EXPORT KDevHTMLPart : public KHTMLPart
{
    Q_OBJECT

public:

    enum Options { CanDuplicate=1, CanOpenInNewWindow=2 };

    KDevHTMLPart();

    void setContext(const QString &context);
    QString context() const;
    virtual bool openURL(const KUrl &url);
    static QString resolveEnvVarsInURL(const QString& url);

    void setOptions(int options) { m_options = options; }

signals:
/*    void fileNameChanged(KParts::ReadOnlyPart *part);*/
    void documentURLChanged( const KUrl &oldURL, const KUrl &newURL );

protected slots:

    void slotStarted(KIO::Job *);
    void slotCompleted();
    void slotCancelled(const QString &errMsg);

    void openURLRequest(const KUrl &url);
    void popup( const QString & url, const QPoint & p );

    void slotReload();
    void slotStop();
    virtual void slotDuplicate() = 0;
    virtual void slotOpenInNewWindow(const KUrl &url) = 0;
    void slotPrint();
    void slotCopy();
    void slotSelectionChanged();

    void slotBack();
    void slotForward();
    void slotBackAboutToShow();
    void slotForwardAboutToShow();

    void slotPopupActivated( int id );
    void addHistoryEntry();

    QLinkedList<DocumentationHistoryEntry>::Iterator lastElement();

private:

    QLinkedList< DocumentationHistoryEntry > m_history;
    QLinkedList< DocumentationHistoryEntry >::Iterator m_Current;

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
