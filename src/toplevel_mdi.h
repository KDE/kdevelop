#ifndef _TOPLEVEL_MDI_H_
#define _TOPLEVEL_MDI_H_


#include <qmap.h>
#include <qptrlist.h>


#include <qextmdimainfrm.h>
#include <qextmdichildview.h>


#include "kdevtoplevel.h"


class TopLevelMDI : public QextMdiMainFrm, public KDevTopLevel
{
  Q_OBJECT

public:

  TopLevelMDI(QWidget* parent=0, const char *name=0);
  ~TopLevelMDI();
    
  void embedPartView(QWidget *view, const QString &title);
  void embedSelectView(QWidget *view, const QString &title);
  void embedOutputView(QWidget *view, const QString &title);
  
  void embedSelectViewRight ( QWidget* view, const QString& title );

  void removeView(QWidget *view);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();

  void init();

  void realClose();

  /** Reimplemented from QextMdiChildView to handle save prompt */
  virtual void childWindowCloseRequest(QextMdiChildView *pWnd);

signals:

  void wantsToQuit();


public slots:

  void createGUI(KParts::Part *part);
  void gotoNextWindow();
  void gotoPreviousWindow();


protected:

  void resizeEvent(QResizeEvent *ev);


private slots:

  void slotQuit();
  void slotKeyBindings();
  void slotSettings();
  void slotActiveProcessCountChanged( uint active );
  
private:
  
  virtual bool queryClose();
  
  QextMdiChildView *wrapper(QWidget *view, const QString &name);

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();
  void saveMDISettings();
  void loadMDISettings();
  
  KAction* m_stopProcesses;
  
  QMap<QWidget*,QextMdiChildView*> m_widgetMap;
  QMap<QextMdiChildView*,QWidget*> m_childViewMap;

  QPtrList<QextMdiChildView> m_outputViews, m_selectViews, m_partViews;

  bool m_closing;

};


#endif
