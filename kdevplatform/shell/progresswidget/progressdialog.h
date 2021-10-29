/*
    SPDX-FileCopyrightText: 2004 Till Adam <adam@kde.org>
    based on imapprogressdialog.cpp ,which is
    SPDX-FileCopyrightText: 2002-2003 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROGRESSDIALOG_H
#define KDEVPLATFORM_PROGRESSDIALOG_H

#include "overlaywidget.h"

#include <QMap>
#include <QScrollArea>
#include <QWidget>

class QProgressBar;
class QFrame;
class QLabel;
class QPushButton;

namespace KDevelop {
class ProgressItem;
class TransactionItem;
class SSLLabel;

class TransactionItemView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TransactionItemView( QWidget * parent = nullptr, const char * name = nullptr );

    ~TransactionItemView() override {}
    TransactionItem *addTransactionItem( ProgressItem *item, bool first );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void slotItemCompleted(TransactionItem * item);

protected:
    void resizeEvent ( QResizeEvent *event ) override;

private:
    QWidget *mBigBox;
};

class TransactionItem : public QWidget
{
    Q_OBJECT
public:
    TransactionItem( QWidget *parent, ProgressItem *item, bool first );

    ~TransactionItem() override;

    void hideHLine();

    void setProgress( int progress );
    void setLabel( const QString & );

    // the given text is interpreted as RichText, so you might need to
    // Qt::escape() it before passing
    void setStatus( const QString & );

    void setTotalSteps( int totalSteps );

    ProgressItem *item() const { return mItem; }

    void addSubTransaction( ProgressItem *item );

    // The progressitem is deleted immediately, we take 5s to go out,
    // so better not use mItem during this time.
    void setItemComplete() { mItem = nullptr; }

public Q_SLOTS:
    void slotItemCanceled();

protected:
    QProgressBar *mProgress;
    QPushButton  *mCancelButton;
    QLabel       *mItemLabel;
    QLabel       *mItemStatus;
    QFrame       *mFrame;
    ProgressItem *mItem;
};

class ProgressDialog : public OverlayWidget
{
    Q_OBJECT
public:
    ProgressDialog( QWidget *alignWidget, QWidget *parent, const char *name = nullptr );
    ~ProgressDialog() override;
    void setVisible( bool b ) override;

public Q_SLOTS:
    void slotToggleVisibility();

protected Q_SLOTS:
    void slotTransactionAdded( KDevelop::ProgressItem *item );
    void slotTransactionCompleted( KDevelop::ProgressItem *item );
    void slotTransactionCanceled( KDevelop::ProgressItem *item );
    void slotTransactionProgress( KDevelop::ProgressItem *item, unsigned int progress );
    void slotTransactionStatus( KDevelop::ProgressItem *item, const QString & );
    void slotTransactionLabel( KDevelop::ProgressItem *item, const QString & );
    void slotTransactionUsesBusyIndicator( KDevelop::ProgressItem *, bool );

    void slotClose();
    void slotShow();
    void slotHide();

Q_SIGNALS:
    void visibilityChanged( bool );

protected:
    void closeEvent( QCloseEvent * ) override;

    TransactionItemView *mScrollView;
    QMap<const ProgressItem *, TransactionItem *> mTransactionsToListviewItems;
    bool mWasLastShown;
};

} // namespace KDevelop

#endif // __KDevelop_PROGRESSDIALOG_H__
