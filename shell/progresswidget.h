/*
  (C) 2004 Till Adam <adam@kde.org>
           Don Sanders
           David Faure <dfaure@kde.org>
  Copyright 2004 David Faure <faure@kde.org>
  Copyright (c) 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
      copied from pimlibs

  This is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2 
  or above, as published by the Free Software Foundation.

  This is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KDEVPLATFORM_PROGRESSWIDGET_H
#define KDEVPLATFORM_PROGRESSWIDGET_H

#include <QFrame>

class QBoxLayout;
class QEvent;
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QBoxLayout;
class QLabel;
class QTimer;

namespace KDevelop {
class ProgressItem;
class ProgressDialog;
class ProgressManager;

/**
  *  A specialized progress widget class, heavily based on
  *  kio_littleprogress_dlg
  *  Copied & modified kdepim/pimlibs/statusprogresswidget.h
  *
  *  This widget shows progress information for multiple items.
  *  By default, a single progressbar aggregates status information
  *  for all items. The widget contains a pushbutton that spawns
  *  an individual progressbar for all items. These progressbars are
  *  stacked vertically and shown in an overlay widget on top of the 
  *  applications mainwindow. This overlay widget is implemented in 
  *  KDevelop::ProgressDialog.
  */
class  StatusbarProgressWidget : public QFrame
{
  Q_OBJECT

public:

  StatusbarProgressWidget(ProgressManager* progressManager, ProgressDialog* progressDialog, QWidget* parent, bool button = true );
  virtual ~StatusbarProgressWidget();

public Q_SLOTS:

  void slotClean();

  void slotProgressItemAdded( KDevelop::ProgressItem *i );
  void slotProgressItemCompleted( KDevelop::ProgressItem *i );
  void slotProgressItemProgress( KDevelop::ProgressItem *i, unsigned int value );

protected Q_SLOTS:
  void slotProgressDialogVisible( bool );
  void slotShowItemDelayed();
  void slotBusyIndicator();

protected:
  void setMode();
  void connectSingleItem();
  void activateSingleItemMode();

  virtual bool eventFilter( QObject *, QEvent * );

private:
  ProgressManager* m_progressManager;
  QProgressBar* m_pProgressBar;
  QLabel* m_pLabel;
  QPushButton* m_pButton;

  enum Mode { None, /*Label,*/ Progress };

  uint mode;
  bool m_bShowButton;

  QBoxLayout *box;
  QStackedWidget *stack;
  ProgressItem *mCurrentItem;
  ProgressDialog* mProgressDialog;
  QTimer *mDelayTimer;
  QTimer *mBusyTimer;
};

} // namespace

#endif
