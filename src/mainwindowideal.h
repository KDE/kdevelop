/***************************************************************************
  mainwindowideal.h  -  KDevelop main widget for IDEAl user interface mode
                             -------------------
    begin                : 22 Dec 2002
    copyright            : (C) 2002 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _MAINWINDOWIDEAL_H_
#define _MAINWINDOWIDEAL_H_


#include "widgets/kdevtabwidget.h"


#include <kparts/mainwindow.h>

#include <qdatetime.h>
#include <qmap.h>
#include <qguardedptr.h>


class KTabZoomWidget;
class MainWindowShare;
#include "kdevmainwindow.h"


class MainWindowIDEAl : public KParts::MainWindow, public KDevMainWindow
{
  Q_OBJECT

public:

  MainWindowIDEAl(QWidget* parent=0, const char *name=0);
  ~MainWindowIDEAl();

  void embedPartView(QWidget *view, const QString &title, const QString &toolTip);
  void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);
  void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);

  void embedSelectViewRight ( QWidget* view, const QString& title, const QString &toolTip);

  void removeView(QWidget *view);
  void setViewAvailable(QWidget *pView, bool bEnabled);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();

  void init();

  void prepareToCloseViews();
  void guiRestoringFinished();

  bool eventFilter( QObject *watched, QEvent *e );
  bool switching(void) const { return m_bSwitching; }
  void setSwitching( const bool switching ) { m_bSwitching = switching; }

protected:
   /**
   * Pointer to the Window menu. SDI specific. MDI uses the feature of qextmdimainfrm.
   */
   QGuardedPtr<QPopupMenu> m_pWindowMenu;

public slots:

  void createGUI(KParts::Part *part);
  void gotoNextWindow();
  void gotoFirstWindow();
  void gotoPreviousWindow();
  void gotoLastWindow();

  void raiseLeftTabbar();
  void raiseRightTabbar();
  void raiseBottomTabbar();
  void raiseEditor();

  /** Store the currently active view tab of the output (bottom) view */
  void storeOutputViewTab();

  /** Restore the previously saved view tab to the output (bottom) view */
  void restoreOutputViewTab();

  /** Just after the project gets opened */
  void projectOpened();

private slots:
  void slotBufferSelected();      // One entry of the Windows menu has been selected
  void slotStatusChange(KParts::Part*);
  void slotTabSelected(QWidget*);
  void slotFillWindowMenu();
  void slotPartAdded(KParts::Part*);
  void slotDocChanged(const KURL& url);
  void slotNewStatus();
  void slotPartJobCompleted();
  void slotBottomTabsChanged();
  void slotRightTabsChanged();
  void slotLeftTabsChanged();
  void fileDirty(const QString& fileName);

private:

  virtual bool queryClose();
  virtual bool queryExit();

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();

  void moveRelativeTab(int);
  void raiseTabbar( KTabZoomWidget *tabBar );
  void updateTabForPart( KParts::ReadWritePart * );

  void clearWindowMenu();

  KAction *m_raiseLeftBar, *m_raiseRightBar, *m_raiseBottomBar, *m_raiseEditor;

  KDevTabWidget *m_tabWidget;
  KTabZoomWidget *m_leftBar, *m_rightBar, *m_bottomBar;

  MainWindowShare*   m_pMainWindowShare;

  QMap<QWidget*, QDateTime> m_timeStamps;
  bool m_bSwitching;

  QValueList<int> m_windowMenus;
  QPtrList<KAction> m_windowDynamicMenus;

  friend class IDEAlEventFilter;
};


#endif
