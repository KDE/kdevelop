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

  void setEncoding(const QString &encoding);
  void editDocument(const KURL &inputUrl, int lineNum=-1, int col=-1);
  void showDocument(const KURL &url, const QString &context = QString::null);
  KParts::Part* findOpenDocument(const KURL& url);

  bool closeAllWindows();
  virtual bool closePartForWidget( const QWidget* widget );
  virtual bool closePartForURL( const KURL & url );

  static void createInstance(QWidget *parent);
  static PartController *getInstance();

  void saveAllFiles();
  void revertAllFiles();

  bool isDirty(KParts::ReadOnlyPart * part);
  bool isModified( KParts::Part * );
  
  bool readyToClose();

  KParts::Part *partForURL(const KURL &url);

  void reinstallPopups();

  void showPart( KParts::Part* part, const QString& name, const QString& shortDescription );


public slots:

  void slotCurrentChanged(QWidget *w);
  void slotClosePartForWidget(const QWidget *widget);
  void slotActivePartChanged( KParts::Part* part );
  void slotCloseAllWindows();

signals:
	// this is typically emitted when an editorpart does "save as"
	// which will change the part's URL
	void partURLChanged( KParts::ReadOnlyPart * );
	
	// this is emitted when the document changes, 
	// either internally or on disc
	void documentChangedState( const KURL &, DocumentState );
	

protected:

  ~PartController();


private slots:

  void slotSaveAllFiles();
  void slotRevertAllFiles();

  void slotOpenFile();
  void slotOpenRecent(const KURL&);

  void slotCloseWindow();
  void slotCloseOtherWindows();

  void slotBack();
  void slotForward();
  void slotBackAboutToShow();
  void slotBackPopupActivated( int id );
  void slotForwardAboutToShow();
  void slotForwardPopupActivated( int id );

  void slotSwitchTo();

  void slotUploadFinished();
  void slotFileNameChanged();

  void updateMenuItems();
  void saveState( KParts::Part* part );
  void restoreState();
  void addHistoryEntry( HistoryEntry* entry );

  void dirty( const QString& fileName );
  void slotFileDirty( const KURL & url );
  void slotNewStatus();

private:
  KURL findURLInProject(const KURL& url);

  void setupActions();

  bool closeWindows( KURL::List const & ignoreList );
  
  void closeActivePart();
  bool closePart(KParts::Part *part);

  QPopupMenu *contextPopupMenu();
  
  void doEmitState( KParts::ReadWritePart * );

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );
  KTextEditor::Editor * createEditorPart();

  void integratePart(KParts::Part *part, const KURL &url, bool isTextEditor=false );

  void activatePart(KParts::Part *part);

  void editText(const KURL &url, int num);

  // returns a list of modified documents
  KURL::List modifiedDocuments();
  void clearModified( KURL::List const & filelist );
  void saveFiles( KURL::List const & filelist );
  
  void revertFile(KParts::Part *part);
  void saveFile(KParts::Part *part);

  static PartController *s_instance;

  KAction *m_closeWindowAction, *m_saveAllFilesAction, *m_revertAllFilesAction;
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction, *m_switchToAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;

  DocumentationPart *findDocPart(const QString &context);

  KToolBarPopupAction* m_backAction;
  KToolBarPopupAction* m_forwardAction;
  QPtrList< HistoryEntry > m_history;
  KDirWatch* dirWatcher;
  QMap< const KParts::ReadOnlyPart*, QDateTime > accessTimeMap;
  bool m_restoring;
  QGuardedPtr<KParts::Factory> _editorFactory;
};



#endif
