/***************************************************************************
  kdevmainwindow.h - Interface to the main window of KDevelop
			     -------------------
    begin                : ?
    copyright            : (C) 2003 by the KDevelop team
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
#ifndef _KDEV_TOPLEVEL_H_
#define _KDEV_TOPLEVEL_H_


class KStatusBar;
class KMainWindow;

class KDevMainWindow
{
public:

  virtual void embedPartView(QWidget *view, const QString &title, const QString& toolTip = QString::null) = 0;
  virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip) = 0;
  virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip) = 0;
  
  virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip) = 0;

  virtual void removeView(QWidget *view) = 0;
  virtual void setViewAvailable(QWidget *pView, bool bEnabled) = 0;
  
  virtual void raiseView(QWidget *view) = 0;
  virtual void lowerView(QWidget *view) = 0;
  virtual void lowerAllViews() = 0;

  /** Store the currently active view tab/window of the output view/window */
  virtual void storeOutputViewTab() = 0;

  /** Restore the previously saved view tab/window to the output view/window */
  virtual void restoreOutputViewTab() = 0;

  virtual void loadSettings() = 0;

  virtual KMainWindow *main() = 0;

  KStatusBar *statusBar();

  virtual void prepareToCloseViews() = 0;
  virtual void realClose();

  virtual void setUserInterfaceMode(const QString& /*uiMode*/) {}
  
  /** this allows to order the mainwindow to do anything special, we use it to trigger the execution of a hack */
  virtual void callCommand(const QString& /*command*/) {}

  protected:
  /** Stores the saved view tab/window of the output view/window */
  QWidget *previous_output_view;
};


#endif
