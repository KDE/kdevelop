/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   jakob@simon-gaarde.dk                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOMUTIL_H_
#define _DOMUTIL_H_

#include <qdom.h>
#include <qpair.h>
#include <qstringlist.h>
#include <q3valuelist.h>
#include <qmap.h>

/**
@file domutil.h
Utility functions to operate on %DOM.
*/

struct DomAttribute
{
  QString name;
  QString value;
};

struct DomPathElement
{
  QString tagName;
  QList<DomAttribute> attribute;
  int matchNumber;  // for use when more than one element matches the path
};

typedef QList<DomPathElement> DomPath;

/**
 * Utility class for conveniently accessing data in a %DOM tree.
 */
class DomUtil
{
public:
    typedef QPair<QString, QString> Pair;
    typedef QList<Pair> PairList;
    /**
     * Remove all child elements from a given element.
     */
    static void makeEmpty( QDomElement& );
    /**
     * Reads a string entry.
     */
    static QString readEntry(const QDomDocument &doc, const QString &path, const QString &defaultEntry = QString());
    /**
     * Reads a number entry.
     */
    static int readIntEntry(const QDomDocument &doc, const QString &path, int defaultEntry = 0);
    /**
     * Reads a boolean entry. The strings "true" and "TRUE" are interpreted
     * as true, all other as false.
     */
    static bool readBoolEntry(const QDomDocument &doc, const QString &path, bool defaultEntry = false);
    /**
     * Reads a list entry. See writeListEntry().
     */
    static QStringList readListEntry(const QDomDocument &doc, const QString &path, const QString &tag);
    /**
     * Reads a list of string pairs. See writePairListEntry().
     */
    static PairList readPairListEntry(const QDomDocument &doc, const QString &path, const QString &tag,
                                      const QString &firstAttr, const QString &secondAttr);
    /**
     * Reads a string to string map. See writeMapEntry()
     */
    static QMap<QString, QString> readMapEntry(const QDomDocument &doc, const QString &path);
    /**
     * Retrieves an element by path, return null if any item along
     * the path does not exist.
     */
    static QDomElement elementByPath( const QDomDocument& doc, const QString& path );
    /**
     * Retrieves an element by path, creating the necessary nodes.
     */
    static QDomElement createElementByPath( QDomDocument& doc, const QString& path );
    /**
     * Retrieves a child element, creating it if it does not exist.
     * The return value is guaranteed to be non isNull()
     */
    static QDomElement namedChildElement( QDomElement& el, const QString& name );
    /**
      Writes a string entry. For example,
      \verbatim
        <code>
          writeEntry(doc, "/general/special", "foo");
        </code>
      \endverbatim creates the %DOM fragment: \verbatim
        <code>
          <general><special>foo</special></general>
        </code>
      \endverbatim
     */
    static void writeEntry(QDomDocument &doc, const QString &path, const QString &value);
    /**
     * Writes a number entry.
     */
    static void writeIntEntry(QDomDocument &doc, const QString &path, int value);
    /**
     * Writes a boolean entry. Booleans are stored as "true", "false" resp.
     */
    static void writeBoolEntry(QDomDocument &doc, const QString &path, bool value);
    /**
      Writes a string list element. The list elements are separated by tag. For example,
      \verbatim
        <code>
          QStringList l; l << "one" << "two";
          writeListEntry(doc, "/general/special", "el", l);
        </code>
      \endverbatim creates the %DOM fragment: \verbatim
        <code>
          <general><special><el>one</el><el>two</el></special></general>
        </code>
      \endverbatim
     */
    static void writeListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                               const QStringList &value);
    /**
      Writes a list of string pairs. The list elements are stored in the attributes
      firstAttr and secondAttr of elements named tag. For example,
      \verbatim
        <code>
          DomUtil::PairList l;
          l << DomUtil::StringPair("one", "1");
          l << DomUtil::StringPair("two", "2");
          writePairListEntry(doc, "/general/special", "el", "first", "second", l);
        </code>
      \endverbatim creates the %DOM fragment: \verbatim
        <code>
          <general><special>
            <el first="one" second="1"/>
            <el first="two" second="2"/>
          </special></general>
        </code>
      \endverbatim
     */
    static void writePairListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                                   const QString &firstAttr, const QString &secondAttr,
                                   const PairList &value);
    /**
     * Writes a string to string map. This map is stored in a way, that it can be read with
     * readMapEntry() and readEntry()
     */
    static void writeMapEntry(QDomDocument &doc, const QString& path, const QMap<QString,QString> &map);

    /**
     * Resolves an extended path
     * Extended path format:
     * pathpart: tag[|attr1=value[;attr2=value;..][|matchNumber]]
     * where matchNumber is zero-based
     * path: pathpart[/pathpart/..]
     */
    static DomPath resolvPathStringExt(const QString pathstring);

    /**
      Retrieve an element specified with extended path
      examples:

       - 1: "widget|class=QDialog/property|name=geometry"
         or "widget|class=QDialog/property||1"
       - 2: "widget/property|name=caption/string"
         or "widget/property||2/string"
       .
      \verbatim
        <widget class="QDialog">
          <property name="name">
              <cstring>KdevFormName</cstring>
          </property>
          <property name="geometry">       <-- 1. reaches this node
              <rect>
                  <x>0</x>
                  <y>0</y>
                  <width>600</width>
                  <height>480</height>
              </rect>
          </property>
          <property name="caption">
              <string>KdevFormCaption</string>     <-- 2. reaches this node
          </property>
        </widget>
      \endverbatim
     */
    static QDomElement elementByPathExt(QDomDocument &doc, const QString &pathstring);

    /**
    * Open file - filename - and set setContents of doc
    */
    static bool openDOMFile(QDomDocument &doc, QString filename);

    /**
    * Store contents of doc in file - filename. Existing file will be truncated!
    */
    static bool saveDOMFile(QDomDocument &doc, QString filename);

    /**
    * Remove all child text nodes of parent described in pathExt
    */
    static bool removeTextNodes(QDomDocument doc,QString pathExt);

    /**
    * Add child text node to parent described in pathExt
    */
    static bool appendText(QDomDocument doc, QString pathExt, QString text);

    /**
    * Replace all chilt text nodes of parent described in pathExt with one new.
    */
    static bool replaceText(QDomDocument doc, QString pathExt, QString text);

private:
    static QString readEntryAux(const QDomDocument &doc, const QString &path);
};

#endif
