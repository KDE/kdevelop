#ifndef __KDEV_PARTCONTROLLER_H__
#define __KDEV_PARTCONTROLLER_H__


#include <qwidget.h>


#include <kurl.h>


namespace KParts
{
  class Part;
};


class KDevPartController : public QWidget
{
  Q_OBJECT

public:

  KDevPartController(QWidget *parent=0, const char *name=0);

  virtual void editDocument(const KURL &url, int lineNum=-1) = 0;
  virtual void showDocument(const KURL &url, int lineNum=-1) = 0;

  virtual void saveAllFiles() = 0; 
  virtual void revertAllFiles() = 0;

  virtual KParts::Part *activePart() = 0;


signals:

  void partAdded(KParts::Part *);
  void partRemoved(KParts::Part *);
  void activePartChanged(KParts::Part *);

};



#endif
