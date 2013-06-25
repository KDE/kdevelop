/*
  (C) 2004 Till Adam <adam@kde.org>
           Don Sanders
           David Faure <dfaure@kde.org>
   
  Copyright 2004 David Faure <faure@kde.org>
  Includes StatusbarProgressWidget which is based on KIOLittleProgressDlg
  by Matt Koss <koss@miesto.sk>

  Copyright (c) 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
       copied from pimlibs

  This is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2 or above,
  as published by the Free Software Foundation.

  Thi is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "progresswidget.h"
#include "progressdialog.h"
#include "progressmanager.h"

#include <KLocale>
#include <KIconLoader>
#include <KDebug>

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>

using namespace KDevelop;

//-----------------------------------------------------------------------------
StatusbarProgressWidget::StatusbarProgressWidget( ProgressManager* progressManager, ProgressDialog* progressDialog, QWidget* parent, bool button )
  : QFrame( parent ), m_progressManager(progressManager), mCurrentItem( 0 ), mProgressDialog( progressDialog ),
    mDelayTimer( 0 ), mBusyTimer( 0 )
{
  m_bShowButton = button;
  int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
  box = new QHBoxLayout( this );
  box->setMargin(0);
  box->setSpacing(0);

  m_pButton = new QPushButton( this );
  m_pButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                         QSizePolicy::Minimum ) );
  QPixmap smallIcon = SmallIcon( "go-up" );
  m_pButton->setIcon( smallIcon );
  box->addWidget( m_pButton  );
  stack = new QStackedWidget( this );
  int maximumHeight = qMax( smallIcon.height(), fontMetrics().height() );
  stack->setMaximumHeight( maximumHeight );
  box->addWidget( stack );

  m_pButton->setToolTip( i18n("Open detailed progress dialog") );

  m_pProgressBar = new QProgressBar( this );
  m_pProgressBar->installEventFilter( this );
  m_pProgressBar->setMinimumWidth( w );
  stack->insertWidget( 1,m_pProgressBar );

  m_pLabel = new QLabel( QString(), this );
  m_pLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  m_pLabel->installEventFilter( this );
  m_pLabel->setMinimumWidth( w );
  stack->insertWidget( 2, m_pLabel );
  m_pButton->setMaximumHeight( maximumHeight );

  mode = None;
  setMode();

  connect( m_pButton, SIGNAL(clicked()),
           progressDialog, SLOT(slotToggleVisibility()) );

  connect ( m_progressManager, SIGNAL(progressItemAdded(KDevelop::ProgressItem*)),
            this, SLOT(slotProgressItemAdded(KDevelop::ProgressItem*)) );
  connect ( m_progressManager, SIGNAL(progressItemCompleted(KDevelop::ProgressItem*)),
            this, SLOT(slotProgressItemCompleted(KDevelop::ProgressItem*)) );

  connect ( progressDialog, SIGNAL(visibilityChanged(bool)),
            this, SLOT(slotProgressDialogVisible(bool)) );

  mDelayTimer = new QTimer( this );
  connect ( mDelayTimer, SIGNAL(timeout()),
            this, SLOT(slotShowItemDelayed()) );
}

StatusbarProgressWidget::~StatusbarProgressWidget()
{
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::slotProgressItemAdded( ProgressItem *item )
{
  if ( item->parent() ) return; // we are only interested in top level items
  connectSingleItem(); // if going to 1 item
  if ( mCurrentItem ) { // Exactly one item
    delete mBusyTimer;
    mBusyTimer = 0;
    mDelayTimer->setSingleShot( true );
    mDelayTimer->start( 500 );
  }
  else { // N items
    if ( !mBusyTimer ) {
      mBusyTimer = new QTimer( this );
      connect( mBusyTimer, SIGNAL(timeout()),
               this, SLOT(slotBusyIndicator()) );
      mDelayTimer->setSingleShot( true );
      mDelayTimer->start( 500 );
    }
  }
}

void StatusbarProgressWidget::slotProgressItemCompleted( ProgressItem *item )
{
  if ( item->parent() ) return; // we are only interested in top level items
  connectSingleItem(); // if going back to 1 item
  if ( m_progressManager->isEmpty() ) { // No item
    // Done. In 5s the progress-widget will close, then we can clean up the statusbar
    QTimer::singleShot( 2000, this, SLOT(slotClean()) );
  } else if ( mCurrentItem ) { // Exactly one item
    delete mBusyTimer;
    mBusyTimer = 0;
    activateSingleItemMode();
  }
}

void StatusbarProgressWidget::connectSingleItem()
{
  if ( mCurrentItem ) {
    disconnect ( mCurrentItem, SIGNAL(progressItemProgress(KDevelop::ProgressItem*,uint)),
                 this, SLOT(slotProgressItemProgress(KDevelop::ProgressItem*,uint)) );
    mCurrentItem = 0;
  }
  mCurrentItem = m_progressManager->singleItem();
  if ( mCurrentItem ) {
    connect ( mCurrentItem, SIGNAL(progressItemProgress(KDevelop::ProgressItem*,uint)),
              this, SLOT(slotProgressItemProgress(KDevelop::ProgressItem*,uint)) );
  }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
  m_pProgressBar->show();
  if( mCurrentItem->busy() ) 
  {
      m_pProgressBar->setMaximum( 0 );
  } else 
  {
    m_pProgressBar->setMaximum( 100 );
    m_pProgressBar->setValue( mCurrentItem->progress() );
  }
  m_pProgressBar->setTextVisible( true );
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
  bool noItems = m_progressManager->isEmpty();
  if ( mCurrentItem ) {
    activateSingleItemMode();
  } else if ( !noItems ) { // N items
    m_pProgressBar->setMaximum( 0 );
    m_pProgressBar->setTextVisible( false );
    Q_ASSERT( mBusyTimer );
    if ( mBusyTimer )
      mBusyTimer->start( 100 );
  }

  if ( !noItems && mode == None ) {
    mode = Progress;
    setMode();
  }
}

void StatusbarProgressWidget::slotBusyIndicator()
{
  int p = m_pProgressBar->value();
  m_pProgressBar->setValue( p + 10 );
}

void StatusbarProgressWidget::slotProgressItemProgress( ProgressItem *item, unsigned int value )
{
  Q_ASSERT( item == mCurrentItem ); // the only one we should be connected to
  if( item->busy() ) {
    m_pProgressBar->setMaximum( 0 );
  } else {
    m_pProgressBar->setMaximum( 100 );
    m_pProgressBar->setValue( value );
  }
}

void StatusbarProgressWidget::setMode() {
  switch ( mode ) {
  case None:
    if ( m_bShowButton ) {
      m_pButton->hide();
    }
    hide();
    // show the empty label in order to make the status bar look better
    //stack->show();
    //stack->setCurrentWidget( m_pLabel );
    break;

  case Progress:
    show();
    stack->show();
    stack->setCurrentWidget( m_pProgressBar );
    if ( m_bShowButton ) {
      m_pButton->show();
    }

    break;
  }
}

void StatusbarProgressWidget::slotClean()
{
  // check if a new item showed up since we started the timer. If not, clear
  if ( m_progressManager->isEmpty() ) {
    m_pProgressBar->setValue( 0 );
    //m_pLabel->clear();
    mode = None;
    setMode();
  }
}

bool StatusbarProgressWidget::eventFilter( QObject *, QEvent *ev )
{
  if ( ev->type() == QEvent::MouseButtonPress ) {
    QMouseEvent *e = (QMouseEvent*)ev;

    if ( e->button() == Qt::LeftButton && mode != None ) {    // toggle view on left mouse button
      // Consensus seems to be that we should show/hide the fancy dialog when the user
      // clicks anywhere in the small one.
      mProgressDialog->slotToggleVisibility();
      return true;
    }
  }
  return false;
}
 
void StatusbarProgressWidget::slotProgressDialogVisible( bool b )
{
  // Update the hide/show button when the detailed one is shown/hidden
  if ( b ) {
    m_pButton->setIcon( SmallIcon( "go-down" ) );
    m_pButton->setToolTip( i18n("Hide detailed progress window") );
    setMode();
  } else {
    m_pButton->setIcon( SmallIcon( "go-up" ) );
    m_pButton->setToolTip( i18n("Show detailed progress window") );
  }
}
