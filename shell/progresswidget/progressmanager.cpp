/*
  progressmanager.cpp

  This file is part of libkdepim.

  Copyright (c) 2004 Till Adam <adam@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "progressmanager.h"

#include <KDebug>
#include <KLocale>
#include <KGlobal>

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
    //   kDebug() << label();
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

    kDebug() << label();
    mCanceled = true;
    // Cancel all children.
    QList<ProgressItem* > kids = mChildren.keys();
    QList<ProgressItem* >::Iterator it( kids.begin() );
    QList<ProgressItem* >::Iterator end( kids.end() );
    for ( ; it != end; it++ ) {
        ProgressItem *kid = *it;
        if ( kid->canBeCanceled() ) {
            kid->cancel();
        }
    }
    setStatus( i18n( "Aborting..." ) );
    emit progressItemCanceled( this );
}

void ProgressItem::setProgress( unsigned int v )
{
    mProgress = v;
    // kDebug() << label() << " :" << v;
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

K_GLOBAL_STATIC( ProgressManagerPrivate, progressManagerPrivate )

ProgressManager::ProgressManager()
    : QObject()
{

}

ProgressManager::~ProgressManager() {}

ProgressManager *ProgressManager::instance()
{
    return progressManagerPrivate.isDestroyed() ? 0 : &progressManagerPrivate->instance ;
}

ProgressItem *ProgressManager::createProgressItemImpl( ProgressItem *parent,
                                                       const QString &id,
                                                       const QString &label,
                                                       const QString &status,
                                                       bool cancellable,
                                                       bool usesCrypto )
{
    ProgressItem *t = 0;
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
        connect ( t, SIGNAL(progressItemCompleted(KDevelop::ProgressItem*)),
                  this, SLOT(slotTransactionCompleted(KDevelop::ProgressItem*)) );
        connect ( t, SIGNAL(progressItemProgress(KDevelop::ProgressItem*,uint)),
                  this, SIGNAL(progressItemProgress(KDevelop::ProgressItem*,uint)) );
        connect ( t, SIGNAL(progressItemAdded(KDevelop::ProgressItem*)),
                  this, SIGNAL(progressItemAdded(KDevelop::ProgressItem*)) );
        connect ( t, SIGNAL(progressItemCanceled(KDevelop::ProgressItem*)),
                  this, SIGNAL(progressItemCanceled(KDevelop::ProgressItem*)) );
        connect ( t, SIGNAL(progressItemStatus(KDevelop::ProgressItem*,QString)),
                  this, SIGNAL(progressItemStatus(KDevelop::ProgressItem*,QString)) );
        connect ( t, SIGNAL(progressItemLabel(KDevelop::ProgressItem*,QString)),
                  this, SIGNAL(progressItemLabel(KDevelop::ProgressItem*,QString)) );
        connect ( t, SIGNAL(progressItemUsesCrypto(KDevelop::ProgressItem*,bool)),
                  this, SIGNAL(progressItemUsesCrypto(KDevelop::ProgressItem*,bool)) );
        connect ( t, SIGNAL(progressItemUsesBusyIndicator(KDevelop::ProgressItem*,bool)),
                  this, SIGNAL(progressItemUsesBusyIndicator(KDevelop::ProgressItem*,bool)) );

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
    ProgressItem *item = 0;
    QHash< QString, ProgressItem* >::const_iterator it = mTransactions.constBegin();
    QHash< QString, ProgressItem* >::const_iterator end = mTransactions.constEnd();
    while ( it != end ) {

        // No single item for progress possible, as one of them is a busy indicator one.
        if ( (*it)->usesBusyIndicator() )
            return 0;

        if ( !(*it)->parent() ) {             // if it's a top level one, only those count
            if ( item ) {
                return 0; // we found more than one
            } else {
                item = (*it);
            }
        }
        ++it;
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

#include "progressmanager.moc"
