/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "servicecombobox.h"


ServiceComboBox::ServiceComboBox(const QValueList<KService::Ptr> &list,
                                 QWidget *parent, const char *name)
    : QComboBox(parent, name)
{
    QValueList<KService::Ptr>::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        insertItem((*it)->comment());
        names << (*it)->name();
    }
}

#if 0
ServiceComboBox::ServiceComboBox(const QValueList<KService::Ptr> &list,
                                 const QString &propertyName,
                                 QWidget *parent, const char *name)
    : QComboBox(parent, name)
{
    QValueList<KService::Ptr>::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        insertItem((*it)->property(propertyName).toString());
        names << (*it)->name();
    }
}
#endif

ServiceComboBox::~ServiceComboBox()
{}


QString ServiceComboBox::currentText() const
{
    if (currentItem() == -1)
        return QString::null;
    return names[currentItem()];
}


void ServiceComboBox::setCurrentText(const QString &str)
{
    for (int i=0; i < count(); ++i)
        if (text(i) == str) {
            setCurrentItem(i);
            break;
        }
}
