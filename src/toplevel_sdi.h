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
  
  void embedSelectViewRight ( QWidget* view, const QString& title );

  void removeView(QWidget *view);
  void setViewVisible(QWidget *pView, bool bEnabled);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();

  void init();

  void realClose();

signals:

  void wantsToQuit();

protected:
   /**
   * Pointer to the Window menu. SDI specific. MDI uses the feature of qextmdimainfrm.
   */
   QPopupMenu              *m_pWindowMenu;

public slots:

  void createGUI(KParts::Part *part);
  void gotoNextWindow();
  void gotoPreviousWindow();


private slots:

  void slotQuit();
  void slotKeyBindings();
  void slotConfigureToolbars();
  void slotConfigureNotifications();
  void slotSettings();
  void slotActiveProcessCountChanged( uint active );
  void slotNewToolbarConfig();
  void slotShowMenuBar();

  void slotBufferSelected();      // One entry of the Windows menu has been selected
  void slotFillWindowMenu();
  void slotPartAdded(KParts::Part*);
  void slotTextChanged();
  void slotUpdateModifiedFlags();

private:

  virtual bool queryClose();

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();

  void moveRelativeTab(unsigned int);

  KAction* m_stopProcesses;

  QTabWidget *m_tabWidget;
  KTabZoomWidget *m_leftBar, *m_rightBar, *m_bottomBar;

  bool m_closing;

};


#endif
