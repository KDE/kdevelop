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
	class Editor; 
}

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
  void showDocument(const KURL &url, bool newWin = false);
  void showPart( KParts::Part* part, const QString& name, const QString& shortDescription );

  KParts::ReadOnlyPart *partForURL(const KURL &url);
  KParts::Part * partForWidget( const QWidget * widget );
  
  void activatePart( KParts::Part * part );
  bool closePart( KParts::Part * part );

  KURL::List openURLs();

  void saveAllFiles();
  void saveFiles( const KURL::List & list);
  
  void revertAllFiles();
  void revertFiles( const KURL::List & list );
  
  bool closeAllFiles();
  bool closeFiles( const KURL::List & list );
  
  DocumentState documentState( KURL const & );
  
  ////////////////////////////////////////
  
  bool readyToClose();
  void reinstallPopups();


public slots:

  void slotCurrentChanged(QWidget *w);
  void slotActivePartChanged( KParts::Part* part );
  void slotCloseWindow();
  void slotCloseOtherWindows();
  void slotCloseAllWindows();

protected:

  ~PartController();

private slots:

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
//  void slotFileNameChanged();

  void updateMenuItems();

  void dirty( const QString& fileName );
  void slotFileDirty( const KURL & url );
  void slotNewStatus();

private:
  KURL findURLInProject(const KURL& url);
  KParts::Part* findOpenDocument(const KURL& url);

  void setupActions();

  bool closeFilesDialog( KURL::List const & ignoreList );
  
  QPopupMenu *contextPopupMenu();
  
  void doEmitState( KURL const & );

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );
  KTextEditor::Editor * createEditorPart();

  void integratePart(KParts::Part *part, const KURL &url, bool isTextEditor=false );

  // returns a list of modified documents
  KURL::List modifiedDocuments();
  void clearModified( KURL::List const & filelist );
  
  bool isDirty( KURL const & url );
    
  void revertFile(KParts::Part *part);
  void saveFile(KParts::Part *part);

  KURL storedURLForPart( KParts::ReadOnlyPart * );
  void updatePartURL( KParts::ReadOnlyPart * );
  bool partURLHasChanged( KParts::ReadOnlyPart * );
  void updateTimestamp( KURL const & );
  void removeTimestamp( KURL const & );
  
  static PartController *s_instance;

  KAction *m_closeWindowAction, *m_saveAllFilesAction, *m_revertAllFilesAction;
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction, *m_switchToAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;

  DocumentationPart *findDocPart(const QString &context);

  KToolBarPopupAction* m_backAction;
  KToolBarPopupAction* m_forwardAction;
  
  KDirWatch* dirWatcher;
  
  bool m_openNextAsText;
  
  QMap< KURL, QDateTime > accessTimeMap;
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
