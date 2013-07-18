/***************************************************************************
 *        Copyright (c) 2004 Till Adam <adam@kde.org>                      *
 *          based on imapprogressdialog.cpp ,which is                      *
 *        Copyright (c) 2002-2003 Klar�vdalens Datakonsult AB              *
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

#ifndef KDEVPLATFORM_PROGRESSDIALOG_H
#define KDEVPLATFORM_PROGRESSDIALOG_H

#include "overlaywidget.h"


#include <QScrollArea>
#include <QMap>
#include <KVBox>

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
    explicit TransactionItemView( QWidget * parent = 0, const char * name = 0 );

    virtual ~TransactionItemView() {}
    TransactionItem *addTransactionItem( ProgressItem *item, bool first );

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public Q_SLOTS:
    void slotLayoutFirstItem();

protected:
    virtual void resizeEvent ( QResizeEvent *event );

private:
    KVBox *mBigBox;
};

class TransactionItem : public KVBox
{
    Q_OBJECT
public:
    TransactionItem( QWidget *parent, ProgressItem *item, bool first );

    ~TransactionItem();

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
    void setItemComplete() { mItem = 0; }

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
    ProgressDialog( QWidget *alignWidget, QWidget *parent, const char *name = 0 );
    ~ProgressDialog();
    void setVisible( bool b );

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
    virtual void closeEvent( QCloseEvent * );

    TransactionItemView *mScrollView;
    QMap<const ProgressItem *, TransactionItem *> mTransactionsToListviewItems;
    bool mWasLastShown;
};

} // namespace KDevelop

#endif // __KDevelop_PROGRESSDIALOG_H__
