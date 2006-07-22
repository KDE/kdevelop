#ifndef __DOCUMENTCONTROLLER_H__
#define __DOCUMENTCONTROLLER_H__

#include "kdevdocumentcontroller.h"

#include <QMap>
#include <QWidget>
#include <QPointer>
#include <QDateTime>
#include <QHash>

#include <kurl.h>

#include <ktexteditor/modificationinterface.h>
#include "shellexport.h"

class PartController;

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
}

class QTabWidget;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class KDirWatch;

/**
Implementation for the KDevDocumentController Interface
*/
class KDEVSHELL_EXPORT DocumentController : public KDevDocumentController
{
    Q_OBJECT
public:
    DocumentController( QWidget *toplevel );
    virtual ~DocumentController();

    //BEGIN KDevDocumentController
    void setEncoding( const QString &encoding );
    KDevDocument* editDocument( const KUrl &inputUrl,
                                const KTextEditor::Cursor& cursor =
                                    KTextEditor::Cursor::invalid(),
                                bool activate = true );

    KDevDocument* showDocumentation( const KUrl& url, bool newWin = false );

    KDevDocument* documentForPart( KParts::Part* part ) const;

    QList<KDevDocument*> openDocuments() const;

    bool saveAllDocuments();
    bool saveDocument( KDevDocument* document, bool force = false );
    bool saveDocuments( const QList<KDevDocument*> & list );

    void reloadAllDocuments();
    void reloadDocument( KDevDocument* document );
    void reloadDocuments( const QList<KDevDocument*> & list );

    bool closeAllDocuments();
    bool closeDocument( KDevDocument* document );
    bool closeDocuments( const QList<KDevDocument*> & list );
    bool closeAllOthers( KDevDocument* document );

    void activateDocument( KDevDocument* document );

    KDevDocument::DocumentState documentState( KDevDocument* document ) const;
    KDevDocument* activeDocument() const;
    //END KDevDocumentController

    static void createInstance( QWidget *parent );
    static DocumentController *getInstance();
    bool readyToClose();
    bool querySaveDocuments();
    void openEmptyTextDocument();
    void integrateTextEditorPart( KTextEditor::Document* doc );

signals:
    void openingDocument( const QString &document );

public slots:
    void slotSave();
    void slotReload();
    void slotCloseWindow();
    void slotCloseAllWindows();
    void slotCloseOtherWindows();

private slots:
    void slotSaveAllDocuments();
    void slotRevertAllDocuments();

    void slotOpenDocument();
    void slotOpenRecent( const KUrl& );

    void slotBack();
    void slotForward();
    void slotBackAboutToShow();
    void slotForwardAboutToShow();
    void slotBackPopupActivated( int id );
    void slotForwardPopupActivated( int id );

    void slotSwitchTo();

    void slotUploadFinished();

    void updateMenuItems();

    void slotDocumentDirty( KTextEditor::Document * doc,
                            bool isModified,
                            KTextEditor::ModificationInterface::ModifiedOnDiskReason reason );
    void slotNewStatus( KTextEditor::Document * doc );
    void slotNewDesignerStatus( const QString &formName, int status );

private:
    void setCursorPosition( KParts::Part *part,
                            const KTextEditor::Cursor& cursor );
    bool openAsDialog( const KUrl &url, KMimeType::Ptr mimeType );
    KDevDocument *addDocument( KParts::Part * part, bool setActive = true );
    void removeDocument( KDevDocument* document );
    void replaceDocument( KDevDocument* newDocument,
                          KDevDocument* oldDocument, bool setActive = true );
    void setActiveDocument( KDevDocument* document, QWidget *widget = 0L );

    void setupActions();
    void doEmitState( KDevDocument* document );

    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName
                                      = QString::null );

    KDevDocument* integratePart( KParts::Part *part,
                                 bool activate = true );

    QList<KDevDocument*> modifiedDocuments() const;
    void clearModified( QList<KDevDocument*> const & filelist );

    bool isDirty( KDevDocument* document ) const;
    bool reactToDirty( KDevDocument* document, unsigned char reason );

    KUrl storedUrlForDocument( KDevDocument* ) const;
    void updateDocumentUrl( KDevDocument* );
    bool documentUrlHasChanged( KDevDocument* );

    bool saveDocumentsDialog( QList<KDevDocument*> const & ignoreList = QList<KDevDocument*>() );
    bool closeDocumentsDialog( QList<KDevDocument*> const & ignoreList = QList<KDevDocument*>() );

    KParts::ReadOnlyPart* activeReadOnly() const;
    KParts::ReadWritePart* activeReadWrite() const;
    KParts::ReadOnlyPart* readOnly( KParts::Part *part ) const;
    KParts::ReadWritePart* readWrite( KParts::Part *part ) const;

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

    QList<KDevDocument*> m_dirtyDocuments;

    QHash<KParts::ReadOnlyPart*, KDevDocument*> m_partHash;

    // used to note when a URL changes (a document changes url)
    QHash< KDevDocument*, KUrl > m_documentUrls;

    QPointer<KParts::Factory> m_editorFactory;
    QPointer<PartController> m_partController;

    struct HistoryEntry
    {
        HistoryEntry()
        {}
        HistoryEntry( const KUrl & u, const KTextEditor::Cursor& cursor );

        KUrl url;
        KTextEditor::Cursor cursor;
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
