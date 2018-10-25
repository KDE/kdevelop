/** -*- c++ -*-
 * progressdialog.cpp
 *
 *  Copyright (c) 2004 Till Adam <adam@kde.org>,
 *                     David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "progressdialog.h"
#include "progressmanager.h"

#include <KLocalizedString>

#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace KDevelop {

static const int MAX_LABEL_WIDTH = 650;

class TransactionItem;

TransactionItemView::TransactionItemView( QWidget *parent, const char *name )
    : QScrollArea( parent )
{
    setObjectName(QString::fromUtf8(name));
    setFrameStyle( NoFrame );
    mBigBox = new QWidget( this );
    auto layout = new QVBoxLayout(mBigBox);
    layout->setMargin(0);
    setWidget( mBigBox );
    setWidgetResizable( true );
    setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
}

TransactionItem *TransactionItemView::addTransactionItem( ProgressItem *item, bool first )
{
    auto *ti = new TransactionItem( mBigBox, item, first );
    mBigBox->layout()->addWidget( ti );

    resize( mBigBox->width(), mBigBox->height() );

    return ti;
}

void TransactionItemView::resizeEvent ( QResizeEvent *event )
{
    // Tell the layout in the parent (progressdialog) that our size changed
    updateGeometry();

    QSize sz = parentWidget()->sizeHint();
    int currentWidth = parentWidget()->width();

    // Don't resize to sz.width() every time when it only reduces a little bit
    if ( currentWidth < sz.width() || currentWidth > sz.width() + 100 ) {
        currentWidth = sz.width();
    }
    parentWidget()->resize( currentWidth, sz.height() );

    QScrollArea::resizeEvent( event );
}

QSize TransactionItemView::sizeHint() const
{
    return minimumSizeHint();
}

QSize TransactionItemView::minimumSizeHint() const
{
    int f = 2 * frameWidth();
    // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
    int vsbExt = verticalScrollBar()->sizeHint().width();
    QSize sz( mBigBox->minimumSizeHint() );
    sz.setWidth( sz.width() + f + vsbExt );
    sz.setHeight( sz.height() + f );
    return sz;
}

void TransactionItemView::slotItemCompleted(TransactionItem* item)
{
    // If completed item is the first, hide separator line for the one that will become first now
    if (mBigBox->layout()->indexOf(item) == 0) {
        auto *secondItem = mBigBox->layout()->itemAt(1);
        if (secondItem) {
            static_cast<TransactionItem *>(secondItem->widget())->hideHLine();
        }
    }

    mBigBox->layout()->removeWidget(item);
    delete item;

    //This slot is called whenever a TransactionItem is deleted, so this is a
    //good place to call updateGeometry(), so our parent takes the new size
    //into account and resizes.
    updateGeometry();
}

// ----------------------------------------------------------------------------

TransactionItem::TransactionItem( QWidget *parent,
                                  ProgressItem *item, bool first )
    : QWidget( parent ), mCancelButton( nullptr ), mItem( item )
{
    auto vbox = new QVBoxLayout(this);
    vbox->setSpacing( 2 );
    vbox->setMargin( 2 );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );

    mFrame = new QFrame( this );
    mFrame->setFrameShape( QFrame::HLine );
    mFrame->setFrameShadow( QFrame::Raised );
    mFrame->show();
    vbox->setStretchFactor( mFrame, 3 );
    vbox->addWidget( mFrame );

    QWidget *h = new QWidget( this );
    auto hboxLayout = new QHBoxLayout(h);
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing( 5 );
    vbox->addWidget( h );

    mItemLabel =
            new QLabel( fontMetrics().elidedText( item->label(), Qt::ElideRight, MAX_LABEL_WIDTH ), h );
    h->layout()->addWidget( mItemLabel );
    h->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );

    mProgress = new QProgressBar( h );
    hboxLayout->addWidget(mProgress);
    mProgress->setMaximum( 100 );
    mProgress->setValue( item->progress() );
    h->layout()->addWidget( mProgress );

    if ( item->canBeCanceled() ) {
        mCancelButton = new QPushButton( QIcon::fromTheme( QStringLiteral("dialog-cancel") ), QString(), h );
        hboxLayout->addWidget(mCancelButton);
        mCancelButton->setToolTip( i18n( "Cancel this operation." ) );
        connect ( mCancelButton, &QPushButton::clicked,
                  this, &TransactionItem::slotItemCanceled);
        h->layout()->addWidget( mCancelButton );
    }

    h = new QWidget( this );
    hboxLayout = new QHBoxLayout(h);
    hboxLayout->setMargin(0);
    hboxLayout->setSpacing( 5 );
    h->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    vbox->addWidget( h );
    mItemStatus = new QLabel( h );
    hboxLayout->addWidget(mItemStatus);
    mItemStatus->setTextFormat( Qt::RichText );
    mItemStatus->setText(
                fontMetrics().elidedText( item->status(), Qt::ElideRight, MAX_LABEL_WIDTH ) );
    h->layout()->addWidget( mItemStatus );
    if ( first ) {
        hideHLine();
    }
}

TransactionItem::~TransactionItem()
{
}

void TransactionItem::hideHLine()
{
    mFrame->hide();
}

void TransactionItem::setProgress( int progress )
{
    mProgress->setValue( progress );
}

void TransactionItem::setLabel( const QString &label )
{
    mItemLabel->setText( fontMetrics().elidedText( label, Qt::ElideRight, MAX_LABEL_WIDTH ) );
}

void TransactionItem::setStatus( const QString &status )
{
    mItemStatus->setText( fontMetrics().elidedText( status, Qt::ElideRight, MAX_LABEL_WIDTH ) );
}

void TransactionItem::setTotalSteps( int totalSteps )
{
    mProgress->setMaximum( totalSteps );
}

void TransactionItem::slotItemCanceled()
{
    if ( mItem ) {
        mItem->cancel();
    }
}

void TransactionItem::addSubTransaction( ProgressItem *item )
{
    Q_UNUSED( item );
}

// ---------------------------------------------------------------------------

ProgressDialog::ProgressDialog( QWidget *alignWidget, QWidget *parent, const char *name )
    : OverlayWidget( alignWidget, parent, name ), mWasLastShown( false )
{
    setAutoFillBackground( true );

    mScrollView = new TransactionItemView( this, "ProgressScrollView" );
    layout()->addWidget( mScrollView );

    // No more close button for now, since there is no more autoshow
    /*
    QVBox* rightBox = new QVBox( this );
    QToolButton* pbClose = new QToolButton( rightBox );
    pbClose->setAutoRaise(true);
    pbClose->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    pbClose->setFixedSize( 16, 16 );
    pbClose->setIcon( KIconLoader::global()->loadIconSet( "window-close", KIconLoader::Small, 14 ) );
    pbClose->setToolTip( i18n( "Hide detailed progress window" ) );
    connect(pbClose, SIGNAL(clicked()), this, SLOT(slotClose()));
    QWidget* spacer = new QWidget( rightBox ); // don't let the close button take up all the height
    rightBox->setStretchFactor( spacer, 100 );
  */

    /*
   * Get the singleton ProgressManager item which will inform us of
   * appearing and vanishing items.
   */
    ProgressManager *pm = ProgressManager::instance();
    connect ( pm, &ProgressManager::progressItemAdded,
              this, &ProgressDialog::slotTransactionAdded );
    connect ( pm, &ProgressManager::progressItemCompleted,
              this, &ProgressDialog::slotTransactionCompleted );
    connect ( pm, &ProgressManager::progressItemProgress,
              this, &ProgressDialog::slotTransactionProgress );
    connect ( pm, &ProgressManager::progressItemStatus,
              this, &ProgressDialog::slotTransactionStatus );
    connect ( pm, &ProgressManager::progressItemLabel,
              this, &ProgressDialog::slotTransactionLabel );
    connect ( pm, &ProgressManager::progressItemUsesBusyIndicator,
              this, &ProgressDialog::slotTransactionUsesBusyIndicator );
    connect ( pm, &ProgressManager::showProgressDialog,
              this, &ProgressDialog::slotShow );
}

