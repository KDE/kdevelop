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


#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include "domutil.h"

#include "partselectwidget.h"
#include "plugincontroller.h"


PartSelectWidget::PartSelectWidget(QDomDocument &projectDom,
                                   QWidget *parent, const char *name)
    : QWidget(parent, name), m_projectDom(projectDom), scope(Project)
{
    init();
}


PartSelectWidget::PartSelectWidget(QWidget *parent, const char *name)
    : QWidget(parent, name), m_projectDom(QDomDocument()), scope(Global)
{
    init();
}


void PartSelectWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QString text = (scope==Global)?
        i18n("Plugins to load at startup:") :
        i18n("Plugins to load for this project:");
    QLabel *label = new QLabel(text, this);

    lv = new QListView(this);
    lv->setSorting(-1);
    lv->addColumn("");
    lv->header()->hide();
    
    layout->addWidget(label);
    layout->addWidget(lv);

    if (scope == Global)
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
    
    for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it) {
        QCheckListItem *item = new QCheckListItem(lv, (*it)->comment(), QCheckListItem::CheckBox);
        names.prepend((*it)->name());
        item->setOn(config->readBoolEntry((*it)->name(), true));
    }    
}


void PartSelectWidget::saveGlobalConfig()
{
    KConfig *config = KGlobal::config();
    config->setGroup("Plugins");

    QListViewItemIterator it1(lv);
    QStringList::Iterator it2;
    for (it2 = names.begin();
         it1.current() && it2 != names.end();
         ++it1, ++it2) {
        QCheckListItem *item = static_cast<QCheckListItem*>(it1.current());
        config->writeEntry(*it2, item->isOn());
    }
}


void PartSelectWidget::readProjectConfig()
{
    QStringList ignoreparts = DomUtil::readListEntry(m_projectDom, "/general/ignoreparts", "part");

    KTrader::OfferList localOffers = PluginController::pluginServices( "Project" );
    for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it) {
        QCheckListItem *item = new QCheckListItem(lv, (*it)->comment(), QCheckListItem::CheckBox);
        names.prepend((*it)->name());
        item->setOn(!ignoreparts.contains((*it)->name()));
    }
}


void PartSelectWidget::saveProjectConfig()
{
    QStringList ignoreparts;
    
    QListViewItemIterator lvit(lv);
    QStringList::Iterator it;
    for (it = names.begin();
         lvit.current() && it != names.end();
         ++lvit, ++it) {
        QCheckListItem *item = static_cast<QCheckListItem*>(lvit.current());
        if (!item->isOn()) {
            ignoreparts.append(*it);
            kdDebug(9000) << "Appending " << (*it) << endl;
        }
    }
    
    DomUtil::writeListEntry(m_projectDom, "/general/ignoreparts", "part", ignoreparts);
    kdDebug(9000) << "xml:" << m_projectDom.toString() << endl;
}


void PartSelectWidget::accept()
{
    if (scope == Global)
        saveGlobalConfig();
    else
        saveProjectConfig();
}

#include "partselectwidget.moc"
