/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   default support: Eray Ozkural (exa)                                   *
 *   additions: John Firebaugh <jfirebaugh@kde.org>                        *
 *              Jakob Simon-Gaarde <jakob@simon-gaarde.dk>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "domutil.h"

#include <kdebug.h>
#include <QStringList>
#include <QFile>
#include <QTextStream>

void DomUtil::makeEmpty( QDomElement& e )
{
    while( !e.firstChild().isNull() )
        e.removeChild( e.firstChild() );
}

QDomElement DomUtil::elementByPath(const QDomDocument &doc, const QString &path)
{
    QStringList l = QStringList::split('/', path);

    QDomElement el;
	if(&doc) el = doc.documentElement();
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

/// @todo consider whether it's okay to accept empty string == default value
/// if not use the below type
///typedef pair<bool,QString> EltInfo;

QString DomUtil::readEntryAux(const QDomDocument &doc, const QString &path)
{
    QDomElement el = elementByPath(doc, path);
    if (el.isNull())
        return QString();
    else
        return el.firstChild().toText().data();
}

int DomUtil::readIntEntry(const QDomDocument &doc, const QString &path, int defaultEntry)
{
    QString entry = readEntryAux(doc, path);
    if (entry.isNull())
      return defaultEntry;
    else
      return entry.toInt();
}


bool DomUtil::readBoolEntry(const QDomDocument &doc, const QString &path, bool defaultEntry)
{
    QString entry = readEntryAux(doc, path);
    if (entry.isNull())
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

QMap<QString, QString> DomUtil::readMapEntry(const QDomDocument &doc, const QString& path)
{
    QMap<QString, QString> map;

    QDomElement el = elementByPath(doc, path);
    QDomElement subEl = el.firstChild().toElement();
    while (!subEl.isNull()) {
        map[subEl.tagName()] = subEl.firstChild().toText().data();
	subEl = subEl.nextSibling().toElement();
    }

    return map;
}

QHash<QString, QString> DomUtil::readHashEntry(const QDomDocument &doc, const QString& path)
{
    QHash<QString, QString> hash;
    QDomElement el = elementByPath(doc, path);
    QDomElement subEl = el.firstChild().toElement();
    while (!subEl.isNull()) {
        hash[subEl.tagName()] = subEl.firstChild().toText().data();
        subEl = subEl.nextSibling().toElement();
    }
    return hash;
}

QDomElement DomUtil::namedChildElement( QDomElement& el, const QString& name )
{
    QDomElement child = el.namedItem( name ).toElement();
    if (child.isNull()) {
        child = el.ownerDocument().createElement( name );
        el.appendChild(child);
    }
    return child;
}


QDomElement DomUtil::createElementByPath(QDomDocument &doc, const QString &path)
{
    QStringList l = QStringList::split('/', path);

    QDomElement el;
      if(&doc) el =  doc.documentElement();
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it)
        el = DomUtil::namedChildElement( el, *it );
        
    while (!el.firstChild().isNull())
        el.removeChild(el.firstChild());

    return el;
}


void DomUtil::writeEntry(QDomDocument &doc, const QString &path, const QString &value)
{
    QDomElement el = createElementByPath(doc, path);
    el.appendChild(doc.createTextNode(value));
}

void DomUtil::writeMapEntry(QDomDocument &doc, const QString &path, const QMap<QString, QString> &map)
{
    QString basePath( path + "/" );
    QMap<QString,QString>::ConstIterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        kdDebug( 9010 ) << "writing " << basePath << ";" << it.key() << ";" << it.value() << endl;
	if( ! it.key().isEmpty() )
            writeEntry(doc, basePath + it.key(), it.value() );
    }
}

