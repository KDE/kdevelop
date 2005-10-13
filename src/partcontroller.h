#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__

#include "kdevpartcontroller.h"

#include <qwidget.h>
#include <qdatetime.h>
#include <kurl.h>
#include <qmap.h>
#include <qpointer.h>

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
Part controler implementation.
*/
class PartController : public KDevPartController
{
  Q_OBJECT

public:

  PartController(QWidget *toplevel);
  static void createInstance(QWidget *parent);
  static PartController *getInstance();

  ///// KDevPartController interface

  void setEncoding(const QString &encoding);
  void editDocument(const KURL &inputUrl, int lineNum=-1, int col=-1);
  void editDocumentInternal(const KURL &inputUrl, int lineNum=-1, int col=-1, bool activate = true);
  void integrateTextEditorPart(KTextEditor::Document* doc);

  void showDocument(const KURL &url, bool newWin = false);
  void showPart( KParts::Part* part, const QString& name, const QString& shortDescription );

  KParts::ReadOnlyPart *partForURL(const KURL &url);
  KParts::Part * partForWidget( const QWidget * widget );

  void activatePart( KParts::Part * part );
  bool closePart( KParts::Part * part );

  KURL::List openURLs();

  bool querySaveDocuments();

  bool saveAllDocuments();
  bool saveDocuments( const KURL::List & list);
  bool saveFile( const KURL & url, bool force = false );

  void revertAllDocuments();
  void revertDocuments( const KURL::List & list );

  bool closeAllDocuments();
  bool closeDocuments( const KURL::List & list );

  DocumentState documentState( KURL const & );

  ////////////////////////////////////////

  bool readyToClose();

  bool closeFile( const KURL & );
  bool closeAllOthers( const KURL & );
  void reloadFile( const KURL & url );

  void openEmptyTextDocument();

public slots:

  void slotActivePartChanged( KParts::Part* part );
  void slotCloseWindow();
  void slotCloseOtherWindows();
  void slotCloseAllWindows();

  void slotSave();
  void slotReload();

protected:

  ~PartController();

private slots:

  void slotWaitForFactoryHack();

  void slotSaveAllDocuments();
  void slotRevertAllDocuments();

  void slotOpenFile();
  void slotOpenRecent(const KURL&);

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

  void slotDocumentDirty( KTextEditor::Document * doc, bool isModified, ModifiedOnDiskReason reason );
  void slotNewStatus();
  void slotNewDesignerStatus(const QString &formName, int status);

private:
  KURL findURLInProject(const KURL& url);
  KParts::Part* findOpenDocument(const KURL& url);

  void setupActions();

  bool closeDocumentsDialog( KURL::List const & ignoreList );
  bool saveDocumentsDialog( KURL::List const & ignoreList );

  void doEmitState( KURL const & );

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );
  KTextEditor::Document *createEditorPart(bool activate);

  void integratePart(KParts::Part *part, const KURL &url, QWidget* widget = 0, bool isTextEditor=false, bool activate=true );

  // returns a list of modified documents
  KURL::List modifiedDocuments();
  void clearModified( KURL::List const & filelist );

  bool isDirty( KURL const & url );
  bool reactToDirty( KURL const & url, unsigned char reason );

  KURL storedURLForPart( KParts::ReadOnlyPart * );
  void updatePartURL( KParts::ReadOnlyPart * );
  bool partURLHasChanged( KParts::ReadOnlyPart * );

  static PartController *s_instance;

  KAction *m_closeWindowAction, *m_saveAllDocumentsAction, *m_revertAllDocumentsAction;
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction, *m_switchToAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;

  KToolBarPopupAction* m_backAction;
  KToolBarPopupAction* m_forwardAction;

  bool m_openNextAsText;

  QList<KParts::ReadWritePart*> _dirtyDocuments;

  QMap< KParts::ReadOnlyPart*, KURL > _partURLMap;  // used to note when a URL changes (a file changes name)

  QPointer<KParts::Factory> _editorFactory;

    struct HistoryEntry
    {
        HistoryEntry() {}
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
