/*
  This file is part of KDevelop

  Copyright (c) 2004 Till Adam <adam@kde.org>
  Copyright (c) 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
       copied from pimlibs

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

namespace KDevelop
{

unsigned int KDevelop::ProgressManager::uID = 42;

ProgressItem::ProgressItem( ProgressItem *parent, const QString &id,
                            const QString &label, const QString &status,
                            bool canBeCanceled)
  :mId( id ), mLabel( label ), mStatus( status ), mParent( parent ),
   mCanBeCanceled( canBeCanceled ), mProgress( 0 ), mTotal( 0 ),
   mCompleted( 0 ), mWaitingForKids( false ), mCanceled( false ), mBusy( false )
{
}

ProgressItem::~ProgressItem()
{
}

void ProgressItem::setComplete()
{
  if ( mChildren.isEmpty() ) {
    if ( !mCanceled ) {
      setProgress( 100 );
    }
    emit progressItemCompleted( this );
    if ( parent() ) {
      parent()->removeChild( this );
    }
    deleteLater();
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
  mChildren.remove( kiddo );
  // in case we were waiting for the last kid to go away, now is the time
  if ( mChildren.count() == 0 && mWaitingForKids ) {
    emit progressItemCompleted( this );
    deleteLater();
  }
}

void ProgressItem::cancel()
{
  if ( mCanceled || !mCanBeCanceled ) {
    return;
  }

  mCanceled = true;
  // Cancel all children.
  QList<ProgressItem*> kids = mChildren.keys();
  QList<ProgressItem*>::Iterator it( kids.begin() );
  QList<ProgressItem*>::Iterator end( kids.end() );
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


void ProgressItem::setBusy(bool busy)
{
    mBusy = busy;
    if( mBusy ) {
        setProgress( 0 );
    }
    emit progressItemProgress( this, progress() );
}

// ======================================

ProgressManager::ProgressManager(QObject *parent)
  : QObject(parent)
{}

ProgressManager::~ProgressManager() {} 

ProgressItem *ProgressManager::createProgressItemImpl( ProgressItem *parent,
                                                       const QString &id,
                                                       const QString &label,
                                                       const QString &status,
                                                       bool cancellable)
{
  ProgressItem *t = 0;
  if ( !mTransactions[ id ] ) {
    t = new ProgressItem ( parent, id, label, status, cancellable);
    mTransactions.insert( id, t );
    if ( parent ) {
      ProgressItem *p = mTransactions[ parent->id() ];
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

    emit progressItemAdded( t );
  } else {
    // Hm, is this what makes the most sense?
    t = mTransactions[id];
  }
  return t;
}

ProgressItem *ProgressManager::createProgressItemImpl( const QString &parent,
                                                       const QString &id,
                                                       const QString &label,
                                                       const QString &status,
                                                       bool canBeCanceled)
{
  ProgressItem *p = mTransactions[parent];
  return createProgressItemImpl( p, id, label, status, canBeCanceled);
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
  QHash< QString, ProgressItem* >::const_iterator it, end = mTransactions.constEnd();
  for ( it = mTransactions.constBegin(); it != end; ++it ) {
    if ( !(it.value())->parent() ) { // if it's a top level one, only those count
      if ( item ) {
        return 0; // we found more than one
      } else {
        item = (it.value());
      }
    }
  }
  return item;
}

void ProgressManager::slotAbortAll()
{
  QHash< QString, ProgressItem* >::iterator it, end = mTransactions.end();
  for ( it = mTransactions.begin(); it != end; ++it ) {
    it.value()->cancel();
  }
}

} // namespace KDevelop

#include "progressmanager.moc"
