/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qcombobox.h>
#include <q3valuelist.h>

#include <kservice.h>
#include <kdebug.h>

#include "service.h"


void ServiceComboBox::insertStringList(QComboBox *combo, const Q3ValueList<KService::Ptr> &list,
                            QStringList *names, QStringList *execs)
{
    Q3ValueList<KService::Ptr>::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        combo->insertItem((*it)->comment());
        (*names) << (*it)->desktopEntryName();
        (*execs) << (*it)->exec();
        kdDebug() << "insertStringList item " << (*it)->name() << "," << (*it)->exec() << endl;
    }
}

QString ServiceComboBox::currentText(QComboBox *combo, const QStringList &names)
{
    if (combo->currentItem() == -1)
        return QString::null;
    return names[combo->currentItem()];
}

void ServiceComboBox::setCurrentText(QComboBox *combo, const QString &str, const QStringList &names)
{
    QStringList::ConstIterator it;
    int i = 0;
    for (it = names.begin(); it != names.end(); ++it) {
        if (*it == str) {
            combo->setCurrentItem(i);
            break;
        }
        ++i;
    }
}

int ServiceComboBox::itemForText(const QString &str, const QStringList &names)
{
    QStringList::ConstIterator it;
    int i = 0;
    for (it = names.begin(); it != names.end(); ++it) {
        if (*it == str) {
            return i;
        }
        ++i;
    }
    return 0;
}

QString ServiceComboBox::defaultCompiler()
{
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Pascal'");
    Q3ValueList<KService::Ptr>::ConstIterator it;
    for (it = offers.begin(); it != offers.end(); ++it) {
        if ((*it)->property("X-KDevelop-Default").toBool()) {
            return (*it)->name();;
        }
    }
    return "";
}
