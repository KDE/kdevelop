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

#include <kdebug.h>

#include "domutil.h"


QString DomUtil::readEntry(const QDomDocument &doc, const QString &path)
{
    QStringList l = QStringList::split('/', path);

    QDomElement el = doc.documentElement();
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        el = el.namedItem(*it).toElement();
    }

    return el.firstChild().toText().data();
}


int DomUtil::readIntEntry(const QDomDocument &doc, const QString &path)
{
    return readEntry(doc, path).toInt();
}


bool DomUtil::readBoolEntry(const QDomDocument &doc, const QString &path)
{
    QString s = readEntry(doc, path);
    return s == "TRUE" || s == "true";
}


void DomUtil::writeEntry(QDomDocument &doc, const QString &path, const QString &value)
{
    QStringList l = QStringList::split('/', path);

    QDomElement el = doc.documentElement();
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        QDomElement child = el.namedItem(*it).toElement();
        if (child.isNull()) {
            child = doc.createElement(*it);
            el.appendChild(child);
        }
        el = child;
    }

    while (!el.firstChild().isNull())
        el.removeChild(el.firstChild());
    el.appendChild(doc.createTextNode(value));
}
    

void DomUtil::writeIntEntry(QDomDocument &doc, const QString &path, int value)
{
    writeEntry(doc, path, QString::number(value));
}


void DomUtil::writeBoolEntry(QDomDocument &doc, const QString &path, bool value)
{
    writeEntry(doc, path, value? "true" : "false");
}
