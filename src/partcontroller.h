#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__


#include <qwidget.h>
#include <qarray.h>


class QTabWidget;
class QPopupMenu;


#include <kurl.h>

namespace KParts
{
  class Part;
  class Factory;
  class PartManager;
};

class KAction;
class KRecentFilesAction;

#include "kdevpartcontroller.h"


class DocumentationPart;


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

  void slotUploadFinished();

  void updateMenuItems();


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
  KAction *m_closeAllWindowsAction, *m_closeOtherWindowsAction;
  KRecentFilesAction *m_openRecentAction;
  QString m_presetEncoding;
    
  DocumentationPart *findDocPart(const QString &context);

};



#endif
