#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__

#include "kdevpartcontroller.h"

#include <qwidget.h>
#include <qptrlist.h>
#include <kurl.h>

namespace KParts
{
  class Part;
  class Factory;
  class PartManager;
};

class QTabWidget;
class QPopupMenu;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;
class DocumentationPart;
class HistoryEntry;

class PartController : public KDevPartController
{
  Q_OBJECT

public:

  PartController(QWidget *toplevel);

  void setEncoding(const QString &encoding);
  void editDocument(const KURL &inputUrl, int lineNum=-1);
  void showDocument(const KURL &url, const QString &context = QString::null);

  bool closeDocuments(const QStringList &list);
  bool closePartForWidget( const QWidget* widget );

  static void createInstance(QWidget *parent);
  static PartController *getInstance();

  void saveAllFiles();
  void revertAllFiles();

  bool readyToClose();

  KParts::Part *partForURL(const KURL &url);


public slots:

  void slotCurrentChanged(QWidget *w);
  void slotClosePartForWidget(const QWidget *widget);
  void slotCloseAllButPartForWidget(QWidget *widget);
  void slotActivePartChanged( KParts::Part* part );

protected:

  ~PartController();


private slots:

  void slotSaveAllFiles();
  void slotRevertAllFiles();

  void slotOpenFile();
  void slotOpenRecent(const KURL&);

  void slotCloseWindow();
  void slotCloseAllWindows();
  void slotCloseOtherWindows();

  void slotBack();
  void slotForward();
  void slotBackAboutToShow();
  void slotBackPopupActivated( int id );
  void slotForwardAboutToShow();
  void slotForwardPopupActivated( int id );
  
  void slotSwitchTo();

  void slotUploadFinished();

  void updateMenuItems();
  void saveState( KParts::Part* part );
  void restoreState();
  void addHistoryEntry( HistoryEntry* entry );

private:

  void setupActions();

  void closeActivePart();
  bool closePart(KParts::Part *part);

  QPopupMenu *contextPopupMenu();

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );

  void integratePart(KParts::Part *part, const KURL &url);

  void activatePart(KParts::Part *part);

  void editText(const KURL &url, int num);

  // returns a list of modified documents
  QStringList getModifiedDocuments( KParts::Part* excludeMe = 0 );
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
  bool m_restoring;
};



#endif
