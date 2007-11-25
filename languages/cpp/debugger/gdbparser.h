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
    DataType  determineType(const char *buf) const;
    QString undecorateValue(const QString& s);

    const char *skipString(const char *buf) const;
    const char *skipQuotes(const char *buf, char quote) const;
    const char *skipDelim(const char *buf, char open, char close) const;

    static GDBParser *getGDBParser();
    static void destroy();

private:
    void parseArray(TrimmableItem *parent, const char *buf);

    const char *skipTokenEnd(const char *buf) const;
    const char *skipTokenValue(const char *buf) const;
    const char *skipNextTokenStart(const char *buf) const;

    QString getName(const char **buf);
    /** Assuming 'buf' points to a value, return a pointer
        to the position right after the value.
    */
    QString getValue(const char **buf);
    QString undecorateValue(DataType type, const QString& s);

protected:
    GDBParser();
    ~GDBParser();
    static GDBParser *GDBParser_;
};

}

#endif
