#ifndef __KONSOLEVIEWWIDGET_H__
#define __KONSOLEVIEWWIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;
class KonsoleWidgetPrivate;
class KonsoleViewPart;
class KParts::ReadOnlyPart;
class QVBoxLayout;


#include "keditor/editor.h"


class KonsoleViewWidget : public QWidget
{
  Q_OBJECT
    
public:
		  
  KonsoleViewWidget(KonsoleViewPart *part);
  ~KonsoleViewWidget();


public slots:

  void setDirectory(QString dirname);


private slots:

  void documentActivated(KEditor::Document *doc);
  void partDestroyed();

 
protected:
 
  virtual void showEvent(QShowEvent *ev);
 
 
private:

  void activate();

  QString url;
  KParts::ReadOnlyPart *part;
  QVBoxLayout *vbox;

};


#endif
