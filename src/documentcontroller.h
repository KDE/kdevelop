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
class HTMLDocumentationPart;
class KDirWatch;
class KDevHTMLPart;

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
    KDevDocument* editDocument( const KUrl &inputUrl, const KTextEditor::Cursor& cursor = KTextEditor::Cursor::invalid() );

    KDevDocument* showDocumentation( const KUrl& url, bool newWin = false );
    KDevDocument* showPart( KParts::Part* part, const QString& name,
                   const QString& shortDescription );

    KDevDocument* documentForPart( KParts::Part* part ) const;

    KParts::Part* partForWidget( const QWidget * widget ) const;

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

    bool closePart( KParts::Part * part );

    void activateDocument( KDevDocument* document );
    void activatePart( KParts::Part * part );

    KDevDocument::DocumentState documentState( KDevDocument* document ) const;
    KDevDocument* activeDocument() const;
    //END KDevDocumentController

    //BEGIN PartManager overrides
    virtual void addPart (KParts::Part *part, bool setActive=true);
    virtual void removePart (KParts::Part *part);
    virtual void replacePart (KParts::Part *oldPart, KParts::Part *newPart, bool setActive=true);
    virtual void setActivePart (KParts::Part *part, QWidget *widget=0L);
    //END

    KDevHTMLPart* htmlPartForURL( KDevDocument* document ) const;

    static void createInstance( QWidget *parent );
    static DocumentController *getInstance();
    bool readyToClose();
    bool querySaveDocuments();
    void openEmptyTextDocument();
    void integrateTextEditorPart( KTextEditor::Document* doc );
    KDevDocument* editDocumentInternal( const KUrl &inputUrl, const KTextEditor::Cursor& cursor = KTextEditor::Cursor::invalid(), bool activate = true );

public slots:
    void slotSave();
    void slotReload();
    void slotCloseWindow();
    void slotCloseAllWindows();
    void slotCloseOtherWindows();
    void slotHTMLDocumentURLChanged( const KUrl& oldUrl, const KUrl& newUrl );

private slots:
    void slotWaitForFactoryHack();

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
    void setupActions();
    void doEmitState( KDevDocument* document);

    KUrl findUrlInProject( const KUrl& url ) const;
    KParts::Part* findOpenDocument( const KUrl& url ) const;
    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName
                                      = QString::null );

    KTextEditor::Document *createEditorPart( bool activate );

    KDevDocument* integratePart( KParts::Part *part,
                        QWidget* widget = 0,
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
