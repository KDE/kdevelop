#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_


#include <kparts/mainwindow.h>


class KTabZoomWidget;


class TopLevel : public KParts::MainWindow
{
  Q_OBJECT

public:

  void embedSelectView(QWidget *view, const QString &title);
  void embedOutputView(QWidget *view, const QString &title);

  void removeView(QWidget *view);

  void raiseView(QWidget *view);

  static void createInstance(QWidget *parent=0, const char *name=0);
  static TopLevel *getInstance();

  void createGUI(KParts::Part *part);

  void loadSettings();


protected:

  TopLevel(QWidget* parent=0, const char *name=0);
  ~TopLevel();


private slots:

  void slotQuit();
  void slotOpenProject();
  void slotOpenRecentProject(const KURL &url);
  void slotCloseProject();
  void slotProjectOptions();
  void slotSettings();


private:

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();

  KTabZoomWidget *m_leftBar, *m_bottomBar;

  static TopLevel *s_instance;

  
  KAction *m_closeProjectAction, *m_projectOptionsAction;
  KRecentFilesAction *m_openRecentProjectAction;

};


#endif
