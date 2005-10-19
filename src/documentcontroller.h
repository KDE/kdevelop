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

    //BEGIN KDevDocumentController
    void setEncoding( const QString &encoding );
    void editDocument( const KURL &inputUrl, int lineNum = -1,
                       int col = -1 );

    void showDocument( const KURL &url, bool newWin = false );
    void showPart( KParts::Part* part, const QString& name,
                   const QString& shortDescription );

    KParts::ReadOnlyPart *partForURL( const KURL &url );
    KTextEditor::Document* textPartForURL( const KURL & url );
    /*    void* designerPartForURL( const KURL & url );*/
    KDevHTMLPart* htmlPartForURL( const KURL & url );
    KDevDocumentType documentTypeForURL( const KURL & url );

    KParts::Part * partForWidget( const QWidget * widget );

    KURL::List openURLs();

    bool saveAllDocuments();
    bool saveDocument( const KURL & url, bool force = false );
    bool saveDocuments( const KURL::List & list );

    void reloadAllDocuments();
    void reloadDocument( const KURL & url );
    void reloadDocuments( const KURL::List & list );

    bool closeAllDocuments();
    bool closeDocument( const KURL & );
    bool closeDocuments( const KURL::List & list );
    bool closeAllOthers( const KURL & );

    bool closePart( KParts::Part * part );
    void activatePart( KParts::Part * part );

    DocumentState documentState( KURL const & );
    KURL activeDocument();
    KDevDocumentType activeDocumentType();

    static void createInstance( QWidget *parent );
    static DocumentController *getInstance();
    //END KDevDocumentController

    bool readyToClose();
    bool querySaveDocuments();
    void openEmptyTextDocument();
    void integrateTextEditorPart( KTextEditor::Document* doc );
    void editDocumentInternal( const KURL &inputUrl, int lineNum = -1,
                               int col = -1, bool activate = true );

public slots:
    void slotSave();
    void slotReload();
    void slotCloseWindow();
    void slotCloseAllWindows();
    void slotCloseOtherWindows();
    void slotActivePartChanged( KParts::Part* part );

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
    void slotNewStatus( KTextEditor::Document * doc );
    void slotNewDesignerStatus( const QString &formName, int status );

private:
    void setupActions();
    void doEmitState( KURL const & );

    KURL findURLInProject( const KURL& url );
    KParts::Part* findOpenDocument( const KURL& url );
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

    bool saveDocumentsDialog( KURL::List const & ignoreList );
    bool closeDocumentsDialog( KURL::List const & ignoreList );

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

    QList<HistoryEntry> m_backHistory;
    QList<HistoryEntry> m_forwardHistory;
    bool m_isJumping;

    HistoryEntry createHistoryEntry();
    void addHistoryEntry();
    void jumpTo( const HistoryEntry & );
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
