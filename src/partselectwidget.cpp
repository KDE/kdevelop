/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qvariant.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qregexp.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include <kurllabel.h>
#include <kapplication.h>
#include "domutil.h"

#include <qcomboview.h>
#include <profileengine.h>

#include "partselectwidget.h"
#include "plugincontroller.h"

class ProfileItem: public QListViewItem {
public:
    ProfileItem(QListView *parent, Profile *profile)
        :QListViewItem(parent), m_profile(profile)
    {
        setText(0, profile->genericName());
    }
    
    ProfileItem(QListViewItem *parent, Profile *profile)
        : QListViewItem(parent), m_profile(profile)
    {
        setText(0, profile->genericName());
    }
    
    Profile *profile() const { return m_profile; }
    
private:
    Profile *m_profile;
};

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


PartSelectWidget::PartSelectWidget(QDomDocument &projectDom,
                                   QWidget *parent, const char *name)
    : QWidget(parent, name), m_projectDom(projectDom)
{
    init();
}

void PartSelectWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
/*
    QGroupBox * groupBox0 = new QGroupBox( i18n("Plugin Profile for This Project"), this );
    groupBox0->setColumnLayout(0, Qt::Vertical );
    groupBox0->layout()->setSpacing( 6 );
    groupBox0->layout()->setMargin( 11 );
    QVBoxLayout * groupBox0Layout = new QVBoxLayout( groupBox0->layout() );
    _pluginProfile = new QComboView(false, groupBox0);
    _profile = DomUtil::readEntry(m_projectDom, "general/profile", PluginController::getInstance()->currentProfile());
    fillProfilesList();
    _pluginProfile->setCurrentText(PluginController::getInstance()->engine().findProfile(_profile)->genericName());
    
    connect(_pluginProfile, SIGNAL(activated(QListViewItem* )),
        this, SLOT(selectProfile(QListViewItem* )));
    groupBox0Layout->addWidget(_pluginProfile);
    QLabel *label0 = new QLabel(i18n("Note: Profile changes will take effect after the project is reloaded"), groupBox0);
    groupBox0Layout->addWidget(label0);
    layout->addWidget(groupBox0);
*/
    QString text( i18n("Plugins to Load for This Project") );

    QGroupBox * groupBox1 = new QGroupBox( text, this );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    QHBoxLayout * groupBox1Layout = new QHBoxLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    _pluginList = new QListView( groupBox1 );
    _pluginList->setResizeMode( QListView::LastColumn );
    _pluginList->addColumn("");
    _pluginList->header()->hide();

    groupBox1Layout->addWidget( _pluginList );
    layout->addWidget( groupBox1 );

    QGroupBox * groupBox2 = new QGroupBox( i18n("Description"), this );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    QVBoxLayout * groupBox2Layout = new QVBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    _pluginDescription = new QLabel( groupBox2 );
    _pluginDescription->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );

    _urlLabel = new KURLLabel( groupBox2 );

    groupBox2Layout->addWidget( _pluginDescription );
    groupBox2Layout->addWidget( _urlLabel );

    layout->addWidget( groupBox2 );
	
	QHBox * hbox = new QHBox( this );
	hbox->setSpacing( 6 );
	hbox->setMargin( 6 );
	QPushButton * setAsDefaultButton = new QPushButton( i18n("Set as Default"), hbox );
	setAsDefaultButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	new QLabel( i18n("Set this plugin selection as the default for this IDE profile"), hbox );
	layout->addWidget( hbox );

    connect( _pluginList, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( itemSelected( QListViewItem * ) ) );
	connect( _urlLabel, SIGNAL( leftClickedURL( const QString & ) ), this, SLOT( openURL( const QString & ) ) );
	connect( setAsDefaultButton, SIGNAL(clicked()), this, SLOT(setAsDefault()) );

	readProjectConfig();
}


PartSelectWidget::~PartSelectWidget()
{}

void PartSelectWidget::readProjectConfig()
{
	QStringList ignoreparts = DomUtil::readListEntry(m_projectDom, "/general/ignoreparts", "part");
	
	KTrader::OfferList localOffers = PluginController::getInstance()->engine().offers(
		PluginController::getInstance()->currentProfile(), ProfileEngine::Project);
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

		PluginItem *item = new PluginItem( _pluginList, (*it)->desktopEntryName(), (*it)->genericName(), Comment, url );
		item->setOn(!ignoreparts.contains((*it)->desktopEntryName()));
	}

	QListViewItem * first = _pluginList->firstChild();
	if ( first )
	{
		_pluginList->setSelected( first, true );
	}
}

void PartSelectWidget::itemSelected( QListViewItem * item )
{
    if ( ! item ) return;

    PluginItem * pitem = static_cast<PluginItem*>( item );
    _pluginDescription->setText( pitem->info() );

	if ( pitem->url().isEmpty() )
	{
		_urlLabel->hide();
	}
	else
	{
		_urlLabel->show();
		_urlLabel->setURL( pitem->url() );
		_urlLabel->setText( pitem->url() );
	}
}

void PartSelectWidget::openURL( const QString & url )
{
	kapp->invokeBrowser( url );
}

void PartSelectWidget::saveProjectConfig()
{
    QStringList ignoreparts;

    QListViewItemIterator it( _pluginList );
    while ( it.current() )
    {
        PluginItem * item = static_cast<PluginItem*>( it.current() );
        if ( ! item->isOn() )
        {
            ignoreparts.append( item->name() );
        }
        ++it;
    }

    DomUtil::writeListEntry(m_projectDom, "/general/ignoreparts", "part", ignoreparts);
//    if (_profile != PluginController::getInstance()->currentProfile())
//        DomUtil::writeEntry(m_projectDom, "/general/profile", _profile);
//    kdDebug(9000) << "xml:" << m_projectDom.toString() << endl;
}


void PartSelectWidget::accept()
{
    saveProjectConfig();
    emit accepted();
}

void PartSelectWidget::selectProfile(QListViewItem *item)
{
    ProfileItem *profileItem = dynamic_cast<ProfileItem*>(item);
    if (!profileItem)
        return;
//    _profile = profileItem->profile()->name();
    _pluginList->clear();
    readProjectConfig();
}

class ProfileListBuilding {
public:
    QListViewItem * operator() (QListViewItem *parent, Profile *profile)
    {
        parent->setOpen(true);
        return new ProfileItem(parent, profile);
    }
};

void PartSelectWidget::setAsDefault( )
{
	kdDebug() << k_funcinfo << endl;
	
	QString profile = DomUtil::readEntry(m_projectDom, "general/profile", PluginController::getInstance()->currentProfile());
	
	if ( profile.isEmpty() ) return;
	
	QStringList ignoreparts;
	QListViewItemIterator it( _pluginList );
	while ( it.current() )
	{
		PluginItem * item = static_cast<PluginItem*>( it.current() );
		if ( !item->isOn() )
		{
			ignoreparts.append( item->name() );
		}
		++it;
	}

	//@fixme: make this use ProfileEngine instead to store settings
	KConfig * config = kapp->config();
	config->setGroup("IgnorePerDefault");
//	config->writeEntry( profile, ignoreparts );	
	config->writeEntry( "KDevelop", ignoreparts );	
}

/*
void PartSelectWidget::fillProfilesList()
{
    ProfileEngine &engine = PluginController::getInstance()->engine();
    ProfileItem *item = new ProfileItem(_pluginProfile->listView(), engine.rootProfile());
    ProfileListBuilding op;
    engine.walkProfiles<ProfileListBuilding, QListViewItem>(op, item, engine.rootProfile());
}
*/
#include "partselectwidget.moc"
