#ifndef __PARTCONTROLLER_H__
#define __PARTCONTROLLER_H__


#include <qwidget.h>
#include <qptrlist.h>
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


#include "kdevpartcontroller.h"


class PartListEntry;

class PartController : public KDevPartController
{
  Q_OBJECT

public:

  PartController(QWidget *toplevel);
  ~PartController();

  void setServiceType(const QString &serviceType);
  void editDocument(const KURL &url, int lineNum=-1);
  void showDocument(const KURL &url, int lineNum=-1);
  void showDocument(const KURL &url, const QString& context);

  static void createInstance(QWidget *parent);
  static PartController *getInstance();

  void saveAllFiles();
  void revertAllFiles();

  bool closeDocuments(const QStringList &list);

  void gotoExecutionPoint(const KURL &url, int lineNum=-1);
  void clearExecutionPoint();

  void setEncoding(const QString &encoding);

public slots:

  void slotCurrentChanged(QWidget *w);


protected:

  PartController(QWidget *parent, QWidget *mainwindow, const char *name=0);

private slots:

  void slotPartAdded(KParts::Part *part);
  void slotPartRemoved(KParts::Part *part);
  void slotActivePartChanged(KParts::Part *part);

private:

  void setLineNumber(int lineNum);
  void closeActivePart();
  void closePart(KParts::Part *part);

  KParts::Factory *findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName = QString::null );

  void integratePart(KParts::Part *part, const KURL &url);

  KParts::Part *partForURL(const KURL &url);
  void activatePart(KParts::Part *part);

  void updateMenuItems();
  void updateBufferMenu();

  void editText(const KURL &url, int num);

  static PartController *s_instance;
  
  // Not used at present because the editor is hard wired in KDevelop
  QString m_presetEncoding;

  KParts::PartManager *m_partManager;
  QPtrList<PartListEntry> m_partList;
};



#endif
