#ifndef __KONSOLEVIEWWIDGET_H__
#define __KONSOLEVIEWWIDGET_H__


#include <qwidget.h>


class KDevProject;
class KonsoleWidgetPrivate;
class KonsoleViewPart;


class KonsoleViewWidget : public QWidget
{
  Q_OBJECT
    
public:
		  
  KonsoleViewWidget(KonsoleViewPart *part);
  ~KonsoleViewWidget();


public slots:

  void setDirectory(QString dirname);


private slots:

  void wentToSourceFile(const QString &fileName);

 
protected:
 
  virtual void showEvent(QShowEvent *ev);
 
 
private:
 
  KonsoleWidgetPrivate *d;

};


#endif
