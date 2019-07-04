/*
  statusbarprogresswidget.cpp

  (C) 2004 Till Adam <adam@kde.org>
           Don Sanders
           David Faure <dfaure@kde.org>

  Copyright 2004 David Faure <faure@kde.org>
  Includes StatusbarProgressWidget which is based on KIOLittleProgressDlg
  by Matt Koss <koss@miesto.sk>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2 or above,
  as published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "statusbarprogresswidget.h"
#include "progressdialog.h"
#include "progressmanager.h"
#ifdef Q_OS_OSX
#include "../macdockprogressview.h"
#endif

#include <KLocalizedString>

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <QApplication>
#include <QStyle>

using namespace KDevelop;

//-----------------------------------------------------------------------------
StatusbarProgressWidget::StatusbarProgressWidget( ProgressDialog* progressDialog, QWidget* parent, bool button )
    : QFrame( parent ), mCurrentItem( nullptr ), mProgressDialog( progressDialog ),
      mDelayTimer( nullptr ), mCleanTimer( nullptr )
{
    m_bShowButton = button;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int w = fontMetrics().horizontalAdvance(QStringLiteral(" 999.9 kB/s 00:00:01 ")) + 8;
#else
    int w = fontMetrics().width( QStringLiteral(" 999.9 kB/s 00:00:01 ") ) + 8;
#endif
    box = new QHBoxLayout( this );
    box->setMargin(0);
    box->setSpacing(0);
    stack = new QStackedWidget( this );

    m_pButton = new QToolButton( this );
    m_pButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                           QSizePolicy::Fixed ) );
    QIcon smallIcon = QIcon::fromTheme( QStringLiteral("go-up") );
    if ( smallIcon.isNull() ) {
        // this can happen everywhere but in particular with a standard build on OS X.
        // QToolButtons won't be visible without an icon, so fall back to showing a Qt::UpArrow.
        m_pButton->setArrowType( Qt::UpArrow );
    } else {
        m_pButton->setIcon( smallIcon );
    }
    m_pButton->setAutoRaise(true);
    QSize iconSize = m_pButton->iconSize();

    m_pProgressBar = new QProgressBar( this );
    m_pProgressBar->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                                QSizePolicy::Fixed ) );
    m_pProgressBar->installEventFilter( this );
    m_pProgressBar->setMinimumWidth( w );
    m_pProgressBar->setAttribute( Qt::WA_LayoutUsesWidgetRect, true );

    // Determine maximumHeight from the progressbar's height and scale the icon.
    // This operation is style specific and cannot infer the style in use
    // from Q_OS_??? because users can have started us using the -style option
    // (or even be using an unexpected QPA).
    // In most cases, maximumHeight should be set to fontMetrics().height() + 2
    // (Breeze, Oxygen, Fusion, Windows, QtCurve etc.); this corresponds to the actual
    // progressbar height plus a 1 pixel margin above and below.
    int maximumHeight = m_pProgressBar->fontMetrics().height() + 2;
    const bool isMacWidgetStyle = QApplication::style()->objectName() == QLatin1String( "macintosh" );

    if ( isMacWidgetStyle && !smallIcon.isNull() ) {
        // QProgressBar height is fixed with the macintosh native widget style
        // and alignment with m_pButton is tricky. Sizing the icon to maximumHeight
        // gives a button that is slightly too high and not perfectly
        // aligned. Annoyingly that doesn't improve by calling setMaximumHeight()
        // which even causes the button to change shape. So we use a "flat" button,
        // an invisible outline which is more in line with platform practices anyway.
        maximumHeight = m_pProgressBar->sizeHint().height();
        iconSize.scale( maximumHeight, maximumHeight, Qt::KeepAspectRatio );
    } else {
        // The icon is scaled to maximumHeight but with 1 pixel margins on each side
        // because it will be in a visible button.
        iconSize.scale( maximumHeight - 2, maximumHeight - 2, Qt::KeepAspectRatio );
        // additional adjustments:
        m_pButton->setAttribute( Qt::WA_LayoutUsesWidgetRect, true );
    }
    stack->setMaximumHeight( maximumHeight );
    m_pButton->setIconSize( iconSize );
    box->addWidget( m_pButton  );

    m_pButton->setToolTip( i18n("Open detailed progress dialog") );

    box->addWidget( stack );

    stack->insertWidget( 1, m_pProgressBar );

    if (m_bShowButton) {
        // create an empty, inactive QToolButton that's as high as m_pButton but only 1 pixel wide
        // this will act as a placeholder when the widget is invisible.
        m_pPlaceHolder.button = new QToolButton(this);
        m_pPlaceHolder.button->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                           QSizePolicy::Fixed ) );
        m_pPlaceHolder.button->setMinimumHeight(m_pButton->minimumSizeHint().height());
        m_pPlaceHolder.button->setMaximumWidth(1);
        m_pPlaceHolder.button->setAutoRaise(true);
        m_pPlaceHolder.button->setAttribute( Qt::WA_LayoutUsesWidgetRect, true );
        m_pPlaceHolder.button->setEnabled(false);
        m_pPlaceHolder.button->installEventFilter( this );
        // the placeholder button should not go into the stack to avoid misalignment
        box->addWidget( m_pPlaceHolder.button );
        m_pPlaceHolder.button->hide();
    } else {
        // when the widget doesn't show m_pButton we can use a QLabel as the placeholder.
        m_pPlaceHolder.label = new QLabel( QString(), this );
        m_pPlaceHolder.label->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                              QSizePolicy::Fixed ) );
        m_pPlaceHolder.label->setAlignment( Qt::AlignHCenter );
        m_pPlaceHolder.label->installEventFilter( this );
        m_pPlaceHolder.label->setMinimumWidth( w );
        m_pPlaceHolder.label->setMaximumHeight( maximumHeight );
        stack->insertWidget( 2, m_pPlaceHolder.label );
    }

    setMinimumWidth( minimumSizeHint().width() );

    mode = None;
    setMode();

    connect( m_pButton, &QPushButton::clicked,
             progressDialog, &ProgressDialog::slotToggleVisibility );

    connect ( ProgressManager::instance(), &ProgressManager::progressItemAdded,
              this, &StatusbarProgressWidget::slotProgressItemAdded );
    connect ( ProgressManager::instance(), &ProgressManager::progressItemCompleted,
              this, &StatusbarProgressWidget::slotProgressItemCompleted );
    connect ( ProgressManager::instance(), &ProgressManager::progressItemUsesBusyIndicator,
              this, &StatusbarProgressWidget::updateBusyMode );

    connect ( progressDialog, &ProgressDialog::visibilityChanged,
              this, &StatusbarProgressWidget::slotProgressDialogVisible );

    mDelayTimer = new QTimer( this );
    mDelayTimer->setSingleShot( true );
    mDelayTimer->setInterval(1000);
    connect ( mDelayTimer, &QTimer::timeout,
              this, &StatusbarProgressWidget::slotShowItemDelayed );

    mCleanTimer = new QTimer( this );
    mCleanTimer->setSingleShot( true );
    mCleanTimer->setInterval(5000);
    connect ( mCleanTimer, &QTimer::timeout,
              this, &StatusbarProgressWidget::slotClean );
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::updateBusyMode()
{
    connectSingleItem(); // if going to 1 item
    if (!mDelayTimer->isActive())
        mDelayTimer->start();
}

void StatusbarProgressWidget::slotProgressItemAdded( ProgressItem *item )
{
    if ( item->parent() )
        return; // we are only interested in top level items

    updateBusyMode();
}

void StatusbarProgressWidget::slotProgressItemCompleted( ProgressItem *item )
{
    if ( item->parent() ) {
        item->deleteLater();
        item = nullptr;
        return; // we are only interested in top level items
    }

    bool itemUsesBusyIndicator = item->usesBusyIndicator();
    item->deleteLater();
    item = nullptr;

    connectSingleItem(); // if going back to 1 item
    if ( ProgressManager::instance()->isEmpty() ) { // No item
        // If completed item uses busy indicator and progress manager doesn't have any
        // other items, then we should set it progress to 100% to indicate finishing.
        // Without this fix we will show busy indicator for already finished item
        // for next 5s.
        if ( itemUsesBusyIndicator ) {
            activateSingleItemMode( 100 );
        }

        // Done. In 5s the progress-widget will close, then we can clean up the statusbar
        mCleanTimer->start();
    } else if ( mCurrentItem ) { // Exactly one item
        activateSingleItemMode();
    }
}

void StatusbarProgressWidget::connectSingleItem()
{
    if ( mCurrentItem ) {
        disconnect ( mCurrentItem, &ProgressItem::progressItemProgress,
                     this, &StatusbarProgressWidget::slotProgressItemProgress );
        mCurrentItem = nullptr;
    }
    mCurrentItem = ProgressManager::instance()->singleItem();
    if ( mCurrentItem ) {
        connect ( mCurrentItem, &ProgressItem::progressItemProgress,
                  this, &StatusbarProgressWidget::slotProgressItemProgress );
    }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
    activateSingleItemMode( mCurrentItem->progress() );
}

void StatusbarProgressWidget::activateSingleItemMode( unsigned int progress )
{
    m_pProgressBar->setMaximum( 100 );
    m_pProgressBar->setValue( progress );
    m_pProgressBar->setTextVisible( true );
#ifdef Q_OS_OSX
    MacDockProgressView::setRange( 0, 100 );
    MacDockProgressView::setProgress( progress );
#endif
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
    bool noItems = ProgressManager::instance()->isEmpty();
    if ( mCurrentItem ) {
        activateSingleItemMode();
    } else if ( !noItems ) { // N items
        m_pProgressBar->setMaximum( 0 );
        m_pProgressBar->setTextVisible( false );
#ifdef Q_OS_OSX
        MacDockProgressView::setRange( 0, 0 );
        MacDockProgressView::setProgress( 0 );
#endif
    }

    if ( !noItems && mode == None ) {
        mode = Progress;
        setMode();
    }
}

void StatusbarProgressWidget::slotProgressItemProgress( ProgressItem *item, unsigned int value )
{
    Q_ASSERT( item == mCurrentItem); // the only one we should be connected to
    Q_UNUSED( item );
    m_pProgressBar->setValue( value );
#ifdef Q_OS_OSX
    MacDockProgressView::setProgress( value );
#endif
}

void StatusbarProgressWidget::setMode() {
    switch ( mode ) {
    case None:
        m_pButton->hide();
        if ( m_bShowButton ) {
            // show the empty button in order to make the status bar look better
            m_pPlaceHolder.button->show();
        } else {
            // show the empty label in order to make the status bar look better
            stack->setCurrentWidget( m_pPlaceHolder.label );
        }
        m_pProgressBar->hide();
        stack->show();
#ifdef Q_OS_OSX
        MacDockProgressView::setProgressVisible( false );
#endif
        break;

    case Progress:
        stack->show();
        m_pProgressBar->show();
        stack->setCurrentWidget( m_pProgressBar );
        if ( m_bShowButton ) {
            m_pButton->show();
            m_pPlaceHolder.button->hide();
        }
#ifdef Q_OS_OSX
        MacDockProgressView::setProgressVisible( true );
#endif
        break;
    }
}

void StatusbarProgressWidget::slotClean()
{
    // check if a new item showed up since we started the timer. If not, clear
    if ( ProgressManager::instance()->isEmpty() ) {
        m_pProgressBar->setValue( 0 );
        //m_pPlaceHolder.label->clear();
        mode = None;
        setMode();
    }
}

bool StatusbarProgressWidget::eventFilter(QObject* object, QEvent* ev)
{
    if ( ev->type() == QEvent::MouseButtonPress ) {
        auto *e = (QMouseEvent*)ev;

        if ( e->button() == Qt::LeftButton && mode != None ) {    // toggle view on left mouse button
            // Consensus seems to be that we should show/hide the fancy dialog when the user
            // clicks anywhere in the small one.
            mProgressDialog->slotToggleVisibility();
            return true;
        }
    }
    return QFrame::eventFilter(object, ev);
}

void StatusbarProgressWidget::slotProgressDialogVisible( bool b )
{
    // Update the hide/show button when the detailed one is shown/hidden
    if ( b ) {
        m_pButton->setIcon( QIcon::fromTheme( QStringLiteral("go-down") ) );
        m_pButton->setToolTip( i18n("Hide detailed progress window") );
        setMode();
    } else {
        m_pButton->setIcon( QIcon::fromTheme( QStringLiteral("go-up") ) );
        m_pButton->setToolTip( i18n("Show detailed progress window") );
    }
}

