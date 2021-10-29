/*
    SPDX-FileCopyrightText: 2004 Till Adam <adam@kde.org>
    SPDX-FileCopyrightText: 2004 Don Sanders
    SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STATUSBARPROGRESSWIDGET_H
#define KDEVPLATFORM_STATUSBARPROGRESSWIDGET_H

#include <QFrame>

class QProgressBar;
class QToolButton;
class QStackedWidget;
class QBoxLayout;
class QLabel;
class QTimer;

namespace KDevelop {
class ProgressItem;
class ProgressDialog;

/**
  *  A specialized progress widget class, heavily based on
  *  kio_littleprogress_dlg (it looks similar)
  */
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
    void updateBusyMode();

protected:
    void setMode();
    void connectSingleItem();
    void activateSingleItemMode();
    void activateSingleItemMode( unsigned int progress );

    bool eventFilter( QObject *, QEvent * ) override;

private:
    QProgressBar* m_pProgressBar;
    union {
        QLabel* label;
        QToolButton* button;
    } m_pPlaceHolder;
    QToolButton* m_pButton;

    enum Mode { None, /*Label,*/ Progress };

    uint mode;
    bool m_bShowButton;

    QBoxLayout *box;
    QStackedWidget *stack;
    ProgressItem *mCurrentItem;
    ProgressDialog* mProgressDialog;
    QTimer *mDelayTimer;
    QTimer *mCleanTimer;
};

} // namespace

#endif
