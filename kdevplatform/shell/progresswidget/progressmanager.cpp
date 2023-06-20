/*
    SPDX-FileCopyrightText: 2004 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "progressmanager.h"
#include "debug.h"

#include <KLocalizedString>

namespace KDevelop {

unsigned int KDevelop::ProgressManager::uID = 42;

ProgressItem::ProgressItem( ProgressItem *parent, const QString &id,
                            const QString &label, const QString &status,
                            bool canBeCanceled, bool usesCrypto )
    : mId( id ), mLabel( label ), mStatus( status ), mParent( parent ),
      mCanBeCanceled( canBeCanceled ), mProgress( 0 ), mTotal( 0 ),
      mCompleted( 0 ), mWaitingForKids( false ), mCanceled( false ),
      mUsesCrypto( usesCrypto ), mUsesBusyIndicator( false ), mCompletedCalled( false )
{
}

ProgressItem::~ProgressItem()
{
}

void ProgressItem::setComplete()
{
    //   qCDebug(SHELL) << label();
    if ( mChildren.isEmpty() ) {
        if ( mCompletedCalled )
            return;
        if ( !mCanceled ) {
            setProgress( 100 );
        }
        mCompletedCalled = true;
        if ( parent() ) {
            parent()->removeChild( this );
        }
        emit progressItemCompleted( this );
    } else {
        mWaitingForKids = true;
    }
}

void ProgressItem::addChild( ProgressItem *kiddo )
{
    mChildren.insert( kiddo, true );
}

void ProgressItem::removeChild( ProgressItem *kiddo )
{
    if ( mChildren.isEmpty() ) {
        mWaitingForKids = false;
        return;
    }

    if ( mChildren.remove( kiddo ) == 0 ) {
        // do nothing if the specified item is not in the map
        return;
    }

    // in case we were waiting for the last kid to go away, now is the time
    if ( mChildren.count() == 0 && mWaitingForKids ) {
        emit progressItemCompleted( this );
    }
}

void ProgressItem::cancel()
{
    if ( mCanceled || !mCanBeCanceled ) {
        return;
    }

    qCDebug(SHELL) << label();
    mCanceled = true;
    // Cancel all children.
    const QList<ProgressItem*> kids = mChildren.keys();
    for (ProgressItem* kid : kids) {
        if ( kid->canBeCanceled() ) {
            kid->cancel();
        }
    }
    setStatus( i18nc("@info", "Aborting..." ) );
    emit progressItemCanceled( this );
}

void ProgressItem::setProgress( unsigned int v )
{
    mProgress = v;
    // qCDebug(SHELL) << label() << " :" << v;
    emit progressItemProgress( this, mProgress );
}

void ProgressItem::setLabel( const QString &v )
{
    mLabel = v;
    emit progressItemLabel( this, mLabel );
}

void ProgressItem::setStatus( const QString &v )
{
    mStatus = v;
    emit progressItemStatus( this, mStatus );
}

void ProgressItem::setUsesCrypto( bool v )
{
    mUsesCrypto = v;
    emit progressItemUsesCrypto( this, v );
}

void ProgressItem::setUsesBusyIndicator( bool useBusyIndicator )
{
    mUsesBusyIndicator = useBusyIndicator;
    emit progressItemUsesBusyIndicator( this, useBusyIndicator );
}

// ======================================

struct ProgressManagerPrivate {
    ProgressManager instance;
};

Q_GLOBAL_STATIC( ProgressManagerPrivate, progressManagerPrivate )

ProgressManager::ProgressManager()
    : QObject()
{

}

ProgressManager::~ProgressManager()
{
    for (auto* item : mTransactions)
        delete item;
}

ProgressManager *ProgressManager::instance()
{
    return progressManagerPrivate.isDestroyed() ? nullptr : &progressManagerPrivate->instance ;
}

ProgressItem *ProgressManager::createProgressItemImpl( ProgressItem *parent,
                                                       const QString &id,
                                                       const QString &label,
                                                       const QString &status,
                                                       bool cancellable,
                                                       bool usesCrypto )
{
    ProgressItem *t = nullptr;
    if ( !mTransactions.value( id ) ) {
        t = new ProgressItem ( parent, id, label, status, cancellable, usesCrypto );
        mTransactions.insert( id, t );
        if ( parent ) {
            ProgressItem *p = mTransactions.value( parent->id() );
            if ( p ) {
                p->addChild( t );
            }
        }
        // connect all signals
        connect ( t, &ProgressItem::progressItemCompleted,
                  this, &ProgressManager::slotTransactionCompleted );
        connect ( t, &ProgressItem::progressItemProgress,
                  this, &ProgressManager::progressItemProgress );
        connect ( t, &ProgressItem::progressItemAdded,
                  this, &ProgressManager::progressItemAdded );
        connect ( t, &ProgressItem::progressItemCanceled,
                  this, &ProgressManager::progressItemCanceled );
        connect ( t, &ProgressItem::progressItemStatus,
                  this, &ProgressManager::progressItemStatus );
        connect ( t, &ProgressItem::progressItemLabel,
                  this, &ProgressManager::progressItemLabel );
        connect ( t, &ProgressItem::progressItemUsesCrypto,
                  this, &ProgressManager::progressItemUsesCrypto );
        connect ( t, &ProgressItem::progressItemUsesBusyIndicator,
                  this, &ProgressManager::progressItemUsesBusyIndicator );

        emit progressItemAdded( t );
    } else {
        // Hm, is this what makes the most sense?
        t = mTransactions.value( id );
    }
    return t;
}

ProgressItem *ProgressManager::createProgressItemImpl( const QString &parent,
                                                       const QString &id,
                                                       const QString &label,
                                                       const QString &status,
                                                       bool canBeCanceled,
                                                       bool usesCrypto )
{
    ProgressItem *p = mTransactions.value( parent );
    return createProgressItemImpl( p, id, label, status, canBeCanceled, usesCrypto );
}

void ProgressManager::emitShowProgressDialogImpl()
{
    emit showProgressDialog();
}

// slots

void ProgressManager::slotTransactionCompleted( ProgressItem *item )
{
    mTransactions.remove( item->id() );
    emit progressItemCompleted( item );
}

void ProgressManager::slotStandardCancelHandler( ProgressItem *item )
{
    item->setComplete();
}

ProgressItem *ProgressManager::singleItem() const
{
    ProgressItem *item = nullptr;
    for (ProgressItem* transactionItem : mTransactions) {
        // No single item for progress possible, as one of them is a busy indicator one.
        if (transactionItem->usesBusyIndicator())
            return nullptr;

        if (!transactionItem->parent()) {             // if it's a top level one, only those count
            if ( item ) {
                return nullptr; // we found more than one
            } else {
                item = transactionItem;
            }
        }
    }
    return item;
}

void ProgressManager::slotAbortAll()
{
    QHashIterator<QString, ProgressItem *> it(mTransactions);
    while (it.hasNext()) {
        it.next();
        it.value()->cancel();
    }

}

} // namespace

#include "moc_progressmanager.cpp"
