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
#include <kapplication.h>
#include <kdevplugin.h>
#include "domutil.h"

#include "languageselectwidget.h"
#include "plugincontroller.h"

class LangPluginItem : public QCheckListItem
{
public:
    // name - "Name", label - "GenericName", info - "Comment"
    LangPluginItem( QListView * parent, QString const & name, QString const & label,
                QString const & info )
        : QCheckListItem( parent, label, QCheckListItem::CheckBox),
        _name( name ), _info( info )
    {}

    QString info() { return _info; }
    QString name() { return _name; }

private:
    QString _name;
    QString _info;
};


LanguageSelectWidget::LanguageSelectWidget(QDomDocument &projectDom,
                                   QWidget *parent, const char *name)
    : QWidget(parent, name), m_projectDom(projectDom)
{
    init();
}

void LanguageSelectWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox * groupBox1 = new QGroupBox( i18n("Additional Language Support"), this );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    QVBoxLayout * groupBox1Layout = new QVBoxLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    _currentLanguage = new QLabel( "", groupBox1 );

    _pluginList = new QListView( groupBox1 );
    _pluginList->setResizeMode( QListView::LastColumn );
    _pluginList->addColumn("");
    _pluginList->header()->hide();

    groupBox1Layout->addWidget(_currentLanguage);
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

    groupBox2Layout->addWidget( _pluginDescription );

    layout->addWidget( groupBox2 );

    connect( _pluginList, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( itemSelected( QListViewItem * ) ) );

    readProjectConfig();
}


LanguageSelectWidget::~LanguageSelectWidget()
{}

void LanguageSelectWidget::readProjectConfig()
{
    KTrader::OfferList languageSupportOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                               QString::fromLatin1("[X-KDevelop-Version] == %1"
                               ).arg( KDEVELOP_PLUGIN_VERSION ));

    QStringList languages = DomUtil::readListEntry(m_projectDom, "/general/secondaryLanguages", "language");
    QString language = DomUtil::readEntry(m_projectDom, "/general/primarylanguage");
    _currentLanguage->setText(i18n("Primary language is '%1'. Please select additional languages the project might contain.").arg(language));

    for (KTrader::OfferList::ConstIterator it = languageSupportOffers.begin(); it != languageSupportOffers.end(); ++it)
    {
        QString la = (*it)->property("X-KDevelop-Language").toString();
        if (la == language)
            continue;
        LangPluginItem *item = new LangPluginItem( _pluginList, (*it)->property("X-KDevelop-Language").toString(), (*it)->genericName(), (*it)->comment() );
        item->setOn(languages.contains(la));
    }

    QListViewItem * first = _pluginList->firstChild();
    if ( first ) {
        _pluginList->setSelected( first, true );
    }
}

void LanguageSelectWidget::itemSelected( QListViewItem * item )
{
    if ( !item ) return;

    LangPluginItem * pitem = static_cast<LangPluginItem*>( item );
    _pluginDescription->setText( pitem->info() );
}

void LanguageSelectWidget::saveProjectConfig()
{
    QStringList languages;

    QListViewItemIterator it( _pluginList );
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
