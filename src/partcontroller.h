#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__

#include "kdevpartcontroller.h"

#include <qwidget.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <kurl.h>
#include <qmap.h>
#include <qguardedptr.h>

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

namespace Kate { class Document; }

class QTabWidget;
class QPopupMenu;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class DocumentationPart;
class HistoryEntry;
class KDirWatch;

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

  bool querySaveFiles();
  
  void saveAllFiles();
  void saveFiles( const KURL::List & list);
  bool saveFile( const KURL & url, bool force = false );
  
  void revertAllFiles();
  void revertFiles( const KURL::List & list );
  
  bool closeAllFiles();
  bool closeFiles( const KURL::List & list );
  
  DocumentState documentState( KURL const & );
  
  ////////////////////////////////////////
  
  bool readyToClose();


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

  void slotSaveAllFiles();
  void slotRevertAllFiles();

  void slotOpenFile();
  void slotOpenRecent(const KURL&);

  void slotBack();
  void slotForward();
  void slotBackAboutToShow();
  void slotForwardAboutToShow();
  void slotPopupActivated( int id );
  
  void slotSwitchTo();
  
  void slotPartAdded( KParts::Part* );
  void slotPartRemoved( KParts::Part* );

  void slotUploadFinished();

  void updateMenuItems();

  void slotDocumentDirty( Kate::Document * doc, bool isModified, unsigned char reason );
  void slotNewStatus();

private:
  KURL findURLInProject(const KURL& url);
  KParts::Part* findOpenDocument(const KURL& url);

  void setupActions();

  bool closeFilesDialog( KURL::List const & ignoreList );
  bool saveFilesDialog( KURL::List const & ignoreList );
  
  void doEmitState( KURL const & );

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );
  KTextEditor::Editor * createEditorPart(bool activate);

  void integratePart(KParts::Part *part, const KURL &url, QWidget* widget = 0, bool isTextEditor=false, bool activate=true );

  // returns a list of modified documents
  KURL::List modifiedDocuments();
  void clearModified( KURL::List const & filelist );
  
  bool isDirty( KURL const & url );
  bool reactToDirty( KURL const & url, bool isModified );
    
  void reloadFile( const KURL &, bool force = false );
  
  KURL storedURLForPart( KParts::ReadOnlyPart * );
  void updatePartURL( KParts::ReadOnlyPart * );
  bool partURLHasChanged( KParts::ReadOnlyPart * );
  
  static PartController *s_instance;

  KAction *m_closeWindowAction, *m_saveAllFilesAction, *m_revertAllFilesAction;
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction, *m_switchToAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;

  KToolBarPopupAction* m_backAction;
  KToolBarPopupAction* m_forwardAction;
  
  bool m_openNextAsText;
  
  QValueList<KParts::ReadWritePart*> _dirtyDocuments;
  
  QMap< KParts::ReadOnlyPart*, KURL > _partURLMap;	// used to note when a URL changes (a file changes name)
  
  QGuardedPtr<KParts::Factory> _editorFactory;

    
	struct HistoryEntry 
	{
		HistoryEntry() {}
		HistoryEntry( const KURL & url, int line, int col );
		
		KURL url;
		int line;
		int col;
		int id;
	};

	void addHistoryEntry(const KURL & url, int line = -1, int col = -1 );
	void jumpTo( const HistoryEntry & );
		
	QValueList<HistoryEntry> m_history;
	QValueList<HistoryEntry>::Iterator m_Current;
	bool m_isJumping;
	QGuardedPtr<KParts::ReadOnlyPart> m_latestPart;
  
};



#endif
