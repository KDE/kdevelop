/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qlabel.h>

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "partexplorerformbase.h"
#include "partexplorerform.h"

///////////////////////////////////////////////////////////////////////////////
// class PropertyItem
///////////////////////////////////////////////////////////////////////////////

class PropertyItem : public KListViewItem
{
public:
    PropertyItem( KListViewItem *parent, const QString &propertyName,
        const QString &propertyType, const QString &propertyValue )
        : KListViewItem( parent )
    {
        setText( 0, propertyName );
        setText( 1, propertyType );
        setText( 2, propertyValue );
    }

    QString tipText() const
    {
        QString tip = i18n("Name: %1 | Type: %2 | Value: %3");
        return tip.arg( text(0) ).arg( text(1) ).arg( text(2) );
    }
};

///////////////////////////////////////////////////////////////////////////////
// class ResultsList
///////////////////////////////////////////////////////////////////////////////

class ResultList;

class ResultsToolTip: public QToolTip
{
public:
    ResultsToolTip( ResultsList* parent );
    virtual void maybeTip( const QPoint& p );

private:
    ResultsList* m_resultsList;
};

class ResultsList : public KListView
{
public:
    ResultsList( QWidget *parent )
        : KListView( parent, "resultslist" )
    {
        this->setShowToolTips( false );
        new ResultsToolTip( this );
    }

    virtual ~ResultsList() {}

    void clear()
    {
        KListView::clear();
    }
};

ResultsToolTip::ResultsToolTip( ResultsList* parent )
    : QToolTip( parent->viewport() ), m_resultsList( parent )
{
}

void ResultsToolTip::maybeTip( const QPoint& p )
{
    PropertyItem *item = dynamic_cast<PropertyItem*>( m_resultsList->itemAt( p ) );
    if ( item )
    {
        QRect r = m_resultsList->itemRect( item );
        if ( r.isValid() )
            tip( r, item->tipText() );
    }
}


///////////////////////////////////////////////////////////////////////////////
// class PartExplorerForm
///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::PartExplorerForm( QWidget *parent )
    : KDialogBase( parent, "parteplorerform", false,
        i18n("Part Explorer - A Services Lister"), User1 | Close, User1, true )
{
    m_base = new PartExplorerFormBase( this, "partexplorerformbase", 0 );
    m_resultsList = new ResultsList( m_base );
    m_resultsList->addColumn( i18n( "Property" ) );
    m_resultsList->addColumn( i18n( "Type" ) );
    m_resultsList->addColumn( i18n( "Value" ) );
    m_resultsList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3,
        (QSizePolicy::SizeType)3, 0, 0,
        m_resultsList->sizePolicy().hasHeightForWidth() ) );
    QWhatsThis::add( m_resultsList, i18n("<b>Matching services</b><p>Results (if any) are shown grouped by matching service name.") );
    m_base->resultsLabel->setBuddy(m_resultsList);
    m_base->layout()->add( m_resultsList );
    setMainWidget( m_base );
    m_base->typeEdit->setFocus();

    // User1 button text
    setButtonText( User1, i18n("&Search") );

    // Resize dialog
    resize( 480, 512 );

    connect( m_base->typeEdit, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
    connect( m_base->costraintsText, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );

    connect( actionButton(User1), SIGNAL(clicked()), this, SLOT(slotSearchRequested()) );
}

///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::~PartExplorerForm()
{
}

///////////////////////////////////////////////////////////////////////////////

QString PartExplorerForm::serviceType() const
{
    QString st = m_base->typeEdit->text();

    return st.isEmpty()? QString::null : st;
}

///////////////////////////////////////////////////////////////////////////////

QString PartExplorerForm::costraints() const
{
    QString c = m_base->costraintsText->text();
    return c.isEmpty()? QString::null : c;
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotSearchRequested()
{
    QString serviceType = this->serviceType(),
        costraints = this->costraints();

    kdDebug(9000) << "===> PartExplorerForm::slotSearchRequested(): " <<
        " serviceType = " << serviceType << ", costraints = " << costraints << endl;

    if (serviceType.isNull())  // It is mandatory
    {
        slotDisplayError( i18n("You must fill at least the service type!!") );
        return;
    }

    // Query for requested services
    KTrader::OfferList foundServices = KTrader::self()->query( serviceType, costraints );
    fillServiceList( foundServices );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotDisplayError( QString errorMessage )
{
    if (errorMessage.isEmpty())
    {
        errorMessage = i18n("Unknown error!");
    }
    KMessageBox::error( this, errorMessage );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::fillServiceList( const KTrader::OfferList &services )
{
    this->m_resultsList->clear();

    if ( services.isEmpty())
    {
        slotDisplayError( i18n("No service found matching the criteria!") );
        return;
    }

    this->m_resultsList->setRootIsDecorated( true );

    KListViewItem *rootItem = 0;

    KTrader::OfferList::ConstIterator it = services.begin();
    for ( ; it != services.end(); ++it )
    {
        KService::Ptr service = (*it);
        KListViewItem *serviceItem = new KListViewItem( this->m_resultsList, rootItem, service->name() );

        QStringList propertyNames = service->propertyNames();
        for ( QStringList::const_iterator it = propertyNames.begin(); it != propertyNames.end(); ++it )
        {
            QString propertyName = (*it);
            QVariant property = service->property( propertyName );
            QString propertyType = property.typeName();
            QString propertyValue;
            if (propertyType == "QStringList") {
                propertyValue = property.toStringList().join(", ");
            }
            else {
                propertyValue = property.toString();
            }

            QString dProperty = " *** Found property < %1, %2, %3 >";
            dProperty = dProperty.arg( propertyName ).arg( propertyType ).arg( propertyValue );
            kdDebug( 9000 ) << dProperty << endl;

            new PropertyItem( serviceItem, propertyName, propertyType, propertyValue );
        }
    }
}

#include "partexplorerform.moc"
