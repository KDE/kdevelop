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

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include "domutil.h"

#include "partselectwidget.h"
#include "plugincontroller.h"

class PluginItem : public QCheckListItem
{
public:
    // name - "Name", label - "GenericName", info - "Comment"
    PluginItem( QListView * parent, QString const & name, QString const & label, QString const & info)
        : QCheckListItem( parent, label, QCheckListItem::CheckBox),
        _name( name ), _info( info )
    {}

    QString info() { return _info; }
    QString name() { return _name; }

private:

    QString _name;
    QString _info;
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

    QString text = (_scope==Global)?
        i18n("Plugins to load at startup:") :
        i18n("Plugins to load for this project:");

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

    QGroupBox * groupBox2 = new QGroupBox( i18n("Description:"), this );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    QHBoxLayout * groupBox2Layout = new QHBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    _pluginDescription = new QLabel( groupBox2 );
    _pluginDescription->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );

    groupBox2Layout->addWidget( _pluginDescription );
    layout->addWidget( groupBox2 );

    connect( _pluginList, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( itemSelected( QListViewItem * ) ) );

    if (_scope == Global)
        readGlobalConfig();
    else
        readProjectConfig();
}


PartSelectWidget::~PartSelectWidget()
{}


void PartSelectWidget::readGlobalConfig()
{
    KTrader::OfferList globalOffers = PluginController::pluginServices( "Global" );
    KConfig *config = KGlobal::config();
    config->setGroup("Plugins");

    for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it)
    {
//TODO remove this clumsiness once the Name/GenericName/Comment approach is in all our .desktop files
        QString Comment = (*it)->comment();
        QString GenericName = (*it)->genericName();
        if ( GenericName.isEmpty() )
        {
            GenericName = Comment;
        }
        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), GenericName, Comment );
// ---------

//TODO enable this instead
//        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), (*it)->genericName(), (*it)->comment() );
        item->setOn(config->readBoolEntry((*it)->name(), true));
    }
}


void PartSelectWidget::saveGlobalConfig()
{
    KConfig *config = KGlobal::config();
    config->setGroup("Plugins");

    QListViewItemIterator it( _pluginList );
    while ( it.current() )
    {
        PluginItem * item = static_cast<PluginItem*>( it.current() );
        config->writeEntry( item->name(), item->isOn() );
        ++it;
    }
}


void PartSelectWidget::readProjectConfig()
{
    QStringList ignoreparts = DomUtil::readListEntry(m_projectDom, "/general/ignoreparts", "part");

    KTrader::OfferList localOffers = PluginController::pluginServices( "Project" );
    for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it)
    {
//TODO remove this clumsiness once the Name/GenericName/Comment approach is in all our .desktop files
        QString Comment = (*it)->comment();
        QString GenericName = (*it)->genericName();
        if ( GenericName.isEmpty() )
        {
            GenericName = Comment;
        }
        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), GenericName, Comment );
// --------------

//TODO enable this instead
//        PluginItem *item = new PluginItem( _pluginList, (*it)->name(), (*it)->genericName(), (*it)->comment() );
        item->setOn(!ignoreparts.contains((*it)->name()));
    }
}

void PartSelectWidget::itemSelected( QListViewItem * item )
{
    if ( ! item ) return;

    PluginItem * pitem = static_cast<PluginItem*>( item );
    _pluginDescription->setText( pitem->info() );
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

#include "partselectwidget.moc"
