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

#include "partexplorer_plugin.h"

#include <kinstance.h>
#include <kaction.h>
#include <klistview.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kmainwindow.h>
#include "kdevmainwindow.h"

#include "partexplorerform.h"

/**
* TODO: Create the main window only when it is requested: for now the form is simply hidden until
* requested. To further investigate.
* TODO: Must eliminate the creation of KListViewItem objects here: should add some methods in the
* part explorer form so this code does not need to access form's widgets. Must find a suitable way.
*/

typedef KGenericFactory<PartExplorerPlugin> PartExplorerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpartexplorer, PartExplorerPluginFactory( "kdevpartexplorer" ) );

PartExplorerPlugin::PartExplorerPlugin(  QObject *parent, const char *name, const QStringList & )
	: KDevPlugin( "PartExplorer", "partexplorer", parent, name ? name : "PartExplorerPlugin" )
{
	// we need an instance
	setInstance( PartExplorerPluginFactory::instance() );
	// set our XML-UI resource file
	setXMLFile( "kdevpartexplorer.rc" );

	KAction *action;
	// This plugin offer only one action, a menu entry for showing the PartExplorerForm ...
	action = new KAction( i18n("Part Explorer"), 0, this, SLOT(slotShowWidget()),
		actionCollection(), "show_partexplorerform" );

	// this should be your custom internal widget
	m_widget = new PartExplorerForm( 0L, "partexplorerform" );
	m_widget->hide();

	// When the user press "Search" button then this part performs the query.
	connect(
		m_widget, SIGNAL(searchQuery(QString,QString)),
		this, SLOT(slotSearchServices(QString,QString))
	);
	// Let the widget to display any error message we may encounter.
	connect(
		this, SIGNAL(displayError(QString)),
		m_widget, SLOT(slotDisplayError(QString))
	);

}

PartExplorerPlugin::~PartExplorerPlugin()
{
	delete m_widget;
}

void PartExplorerPlugin::slotShowWidget()
{
	m_widget->show();
}

void PartExplorerPlugin::slotSearchServices( QString serviceType, QString costraints)
{
	kdDebug(9000) << ">> slot slotSearchServices(QString, QString): " << endl
		<< "  ** serviceType = " << serviceType << ", costraints = " << costraints
		<< endl;

	KTrader::OfferList foundServices = KTrader::self()->query( serviceType, costraints );
	fillWidget( foundServices );
}

/**
* Property values in a KListView
*/
class PropertyItem : public KListViewItem
{
public:
    PropertyItem( KListViewItem *parent,
		const QString &propertyName, const QString &propertyType, const QString &propertyValue )
		: KListViewItem( parent )
	{
//	    setPixmap(0, SmallIcon("document"));
		setText( 0, propertyName );
		setText( 1, propertyType );
		setText( 2, propertyValue );
	}
};

void PartExplorerPlugin::fillWidget( const KTrader::OfferList &services )
{
	KListView *listView = m_widget->getListViewResults();
	listView->clear();

	if ( services.isEmpty())
	{
		kdDebug( 9000 ) << "OfferList is empty!" << endl;
		emit displayError( "No service found matching the criteria!" );
		return;
	}

	listView->setRootIsDecorated( true );

	KListViewItem *rootItem = 0;

	KTrader::OfferList::ConstIterator it = services.begin();
	for ( ; it != services.end(); ++it )
	{
		KService::Ptr service = (*it);
		kdDebug( 9000 ) << "  ** Found service: " << service->name() << endl;

		KListViewItem *serviceItem = new KListViewItem( listView, rootItem, service->name() );

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

#include "partexplorer_plugin.moc"