void ProgressDialog::closeEvent( QCloseEvent *e )
{
    e->accept();
    hide();
}

/*
 *  Destructor
 */
ProgressDialog::~ProgressDialog()
{
    // no need to delete child widgets.
}

void ProgressDialog::slotTransactionAdded( ProgressItem *item )
{
    if ( item->parent() ) {
        if ( mTransactionsToListviewItems.contains( item->parent() ) ) {
            TransactionItem * parent = mTransactionsToListviewItems[ item->parent() ];
            parent->addSubTransaction( item );
        }
    } else {
        const bool first = mTransactionsToListviewItems.empty();
        TransactionItem *ti = mScrollView->addTransactionItem( item, first );
        if ( ti ) {
            mTransactionsToListviewItems.insert( item, ti );
        }
        if ( first && mWasLastShown ) {
            QTimer::singleShot( 1000, this, &ProgressDialog::slotShow );
        }

    }
}

void ProgressDialog::slotTransactionCompleted( ProgressItem *item )
{
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        mTransactionsToListviewItems.remove( item );
        ti->setItemComplete();
        QTimer::singleShot( 3000, mScrollView, [=] { mScrollView->slotItemCompleted(ti); } );
    }
    // This was the last item, hide.
    if ( mTransactionsToListviewItems.empty() ) {
        QTimer::singleShot( 3000, this, &ProgressDialog::slotHide );
    }
}

void ProgressDialog::slotTransactionCanceled( ProgressItem * )
{
}

void ProgressDialog::slotTransactionProgress( ProgressItem *item,
                                              unsigned int progress )
{
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setProgress( progress );
    }
}

void ProgressDialog::slotTransactionStatus( ProgressItem *item,
                                            const QString &status )
{
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setStatus( status );
    }
}

void ProgressDialog::slotTransactionLabel( ProgressItem *item,
                                           const QString &label )
{
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        ti->setLabel( label );
    }
}

void ProgressDialog::slotTransactionUsesBusyIndicator( KDevelop::ProgressItem *item, bool value )
{
    if ( mTransactionsToListviewItems.contains( item ) ) {
        TransactionItem *ti = mTransactionsToListviewItems[ item ];
        if ( value ) {
            ti->setTotalSteps( 0 );
        } else {
            ti->setTotalSteps( 100 );
        }
    }
}

void ProgressDialog::slotShow()
{
    setVisible( true );
}

void ProgressDialog::slotHide()
{
    // check if a new item showed up since we started the timer. If not, hide
    if ( mTransactionsToListviewItems.isEmpty() ) {
        setVisible( false );
    }
    mWasLastShown = false;
}

void ProgressDialog::slotClose()
{
    mWasLastShown = false;
    setVisible( false );
}

void ProgressDialog::setVisible( bool b )
{
    OverlayWidget::setVisible( b );
    emit visibilityChanged( b );
}

void ProgressDialog::slotToggleVisibility()
{
    /* Since we are only hiding with a timeout, there is a short period of
   * time where the last item is still visible, but clicking on it in
   * the statusbarwidget should not display the dialog, because there
   * are no items to be shown anymore. Guard against that.
   */
    mWasLastShown = isHidden();
    if ( !isHidden() || !mTransactionsToListviewItems.isEmpty() ) {
        setVisible( isHidden() );
    }
}

}

