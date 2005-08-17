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
#include <q3textedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <q3whatsthis.h>
#include <qlabel.h>

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kservicetype.h> 

#include "partexplorerformbase.h"
#include "partexplorerform.h"

///////////////////////////////////////////////////////////////////////////////
// class PropertyItem
///////////////////////////////////////////////////////////////////////////////
namespace PartExplorer{

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

}
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
    PartExplorer::PropertyItem *item = dynamic_cast<PartExplorer::PropertyItem*>( m_resultsList->itemAt( p ) );
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
    Q3WhatsThis::add( m_resultsList, i18n("<b>Matching services</b><p>Results (if any) are shown grouped by matching service name.") );
    m_base->resultsLabel->setBuddy(m_resultsList);
    m_base->layout()->add( m_resultsList );
    setMainWidget( m_base );
    m_base->typeCombo->lineEdit()->setFocus();

    // User1 button text
    setButtonText( User1, i18n("&Search") );

    // Resize dialog
    resize( 480, 512 );

//    connect( m_base->typeCombo->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
//    connect( m_base->constraintsText, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );

    connect( actionButton(User1), SIGNAL(clicked()), this, SLOT(slotSearchRequested()) );
//    connect( m_base->typeCombo->lineEdit(), SIGNAL( textChanged ( const QString & ) ), this,  SLOT( slotServicetypeChanged( const QString&  ) ) );
//    slotServicetypeChanged( m_base->typeCombo->lineEdit()->text() );

	// populating with all known servicetypes
	KServiceType::List serviceList = KServiceType::allServiceTypes();
	QStringList list;
	KServiceType::List::Iterator it = serviceList.begin();
	while( it != serviceList.end() )
	{
		list << (*it)->name();
		++it;
	}
	list.sort();
	m_base->typeCombo->insertStringList( list );
}

///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::~PartExplorerForm()
{
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotSearchRequested()
{
	QString serviceType = m_base->typeCombo->lineEdit()->text();
	QString constraints = m_base->constraintsText->text();

    kdDebug(9000) << "===> PartExplorerForm::slotSearchRequested(): " <<
        " serviceType = " << serviceType << ", constraints = " << constraints << endl;

    // Query for requested services
    KTrader::OfferList foundServices = KTrader::self()->query( serviceType, constraints );
    fillServiceList( foundServices );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotDisplayError( QString errorMessage )
{
    if (errorMessage.isEmpty())
    {
        errorMessage = i18n("Unknown error.");
    }
    KMessageBox::error( this, errorMessage );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::fillServiceList( const KTrader::OfferList &services )
{
    this->m_resultsList->clear();

    if ( services.isEmpty())
    {
        slotDisplayError( i18n("No service found matching the criteria.") );
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

            new PartExplorer::PropertyItem( serviceItem, propertyName, propertyType, propertyValue );
        }
    }
}

#include "partexplorerform.moc"
