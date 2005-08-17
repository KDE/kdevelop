/***************************************************************************
          mainwindowshare.h  -  shared stuff of the main widgets
                             -------------------
    begin                : 19 Dec 2002
    copyright            : (C) 2002 by Falk Brettschneider
    email                : falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __MAINWINDOWSHARE_H__
#define __MAINWINDOWSHARE_H__

#include <qobject.h>
#include <qlist.h>
#include <qmenu.h>

class KAction;
class KToggleAction;
namespace KParts {
  class MainWindow;
}
/**
Shared main window functionality.
*/
class MainWindowShare : public QObject
{
  Q_OBJECT
public:
  MainWindowShare(QObject* pParent = 0);

  void createActions();
  void init();

public slots:
  void slotGUICreated( KParts::Part * );

signals:
  void gotoNextWindow();
  void gotoPreviousWindow();
  void gotoFirstWindow();
  void gotoLastWindow();

private slots:
  void slotReportBug();
  void slotKeyBindings();
  void slotConfigureToolbars();
  void slotConfigureNotifications();
  void slotConfigureEditors();
  void slotSettings();
  void slotActiveProcessChanged( KDevPlugin*, bool );
  void slotActivePartChanged( KParts::Part* part );
  void slotStopPopupActivated(int);
  void slotStopMenuAboutToShow();
  void slotStopButtonPressed();
  void slotNewToolbarConfig();
  void slotShowMenuBar();
  void slotActionStatusText( const QString &text );

  void slotToggleMainToolbar();
  void slotToggleBuildToolbar();
  void slotToggleViewToolbar();
  void slotToggleBrowserToolbar();
  void slotToggleStatusbar();

  void contextMenu(QMenu*, const Context *);

private:
  KToggleAction*   m_toggleMainToolbar;
  KToggleAction*   m_toggleBuildToolbar;
  KToggleAction*   m_toggleViewToolbar;
  KToggleAction*   m_toggleBrowserToolbar;
  KToggleAction*   m_toggleStatusbar;

  KAction * m_configureEditorAction;

  KToolBarPopupAction*  m_stopProcesses;                 //!< Stops all running processes

  KParts::MainWindow*   m_pMainWnd;
  QList<KDevPlugin *>  activeProcesses;
};

#endif // __MAINWINDOWSHARE_H__