void DomUtil::writeHashEntry(QDomDocument &doc, const QString &path, const QHash<QString, QString> &hash)
{
    QString basePath( path + "/" );
    QHash<QString,QString>::ConstIterator it;
    for (it = hash.begin(); it != hash.end(); ++it)
    {
        kdDebug( 9010 ) << "writing " << basePath << ";" << it.key() << ";" << it.value() << endl;
	if( ! it.key().isEmpty() )
            writeEntry(doc, basePath + it.key(), it.value() );
    }
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

DomPath DomUtil::resolvPathStringExt(const QString pathstring)
{
    // parse path
    int i;
    QStringList pathParts = QStringList::split('/',pathstring);
    DomPath dompath;
    for (i=0; i<pathParts.count(); i++)
    {
      QStringList pathElemParts = QStringList::split('|',pathParts[i],TRUE);
      DomPathElement dompathelem;
      dompathelem.tagName = pathElemParts[0].simplifyWhiteSpace();
      if (pathElemParts.count()>1)
      {
        // handle attributes
        QStringList attrParts = QStringList::split(';',pathElemParts[1]);
        for (int j=0; j<attrParts.count(); j++)
        {
          QStringList attribSet = QStringList::split('=',attrParts[j]);
          if (attribSet.count()<2)
            continue;
          DomAttribute domattribute;
          domattribute.name = attribSet[0].simplifyWhiteSpace();
          domattribute.value = attribSet[1].simplifyWhiteSpace();
          dompathelem.attribute.append(domattribute);
        }
      }
      if (pathElemParts.count()>2)
	dompathelem.matchNumber = pathElemParts[2].toInt();
      else
	dompathelem.matchNumber = 0; // or else the first
      dompath.append(dompathelem);
    }
    return dompath;
}


#define rightchild !wrongchild

QDomElement DomUtil::elementByPathExt(QDomDocument &doc, const QString &pathstring)
{
  DomPath dompath = resolvPathStringExt(pathstring);
  QDomElement elem = doc.documentElement();
  QDomNodeList children;
  QDomElement nextElem = elem;
  for (int j=0; j<dompath.count(); j++)
  {
    children = nextElem.childNodes();
    DomPathElement dompathelement= dompath[j];
    bool wrongchild = false;
    int matchCount = 0;
    for (int i=0; i<children.count(); i++)
    {
      wrongchild = false;
      QDomElement child = children.item(i).toElement();
      QString tag = child.tagName();
      tag = dompathelement.tagName;
      if (child.tagName() == dompathelement.tagName)
      {
        for (int k=0; k<dompathelement.attribute.count(); k++)
        {
          DomAttribute domattribute = dompathelement.attribute[k];
          QDomAttr domattr = child.attributeNode(domattribute.name);
          if (domattr.isNull() ||
	      domattr.value() != domattribute.value)
          {
            wrongchild = true;
            break;
          }
        }      
      }
      else
        wrongchild=true;
      if (rightchild)
      {
        if (dompathelement.matchNumber == matchCount++)
        {
          nextElem = child;
          break;
        }
      }
    }
    if (wrongchild)
    {
      QDomElement nullDummy;
      nullDummy.clear();
      return nullDummy;
    }
  }
  return nextElem;
}


bool DomUtil::openDOMFile(QDomDocument &doc, QString filename)
{
  QFile file( filename );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;
  if ( !doc.setContent( &file ) ) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool DomUtil::saveDOMFile(QDomDocument &doc, QString filename)
{
  QFile file( filename );
  if ( !file.open( QIODevice::ReadWrite | QIODevice::Truncate ) )
    return false;
  QTextStream t( &file );
  t << doc.toString();
  file.close();
  return true;
}

bool DomUtil::removeTextNodes(QDomDocument doc,QString pathExt)
{
  QDomElement elem = elementByPathExt(doc,pathExt);
  if (elem.isNull())
    return false;
  QDomNodeList children = elem.childNodes();
  for (int i=0;i<children.count();i++)
    if (children.item(i).isText())
      elem.removeChild(children.item(i));
  return true;
}


bool DomUtil::appendText(QDomDocument doc, QString pathExt, QString text)
{
  QDomElement elem = elementByPathExt(doc,pathExt);
  if (elem.isNull())
    return false;
  elem.appendChild(doc.createTextNode(text));
  return true;
}


bool DomUtil::replaceText(QDomDocument doc, QString pathExt, QString text)
{
  if (removeTextNodes(doc,pathExt) &&
      appendText(doc,pathExt,text))
    return true;
  else
    return false;
}
