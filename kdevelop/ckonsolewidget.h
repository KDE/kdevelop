#ifndef CKONSOLEWIDGET_H
#define CKONSOLEWIDGET_H


#include <qwidget.h>
#include <qstring.h>


class CKonsoleWidgetPrivate;


class CKonsoleWidget : public QWidget
{
  Q_OBJECT

public:
	  
  CKonsoleWidget(QWidget* parent, const char* name=0);
  ~CKonsoleWidget();


public slots:

  void setDirectory(QString dirname);


protected:

  virtual void showEvent(QShowEvent *ev);


private:

  CKonsoleWidgetPrivate *d;
  
};


#endif
