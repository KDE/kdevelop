#ifndef __DOCUMENTCONTROLLER_H__
#define __DOCUMENTCONTROLLER_H__

#include "kdevdocumentcontroller.h"

#include <qmap.h>
#include <qwidget.h>
#include <qpointer.h>
#include <qdatetime.h>

#include <kurl.h>

namespace KParts
{
class Part;
class Factory;
class PartManager;
class ReadOnlyPart;
class ReadWritePart;
}

namespace KTextEditor
{
class Document;
class Editor;
class ModificationInterface;
}

typedef int ModifiedOnDiskReason;

class QTabWidget;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class HTMLDocumentationPart;
class HistoryEntry;
class KDirWatch;

/**
Implementation for the KDevDocumentController Interface
*/
class DocumentController : public KDevDocumentController
{
    Q_OBJECT
public:
    DocumentController( QWidget *toplevel );
    virtual ~DocumentController();

    static void createInstance( QWidget *parent );
    static DocumentController *getInstance();

    void setEncoding( const QString &encoding );
    void editDocument( const KURL &inputUrl, int lineNum = -1,
                       int col = -1 );
    void editDocumentInternal( const KURL &inputUrl, int lineNum = -1,
                               int col = -1, bool activate = true );
    void integrateTextEditorPart( KTextEditor::Document* doc );

    void showDocument( const KURL &url, bool newWin = false );
    void showPart( KParts::Part* part, const QString& name,
                   const QString& shortDescription );

    KParts::ReadOnlyPart *partForURL( const KURL &url );
    KParts::Part * partForWidget( const QWidget * widget );

    void activatePart( KParts::Part * part );
    bool closePart( KParts::Part * part );

    KURL::List openURLs();

    bool querySaveDocuments();

    bool saveAllDocuments();
    bool saveDocuments( const KURL::List & list );
    bool saveDocument( const KURL & url, bool force = false );

    void reloadAllDocuments();
    void reloadDocuments( const KURL::List & list );

    bool closeAllDocuments();
    bool closeDocuments( const KURL::List & list );

    DocumentState documentState( KURL const & );

    bool readyToClose();

    bool closeDocument( const KURL & );
    bool closeAllOthers( const KURL & );
    void reloadDocument( const KURL & url );

    void openEmptyTextDocument();

public slots:
    void slotActivePartChanged( KParts::Part* part );
    void slotCloseWindow();
    void slotCloseOtherWindows();
    void slotCloseAllWindows();

    void slotSave();
    void slotReload();

private slots:
    void slotWaitForFactoryHack();

    void slotSaveAllDocuments();
    void slotRevertAllDocuments();

    void slotOpenDocument();
    void slotOpenRecent( const KURL& );

    void slotBack();
    void slotForward();
    void slotBackAboutToShow();
    void slotForwardAboutToShow();
    void slotBackPopupActivated( int id );
    void slotForwardPopupActivated( int id );

    void slotSwitchTo();

    void slotPartAdded( KParts::Part* );
    void slotPartRemoved( KParts::Part* );

    void slotUploadFinished();

    void updateMenuItems();

    void slotDocumentDirty( KTextEditor::Document * doc,
                            bool isModified,
                            ModifiedOnDiskReason reason );
    void slotNewStatus();
    void slotNewDesignerStatus( const QString &formName, int status );

private:
    KURL findURLInProject( const KURL& url );
    KParts::Part* findOpenDocument( const KURL& url );

    void setupActions();

    bool closeDocumentsDialog( KURL::List const & ignoreList );
    bool saveDocumentsDialog( KURL::List const & ignoreList );

    void doEmitState( KURL const & );

    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName
                                      = QString::null );
    KTextEditor::Document *createEditorPart( bool activate );

    void integratePart( KParts::Part *part, const KURL &url,
                        QWidget* widget = 0,
                        bool isTextEditor = false,
                        bool activate = true );

    KURL::List modifiedDocuments();
    void clearModified( KURL::List const & filelist );

    bool isDirty( KURL const & url );
    bool reactToDirty( KURL const & url, unsigned char reason );

    KURL storedURLForPart( KParts::ReadOnlyPart * );
    void updatePartURL( KParts::ReadOnlyPart * );
    bool partURLHasChanged( KParts::ReadOnlyPart * );

    static DocumentController *s_instance;

    KAction *m_closeWindowAction;
    KAction *m_saveAllDocumentsAction;
    KAction *m_revertAllDocumentsAction;
    KAction *m_closeAllWindowsAction;
    KAction *m_closeOtherWindowsAction;
    KAction *m_switchToAction;
    KRecentFilesAction *m_openRecentAction;
    QString m_presetEncoding;

    KToolBarPopupAction* m_backAction;
    KToolBarPopupAction* m_forwardAction;

    bool m_openNextAsText;

    QList<KParts::ReadWritePart*> m_dirtyDocuments;

    // used to note when a URL changes (a document changes url)
    QMap< KParts::ReadOnlyPart*, KURL > m_partURLMap;

    QPointer<KParts::Factory> m_editorFactory;

    struct HistoryEntry
    {
        HistoryEntry()
        {}
        HistoryEntry( const KURL & url, int line, int col );

        KURL url;
        int line;
        int col;
        int id;
    };

    void addHistoryEntry();
    HistoryEntry createHistoryEntry();
    void jumpTo( const HistoryEntry & );

    QList<HistoryEntry> m_backHistory;
    QList<HistoryEntry> m_forwardHistory;
    bool m_isJumping;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
