/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   default support: Eray Ozkural (exa)                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "domutil.h"

#include <kdebug.h>
#include <qstringlist.h>


static QDomElement elementByPath(const QDomDocument &doc, const QString &path)
{
    QStringList l = QStringList::split('/', path);

    QDomElement el = doc.documentElement();
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        el = el.namedItem(*it).toElement();
    }

    return el;
}


QString DomUtil::readEntry(const QDomDocument &doc, const QString &path, const QString &defaultEntry)
{
    QDomElement el = elementByPath(doc, path);
    if (el.isNull())
        return defaultEntry;
    else
        return el.firstChild().toText().data();
}

// TODO: consider whether it's okay to accept empty string == default value
// if not use the below type
//typedef pair<bool,QString> EltInfo;

QString DomUtil::readEntryAux(const QDomDocument &doc, const QString &path)
{
    QDomElement el = elementByPath(doc, path);
    if (el.isNull())
        return QString::null;
    else
        return el.firstChild().toText().data();
}

int DomUtil::readIntEntry(const QDomDocument &doc, const QString &path, int defaultEntry)
{
    QString entry = readEntryAux(doc, path);
    if (entry==QString::null)
      return defaultEntry;
    else
      return entry.toInt();
}


bool DomUtil::readBoolEntry(const QDomDocument &doc, const QString &path, bool defaultEntry)
{
    QString entry = readEntryAux(doc, path);
    if (entry==QString::null)
      return defaultEntry;
    else
      return entry == "TRUE" || entry == "true";
}


QStringList DomUtil::readListEntry(const QDomDocument &doc, const QString &path, const QString &tag)
{
    QStringList list;
    
    QDomElement el = elementByPath(doc, path);
    QDomElement subEl = el.firstChild().toElement();
    while (!subEl.isNull()) {
        if (subEl.tagName() == tag)
            list << subEl.firstChild().toText().data();
        subEl = subEl.nextSibling().toElement();
    }

    return list;
}


DomUtil::PairList DomUtil::readPairListEntry(const QDomDocument &doc, const QString &path, const QString &tag,
                                             const QString &firstAttr, const QString &secondAttr)
{
    PairList list;
    
    QDomElement el = elementByPath(doc, path);
    QDomElement subEl = el.firstChild().toElement();
    while (!subEl.isNull()) {
        if (subEl.tagName() == tag) {
            QString first = subEl.attribute(firstAttr);
            QString second = subEl.attribute(secondAttr);
            list << Pair(first, second);
        }
        subEl = subEl.nextSibling().toElement();
    }
    
    return list;
}


static QDomElement createElementByPath(QDomDocument &doc, const QString &path)
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

    return el;
}


void DomUtil::writeEntry(QDomDocument &doc, const QString &path, const QString &value)
{
    QDomElement el = createElementByPath(doc, path);
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


void DomUtil::writeListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                             const QStringList &value)
{
    QDomElement el = createElementByPath(doc, path);

    QStringList::ConstIterator it;
    for (it = value.begin(); it != value.end(); ++it) {
        QDomElement subEl = doc.createElement(tag);
        subEl.appendChild(doc.createTextNode(*it));
        el.appendChild(subEl);
    }
}


void DomUtil::writePairListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                                 const QString &firstAttr, const QString &secondAttr,
                                 const PairList &value)
{
    QDomElement el = createElementByPath(doc, path);

    PairList::ConstIterator it;
    for (it = value.begin(); it != value.end(); ++it) {
        QDomElement subEl = doc.createElement(tag);
        subEl.setAttribute(firstAttr, (*it).first);
        subEl.setAttribute(secondAttr, (*it).second);
        el.appendChild(subEl);
    }
}
