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
#include <qpushbutton.h>

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
};

///////////////////////////////////////////////////////////////////////////////
// class PartExplorerForm
///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::PartExplorerForm( QWidget *parent )
    : KDialogBase( parent, "partexplorerform", true, i18n("PartExplorer, a services lister"),
        User1 | Close, User1, true )
{
    m_base = new PartExplorerFormBase( this, "partexplorerformbase" );
    this->setMainWidget( m_base );

	setButtonText( User1, i18n("&Start query") );

	connect( this, SIGNAL(user1Clicked()), this, SLOT(slotSearchRequested()) );
    connect( m_base->lineEditType, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
    connect( m_base->lineEditCostraints, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
}

PartExplorerForm::~PartExplorerForm()
{
}

void PartExplorerForm::slotSearchRequested()
{
    QString serviceType = m_base->lineEditType->text(),
        costraints = m_base->lineEditCostraints->text();

    kdDebug(9000) << ">> slot PartExplorerForm::slotSearchRequested(): " << endl
        << "  ** serviceType = " << serviceType << ", costraints = " << costraints
        << endl;

    if (serviceType.isNull() || serviceType.isEmpty())
    {
        slotDisplayError( "You must fill at least the type field!!" );
        return;
    }

	// Query for requested services
    KTrader::OfferList foundServices = KTrader::self()->query( serviceType, costraints );
	fillWidget( foundServices );
}

void PartExplorerForm::slotDisplayError( QString errorMessage )
{
    if (errorMessage.isNull() || errorMessage.isEmpty())
    {
        errorMessage = "Unknown error!";
    }
    KMessageBox::error( this, errorMessage );
}

void PartExplorerForm::fillWidget( const KTrader::OfferList &services )
{
    m_base->listViewResults->clear();

    if ( services.isEmpty())
    {
        kdDebug( 9000 ) << "OfferList is empty!" << endl;
        slotDisplayError( "No service found matching the criteria!" );
        return;
    }

    m_base->listViewResults->setRootIsDecorated( true );

    KListViewItem *rootItem = 0;

    KTrader::OfferList::ConstIterator it = services.begin();
    for ( ; it != services.end(); ++it )
    {
        KService::Ptr service = (*it);
        kdDebug( 9000 ) << "  ** Found service: " << service->name() << endl;

        KListViewItem *serviceItem = new KListViewItem( m_base->listViewResults, rootItem, service->name() );

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
            kdDebug( 9000 ) << "  **** Found property: " << propertyName << endl;
            kdDebug( 9000 ) << "  ****           Type: " << propertyType << endl;
            kdDebug( 9000 ) << "  ****          Value: " << propertyValue << endl;

            new PropertyItem( serviceItem, propertyName, propertyType, propertyValue );
        }
    }
}

#include "partexplorerform.moc"
