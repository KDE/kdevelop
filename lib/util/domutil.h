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

#ifndef _DOMUTIL_H_
#define _DOMUTIL_H_

#include <qdom.h>
#include <qpair.h>
#include <qstringlist.h>
#include <qvaluelist.h>


/**
 * Utility class for conveniently accessing data in a DOM tree.
 */
class DomUtil
{
public:
    typedef QPair<QString, QString> Pair;
    typedef QValueList<Pair> PairList;
    /**
     * Remove all child elements from a given element.
     */
    static void makeEmpty( QDomElement& );
    /**
     * Reads a string entry.
     */
    static QString readEntry(const QDomDocument &doc, const QString &path, const QString &defaultEntry = QString::null);
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
     * Writes a string entry. For example,
     * <code>
     *   writeEntry(doc, "/general/special", "foo");
     * </code>
     * creates the DOM fragment
     * <code>
     *   <general><special>foo</special></general>
     * </code>
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
     * Writes a string list element. The list elements are separated by tag. For example,
     * <code>
     *   QStringList l; l << "one" << "two";
     *   writeEntry(doc, "/general/special", "el", l);
     * </code>
     * creates the DOM fragment
     * <code>
     *   <general><special><el>one</el><el>two</el></special></general>
     * </code>
     */
    static void writeListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                               const QStringList &value);
    /**
     * Writes a list of string pairs. The list elements are stored in the attributes
     * firstAttr and secondAttr of elements named tag. For example,
     * <code>
     *   DomUtil::StringPairList l;
     *   l << DomUtil::StringPair("one", "1");
     *   l << DomUtil::StringPair("two", "2");
     *   writeEntry(doc, "/general/special", "el", "first", "second", l);
     * </code>
     * creates the DOM fragment
     * <code>
     *   <general><special>
     *     <el first="one" second="1"/>
     *     <el first="two" second="2"/>
     *   </special></general>
     * </code>
     */
    static void writePairListEntry(QDomDocument &doc, const QString &path, const QString &tag,
                                   const QString &firstAttr, const QString &secondAttr,
                                   const PairList &value);
private:
    static QString readEntryAux(const QDomDocument &doc, const QString &path);
};

#endif
