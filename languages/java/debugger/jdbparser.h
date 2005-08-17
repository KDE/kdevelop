/***************************************************************************
                          jdbparser.h  -  description
                             -------------------
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

#ifndef _JDBPARSER_H_
#define _JDBPARSER_H_

#include "variablewidget.h"
//Added by qt3to4:
#include <Q3CString>

namespace JAVADebugger
{

/**
 * @author John Birch
 */
class JDBParser
{
public:
    JDBParser();
    ~JDBParser();

    void parseData(TrimmableItem *parent, char *buf,
                   bool requested, bool params);
    DataType  determineType(char *buf) const;

    char *skipString(char *buf) const;
    char *skipQuotes(char *buf, char quote) const;
    char *skipDelim(char *buf, char open, char close) const;

private:
    TrimmableItem *getItem(TrimmableItem *parent, DataType itemType,
                           const QString &varName, bool requested);

    void parseArray(TrimmableItem *parent, char *buf);

    char *skipTokenEnd(char *buf) const;
    char *skipTokenValue(char *buf) const;
    char *skipNextTokenStart(char *buf) const;

    QString getName(char **buf);
    Q3CString getValue(char **buf, bool requested);
    void setItem(TrimmableItem *parent, const QString &varName, DataType dataType,
                 const Q3CString &value, bool requested, bool params);
};

}

#endif
