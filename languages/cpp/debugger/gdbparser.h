/***************************************************************************
    begin                : Tue Aug 17 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GDBPARSER_H_
#define _GDBPARSER_H_

#include "variablewidget.h"

namespace GDBDebugger
{

class GDBParser
{
public:
    /** Strips gdb decorations from value of a single variable
        and sets text of 'item' appropriately.
        For values of composite types also calls item->setCache.
        Does not recurse into composite types and does not creates
        child items of 'item' (that will be done in item->setOpen,
        when that's method is called.

        "Decorations" are {} around arrays and structures and value
        type that is printed before value itself in some cases.
    */
    void parseValue(TrimmableItem *item, const char *buf);

    /** Parses gdb-provided value 'buf' of a composite type
        (struct/array), and assigns proper values to children
        of 'parent'. As a special hack, the output from
        "info locals" and "info args" can be passed to this
        method, as the output looks like just value of some
        imaginary 'struct local_variables'.
        The value should contain all the decorations from gdb
        (opening braces of arrays, and so on).
    */
    void parseCompositeValue(TrimmableItem* parent, const char* buf);

    DataType  determineType(const char *buf) const;

    const char *skipString(const char *buf) const;
    const char *skipQuotes(const char *buf, char quote) const;
    const char *skipDelim(const char *buf, char open, char close) const;

    static GDBParser *getGDBParser();
    static void destroy();

private:
    TrimmableItem *getItem(TrimmableItem *parent, DataType itemType,
                           const QString &varName, bool requested);

    void parseArray(TrimmableItem *parent, const char *buf);

    const char *skipTokenEnd(const char *buf) const;
    const char *skipTokenValue(const char *buf) const;
    const char *skipNextTokenStart(const char *buf) const;

    QString getName(const char **buf);
    /** Assuming 'buf' points to a value, return a pointer
        to the position right after the value.
    */
    QCString getValue(const char **buf);
    QCString GDBParser::undecorateValue(DataType type, const QCString& s);
    void setItem(TrimmableItem *parent, const QString &varName, DataType dataType,
                 const QCString &value, bool requested);

protected:
    GDBParser();
    ~GDBParser();
    static GDBParser *GDBParser_;
};

}

#endif
