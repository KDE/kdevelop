#ifndef KDEVPLATFORM_STATUSBARPROGRESSWIDGET_H
#define KDEVPLATFORM_STATUSBARPROGRESSWIDGET_H
/***************************************************************************
 *      (C) 2004 Till Adam <adam@kde.org>                                  *
 *              Don Sanders                                                *
 *              David Faure <dfaure@kde.org>                               *
 *      Copyright 2004 David Faure <faure@kde.org>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

/**
  *  A specialized progress widget class, heavily based on
  *  kio_littleprogress_dlg (it looks similar)
  */

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

class StatusbarProgressWidget : public QFrame {

    Q_OBJECT

public:

    StatusbarProgressWidget( ProgressDialog* progressDialog, QWidget* parent, bool button = true );

public Q_SLOTS:

    void slotClean();

    void slotProgressItemAdded( KDevelop::ProgressItem *i );
    void slotProgressItemCompleted( KDevelop::ProgressItem *i );
    void slotProgressItemProgress( KDevelop::ProgressItem *i, unsigned int value );

protected Q_SLOTS:
    void slotProgressDialogVisible( bool );
    void slotShowItemDelayed();
    void slotBusyIndicator();
    void updateBusyMode();

protected:
    void setMode();
    void connectSingleItem();
    void activateSingleItemMode();

    virtual bool eventFilter( QObject *, QEvent * );

private:
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
    QTimer *mCleanTimer;
};

} // namespace

#endif
