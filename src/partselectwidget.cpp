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
    : QWidget(parent, name), m_projectDom(projectDom), _scope(Project)
{
    init();
}


PartSelectWidget::PartSelectWidget(QWidget *parent, const char *name)
    : QWidget(parent, name), m_projectDom(QDomDocument()), _scope(Global)
{
    init();
}


void PartSelectWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

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

    QString text = (_scope==Global)?
        i18n("Plugins to Load at Startup") :
        i18n("Plugins to Load for This Project");

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

    connect( _pluginList, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( itemSelected( QListViewItem * ) ) );
	connect( _urlLabel, SIGNAL( leftClickedURL( const QString & ) ), this, SLOT( openURL( const QString & ) ) );

    if (_scope == Global)
        readGlobalConfig();
    else
        readProjectConfig();
}


PartSelectWidget::~PartSelectWidget()
{}


void PartSelectWidget::readGlobalConfig()
{
    //FIXME: fix this, use new profiles arch !!!!!
    KTrader::OfferList globalOffers = PluginController::getInstance()->engine().offers(
        PluginController::getInstance()->currentProfile(), ProfileEngine::Global);
//     KConfig config( PluginController::getInstance()->currentProfilePath() );
//     config.setGroup("Plugins");

    for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it)
    {
//		parse out any existing url to make it clickable
		QString Comment = (*it)->comment();
		QRegExp re("\\bhttp://[\\S]*");
		re.search( Comment );
		Comment.replace( re, "" );

		QString url;
		if ( re.pos() > -1 )
		{
			url = re.cap();
		}

        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), (*it)->genericName(), Comment, url );
        item->setOn(/*config.readBoolEntry((*it)->name(), true)*/ true);
    }

	QListViewItem * first = _pluginList->firstChild();
	if ( first )
	{
		_pluginList->setSelected( first, true );
	}
}


void PartSelectWidget::saveGlobalConfig()
{
    //FIXME: fix this, use new profiles arch !!!!!!!!!!!!
/*    KConfig config( PluginController::getInstance()->currentProfilePath() );
    config.setGroup("Plugins");

    QListViewItemIterator it( _pluginList );
    while ( it.current() )
    {
        PluginItem * item = static_cast<PluginItem*>( it.current() );
        config.writeEntry( item->name(), item->isOn() );
        ++it;
    }*/
}


void PartSelectWidget::readProjectConfig()
{
    //FIXME: include not only project plugins but also global ones here
    QStringList ignoreparts = DomUtil::readListEntry(m_projectDom, "/general/ignoreparts", "part");

    KTrader::OfferList localOffers = PluginController::getInstance()->engine().offers(
        PluginController::getInstance()->currentProfile(), ProfileEngine::Project);
    for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it)
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

        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), (*it)->genericName(), Comment, url );
        item->setOn(!ignoreparts.contains((*it)->name()));
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
    if (_profile != PluginController::getInstance()->currentProfile())
        DomUtil::writeEntry(m_projectDom, "/general/profile", _profile);
    kdDebug(9000) << "xml:" << m_projectDom.toString() << endl;
}


void PartSelectWidget::accept()
{
    if (_scope == Global)
        saveGlobalConfig();
    else
        saveProjectConfig();
    emit accepted();
}

void PartSelectWidget::selectProfile(QListViewItem *item)
{
    ProfileItem *profileItem = dynamic_cast<ProfileItem*>(item);
    if (!profileItem)
        return;
    _profile = profileItem->profile()->name();
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

void PartSelectWidget::fillProfilesList()
{
    ProfileEngine &engine = PluginController::getInstance()->engine();
    ProfileItem *item = new ProfileItem(_pluginProfile->listView(), engine.rootProfile());
    ProfileListBuilding op;
    engine.walkProfiles<ProfileListBuilding, QListViewItem>(op, item, engine.rootProfile());
}

#include "partselectwidget.moc"
