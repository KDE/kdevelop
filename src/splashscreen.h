#ifndef __SPLASHSCREEN_H__
#define __SPLASHSCREEN_H__


#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>


class SplashScreen : public QObject
{
  Q_OBJECT

public:

  SplashScreen();
  ~SplashScreen();

  virtual bool eventFilter(QObject* obj, QEvent* e);

public slots:

  void showMessage(const QString &message);

	
private:
	
  QWidget *m_splash;
  QLabel  *m_message;

};


#endif
