#ifndef _KDEV_TOPLEVEL_H_
#define _KDEV_TOPLEVEL_H_


class KStatusBar;
class KMainWindow;

class KDevTopLevel
{
public:

  virtual void embedPartView(QWidget *view, const QString &title) = 0;
  virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip) = 0;
  virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip) = 0;
  
  virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip) = 0;

  virtual void removeView(QWidget *view) = 0;
  virtual void setViewVisible(QWidget *pView, bool bEnabled) = 0;
  
  virtual void raiseView(QWidget *view) = 0;
  virtual void lowerView(QWidget *view) = 0;
  virtual void lowerAllViews() = 0;

  virtual void loadSettings() = 0;

  virtual KMainWindow *main() = 0;

  KStatusBar *statusBar();

  virtual void realClose();

};


#endif
