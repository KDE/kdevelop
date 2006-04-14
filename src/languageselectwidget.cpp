/***************************************************************************
 *   Copyright (C) 2003 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qvariant.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listview.h>
#include <q3groupbox.h>
#include <q3hbox.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include <kapplication.h>
#include <kdevplugin.h>
#include "domutil.h"

#include "languageselectwidget.h"
#include "plugincontroller.h"

class LangPluginItem : public Q3CheckListItem
{
public:
    // name - "Name", label - "GenericName", info - "Comment"
    LangPluginItem( Q3ListView * parent, QString const & name, QString const & label,
                QString const & info )
        : Q3CheckListItem( parent, label, Q3CheckListItem::CheckBox),
        _name( name ), _info( info )
    {}

    QString info() { return _info; }
    QString name() { return _name; }

private:
    QString _name;
    QString _info;
};


LanguageSelectWidget::LanguageSelectWidget(QDomDocument &projectDom,
                                   QWidget *parent)
    : QWidget(parent), m_projectDom(projectDom)
{
    init();
}

void LanguageSelectWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    Q3GroupBox * groupBox1 = new Q3GroupBox( i18n("Additional Language Support"), this );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    QVBoxLayout * groupBox1Layout = new QVBoxLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    _currentLanguage = new QLabel( "", groupBox1 );

    _pluginList = new Q3ListView( groupBox1 );
    _pluginList->setResizeMode( Q3ListView::LastColumn );
    _pluginList->addColumn("");
    _pluginList->header()->hide();

    groupBox1Layout->addWidget(_currentLanguage);
    groupBox1Layout->addWidget( _pluginList );
    layout->addWidget( groupBox1 );

    Q3GroupBox * groupBox2 = new Q3GroupBox( i18n("Description"), this );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    QVBoxLayout * groupBox2Layout = new QVBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    _pluginDescription = new QLabel( groupBox2 );
    _pluginDescription->setAlignment( int( Qt::TextWordWrap | Qt::AlignVCenter ) );

    groupBox2Layout->addWidget( _pluginDescription );

    layout->addWidget( groupBox2 );

    connect( _pluginList, SIGNAL( selectionChanged( Q3ListViewItem * ) ), this, SLOT( itemSelected( Q3ListViewItem * ) ) );

    readProjectConfig();
}


LanguageSelectWidget::~LanguageSelectWidget()
{}

void LanguageSelectWidget::readProjectConfig()
{
    KTrader::OfferList languageSupportOffers =
        KTrader::self()->query(QLatin1String("KDevelop/LanguageSupport"),
                               QString::fromLatin1("[X-KDevelop-Version] == %1"
                               ).arg( KDEVELOP_PLUGIN_VERSION ));

    QStringList languages = DomUtil::readListEntry(m_projectDom, "/general/secondaryLanguages", "language");
    QString language = DomUtil::readEntry(m_projectDom, "/general/primarylanguage");
    _currentLanguage->setText(i18n("Primary language is '%1'. Please select additional languages the project might contain.", language));

    for (KTrader::OfferList::ConstIterator it = languageSupportOffers.begin(); it != languageSupportOffers.end(); ++it)
    {
        QString la = (*it)->property("X-KDevelop-Language").toString();
        if (la == language)
            continue;
        LangPluginItem *item = new LangPluginItem( _pluginList, (*it)->property("X-KDevelop-Language").toString(), (*it)->genericName(), (*it)->comment() );
        item->setOn(languages.contains(la));
    }

    Q3ListViewItem * first = _pluginList->firstChild();
    if ( first ) {
        _pluginList->setSelected( first, true );
    }
}

void LanguageSelectWidget::itemSelected( Q3ListViewItem * item )
{
    if ( !item ) return;

    LangPluginItem * pitem = static_cast<LangPluginItem*>( item );
    _pluginDescription->setText( pitem->info() );
}

void LanguageSelectWidget::saveProjectConfig()
{
    QStringList languages;

    Q3ListViewItemIterator it( _pluginList );
    while ( it.current() )
    {
        LangPluginItem * item = static_cast<LangPluginItem*>( it.current() );
        if (item->isOn())
        {
            languages.append( item->name() );
        }
        ++it;
    }

    DomUtil::writeListEntry(m_projectDom, "/general/secondaryLanguages", "language", languages);
}


void LanguageSelectWidget::accept()
{
    saveProjectConfig();
    emit accepted();
}

#include "languageselectwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
