/* This file is part of the KDE project
   Copyright (C) 2003 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
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
  virtual void guiRestoringFinished() = 0;

  virtual void setUserInterfaceMode(const QString& /*uiMode*/) {}

  /** this allows to order the mainwindow to do anything special, we use it to trigger the execution of a hack */
  virtual void callCommand(const QString& /*command*/) {}

  protected:
  /** Stores the saved view tab/window of the output view/window */
  QWidget *previous_output_view;
};


#endif
