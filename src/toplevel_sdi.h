#ifndef _TOPLEVEL_SDI_H_
#define _TOPLEVEL_SDI_H_


#include <qtabwidget.h>


#include <kparts/mainwindow.h>


class KTabZoomWidget;


#include "kdevtoplevel.h"


class TopLevelSDI : public KParts::MainWindow, public KDevTopLevel
{
  Q_OBJECT

public:

  TopLevelSDI(QWidget* parent=0, const char *name=0);
  ~TopLevelSDI();
    
  void embedPartView(QWidget *view, const QString &title);
  void embedSelectView(QWidget *view, const QString &title);
  void embedOutputView(QWidget *view, const QString &title);

  void removeView(QWidget *view);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();

  void init();

  void realClose();


signals:

  void wantsToQuit();


public slots:

  void createGUI(KParts::Part *part);


private slots:

  void slotQuit();
  void slotSettings();


private:

  virtual bool queryClose();

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();

  QTabWidget *m_tabWidget;
  KTabZoomWidget *m_leftBar, *m_bottomBar;

  bool m_closing;

};


#endif
