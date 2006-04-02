/***************************************************************************
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlistview.h>
#include <qheader.h>
#include <qlabel.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kurllabel.h>

#include "kdevplugin.h"
#include "projectmanager.h"
#include "plugincontroller.h"
#include "pluginselectdialog.h"

class PluginItem : public QCheckListItem
{
public:
	// name - "Name", label - "GenericName", info - "Comment"
	PluginItem( QListView * parent, QString const & name, QString const & label,
				QString const & info, QString const url = QString::null )
		: QCheckListItem( parent, label, QCheckListItem::CheckBox),
		_name( name ), _info( info ), _url( url )
	{}

	QString info() { return _info; }
	QString name() { return _name; }
	QString url()  { return _url; }

private:
	QString _name;
	QString _info;
	QString _url;
};


PluginSelectDialog::PluginSelectDialog(QWidget* parent, const char* name, bool modal, WFlags fl )
	: PluginSelectDialogBase( parent,name, modal,fl )
{
	plugin_list->setResizeMode( QListView::LastColumn );
	plugin_list->addColumn("");
	plugin_list->header()->hide();

	connect( plugin_list, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( itemSelected( QListViewItem * ) ) );
	connect( urllabel, SIGNAL( leftClickedURL( const QString & ) ), this, SLOT( openURL( const QString & ) ) );

	init();
}

PluginSelectDialog::~PluginSelectDialog()
{
}

void PluginSelectDialog::saveAsDefault()
{
	kdDebug(9000) << k_funcinfo << endl;

	ProfileEngine & engine = PluginController::getInstance()->engine();
	Profile * profile = engine.findProfile( PluginController::getInstance()->currentProfile() );

	profile->clearList( Profile::ExplicitDisable );

	QListViewItemIterator it( plugin_list );
	while ( it.current() )
	{
		PluginItem * item = static_cast<PluginItem*>( it.current() );
		if ( !item->isOn() )
		{
			profile->addEntry( Profile::ExplicitDisable, item->name() );
		}
		++it;
	}

	profile->save();
}

void PluginSelectDialog::openURL( const QString & url )
{
	kapp->invokeBrowser( url );
}

void PluginSelectDialog::itemSelected( QListViewItem * item )
{
    if ( ! item ) return;

    PluginItem * pitem = static_cast<PluginItem*>( item );
    plugin_description_label->setText( pitem->info() );

	if ( pitem->url().isEmpty() )
	{
		urllabel->clear();
	}
	else
	{
		urllabel->setURL( pitem->url() );
		urllabel->setText( pitem->url() );
	}
}

void PluginSelectDialog::init( )
{
	const QValueList<KDevPlugin*> loadedPlugins = PluginController::getInstance()->loadedPlugins();
	QStringList loadedPluginDesktopNames;
	QValueList<KDevPlugin*>::ConstIterator it = loadedPlugins.begin();
	while( it != loadedPlugins.end() )
	{
		loadedPluginDesktopNames << (*it)->instance()->instanceName();
		++it;
	}

	kdDebug(9000) << " *** loadedPluginDesktopNames: " << loadedPluginDesktopNames << endl;

	KTrader::OfferList localOffers;
	if ( ProjectManager::getInstance()->projectLoaded() )
	{
		localOffers = PluginController::getInstance()->engine().offers(
			PluginController::getInstance()->currentProfile(), ProfileEngine::Project );
	}

	KTrader::OfferList globalOffers = PluginController::getInstance()->engine().offers(
		PluginController::getInstance()->currentProfile(), ProfileEngine::Global);

	KTrader::OfferList offers = localOffers + globalOffers;
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		// parse out any existing url to make it clickable
		QString Comment = (*it)->comment();
		QRegExp re("\\bhttp://[\\S]*");
		re.search( Comment );
		Comment.replace( re, "" );

		QString url;
		if ( re.pos() > -1 )
		{
			url = re.cap();
		}

		PluginItem *item = new PluginItem( plugin_list, (*it)->desktopEntryName(), (*it)->genericName(), Comment, url );
		item->setOn( loadedPluginDesktopNames.contains( (*it)->desktopEntryName() ) );

		kdDebug(9000) << (*it)->desktopEntryName() << " : " << (loadedPluginDesktopNames.contains( (*it)->desktopEntryName() ) ? "YES" : "NO" ) << endl;
	}

	QListViewItem * first = plugin_list->firstChild();
	if ( first )
	{
		plugin_list->setSelected( first, true );
	}
}

QStringList PluginSelectDialog::unselectedPluginNames( )
{
	QStringList unselectedPlugins;
	QListViewItem * item = plugin_list->firstChild();
	while ( item )
	{
		PluginItem * pluginItem = static_cast<PluginItem*>( item );
		if ( !pluginItem->isOn() )
		{
			unselectedPlugins << pluginItem->name();
		}
		item = item->nextSibling();
	}
	return unselectedPlugins;
}



#include "pluginselectdialog.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
