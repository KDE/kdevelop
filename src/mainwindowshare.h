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

class KAction;
class KToggleAction;
namespace KParts {
  class MainWindow;
}

class MainWindowShare : public QObject
{
  Q_OBJECT
public:
  MainWindowShare(QObject* pParent = 0L, const char* name = 0L);
  ~MainWindowShare() {}

  void createActions();

private slots:
  void slotKeyBindings();
  void slotConfigureToolbars();
  void slotConfigureNotifications();
  void slotSettings();
  void slotActiveProcessCountChanged( uint active );
  void slotNewToolbarConfig();
  void slotShowMenuBar();

  void slotToggleMainToolbar();
  void slotToggleBuildToolbar();
  void slotToggleViewToolbar();
  void slotToggleBrowserToolbar();
  void slotToggleStatusbar();

private:
  KToggleAction*   m_toggleMainToolbar;
  KToggleAction*   m_toggleBuildToolbar;
  KToggleAction*   m_toggleViewToolbar;
  KToggleAction*   m_toggleBrowserToolbar;
  KToggleAction*   m_toggleStatusbar;

  KAction*         m_stopProcesses;                 //!< Stops all running processes

  KParts::MainWindow*   m_pMainWnd;
};

#endif // __MAINWINDOWSHARE_H__
