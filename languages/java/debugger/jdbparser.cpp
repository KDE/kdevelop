// **************************************************************************
//                          jdbparser.cpp  -  description
//                             -------------------
//    begin                : Tue Aug 17 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************

// **************************************************************************
//                                                                          *
//    This program is free software; you can redistribute it and/or modify  *
//    it under the terms of the GNU General Public License as published by  *
//    the Free Software Foundation; either version 2 of the License, or     *
//    (at your option) any later version.                                   *
//                                                                          *
// **************************************************************************

#include "jdbparser.h"
#include "variablewidget.h"

#include <qregexp.h>

#include <ctype.h>
#include <stdlib.h>

namespace JAVADebugger
{

// **************************************************************************
// **************************************************************************
// **************************************************************************

JDBParser::JDBParser()
{
}

// **************************************************************************

JDBParser::~JDBParser()
{
}

// **************************************************************************

void JDBParser::parseData(TrimmableItem *parent, char *buf,
                          bool requested, bool params)
{
    static const char *unknown = "?";

    Q_ASSERT(parent);
    if (!buf)
        return;

    if (parent->getDataType() == typeArray) {
        parseArray(parent, buf);
        return;
    }

    if (requested && !*buf)
        buf = (char*)unknown;

    while (*buf) {
        QString varName = "";
        DataType dataType = determineType(buf);

        if (dataType == typeName) {
            varName = getName(&buf);
            dataType = determineType(buf);
        }

        QCString value = getValue(&buf, requested);
        setItem(parent, varName, dataType, value, requested, params);
    }
}

// **************************************************************************

void JDBParser::parseArray(TrimmableItem *parent, char *buf)
{
    QString elementRoot = parent->getName() + "[%1]";
    int idx = 0;
    while (*buf) {
            buf = skipNextTokenStart(buf);
            if (!*buf)
                return;

            DataType dataType = determineType(buf);
            QCString value = getValue(&buf, false);
            QString varName = elementRoot.arg(idx);
            setItem(parent, varName, dataType, value, false, false);

            int pos = value.find(" <repeats", 0);
            if (pos > -1) {
                if (int i = atoi(value.data()+pos+10))
                    idx += (i-1);
            }

            idx++;
    }
}

// **************************************************************************

QString JDBParser::getName(char **buf)
{
    char *start = skipNextTokenStart(*buf);
    if (*start) {
        *buf = skipTokenValue(start);
        return QCString(start, *buf - start + 1);
    } else
        *buf = start;

    return QString();
}

// **************************************************************************

QCString JDBParser::getValue(char **buf, bool requested)
{
    char *start = skipNextTokenStart(*buf);
    *buf = skipTokenValue(start);

    if (*start == '{')
        return QCString(start+1, *buf - start -1);

    QCString value(start, *buf - start + 1);

    // QT2.x string handling
    // A very bad hack alert!
    if (requested)
        return value.replace( QRegExp("\\\\000"), "" );

    return value;
}

// ***************************************************************************

TrimmableItem *JDBParser::getItem(TrimmableItem *parent, DataType dataType,
                                  const QString &varName, bool requested)
{
    if (requested)
        return parent;

    if (varName.isEmpty()) {
        if (parent->getDataType() == typeReference)
            return parent;

        return 0;
    }

    return parent->findMatch(varName, dataType);
}

// **************************************************************************

void JDBParser::setItem(TrimmableItem *parent, const QString &varName,
                        DataType dataType, const QCString &value,
                        bool requested, bool)
{
    TrimmableItem *item = getItem(parent, dataType, varName, requested);
    if (!item) {
        if (varName.isEmpty())
            return;

        item = new VarItem(parent, varName, dataType);
    } else {
        // Don't update a "this" item because it'll alwasy stay red because the local
        // this looks different than the param this.
        // @todo - this is not good code. We should check to see if the address is the
        // same between params and locals before ignoring the param.
        //    if (params && varName == "this")
        //      return;
    }

    switch (dataType) {
    case typePointer:
        item->setText(ValueCol, value);
        item->setExpandable(varName != "_vptr.");
        break;

    case typeStruct:
    case typeArray:
        item->setCache(value);
        break;

    case typeReference:
        {
            int pos;
            if ((pos = value.find(':', 0)) != -1) {
                QCString rhs((value.mid(pos+2, value.length()).data()));
                if (determineType(rhs.data()) != typeValue) {
                    item->setCache(rhs);
                    item->setText(ValueCol, value.left(pos));
                    break;
                }
            }
            item->setText(ValueCol, value);
            item->setExpandable(!value.isEmpty() && (value[0] == '@'));
            break;
        }

    case typeValue:
        item->setText(ValueCol, value);
        break;

    default:
        break;
    }
}

// **************************************************************************

DataType JDBParser::determineType(char *buf) const
{
    if (!buf || !*(buf= skipNextTokenStart(buf)))
        return typeUnknown;

    // A reference, probably from a parameter value.
    if (*buf == '@')
        return typeReference;

    // Structures and arrays - (but which one is which?)
    // {void (void)} 0x804a944 <__builtin_new+41> - this is a fn pointer
    // (void (*)(void)) 0x804a944 <f(E *, char)>  - so is this - ugly!!!
    if (*buf == '{') {
        if (strncmp(buf, "{{", 2) == 0)
            return typeArray;

        if (strncmp(buf, "{<No data fields>}", 18) == 0)
            return typeValue;

        buf++;
        while (*buf) {
            switch (*buf) {
            case '=':
                return typeStruct;
            case '"':
                buf = skipString(buf);
                break;
            case '\'':
                buf = skipQuotes(buf, '\'');
                break;
            case ',':
                if (*(buf-1) == '}')
                    Q_ASSERT(false);
                return typeArray;
            case '}':
                if (*(buf+1) == ',' || *(buf+1) == '\n' || !*(buf+1))
                    return typeArray;                     // Hmm a single element array??
                if (strncmp(buf+1, " 0x", 3) == 0)
                    return typePointer;                   // What about references?
                return typeUnknown;                     // very odd?
            case '(':
                buf = skipDelim(buf, '(', ')');
                break;
            case '<':
                buf = skipDelim(buf, '<', '>');
                break;
            default:
                buf++;
                break;
            }
        }
        return typeUnknown;
    }

    // some sort of address. We need to sort out if we have
    // a 0x888888 "this is a char*" type which we'll term a value
    // or whether we just have an address
    if (strncmp(buf, "0x", 2) == 0) {
        while (*buf) {
            if (!isspace(*buf))
                buf++;
            else if (*(buf+1) == '\"')
                return typeValue;
            else
                break;
        }

        return typePointer;
    }

    // Pointers and references - references are a bit odd
    // and cause JDB to fail to produce all the local data
    // if they haven't been initialised. but that's not our problem!!
    // (void (*)(void)) 0x804a944 <f(E *, char)> - this is a fn pointer
    if (*buf == '(') {
        buf = skipDelim(buf, '(', ')');
        switch (*(buf-2)) {
        case '*':
            return typePointer;
        case '&':
            return typeReference;
        default:
            return typeUnknown;
        }
    }

    buf = skipTokenValue(buf);
    if ((strncmp(buf, " = ", 3) == 0) || (*buf == '='))
        return typeName;

    return typeValue;
}

// **************************************************************************

char *JDBParser::skipString(char *buf) const
{
    if (buf && *buf == '\"') {
        buf = skipQuotes(buf, *buf);
        while (*buf) {
            if ((strncmp(buf, ", \"", 3) == 0) || (strncmp(buf, ", '", 3) == 0))
                buf = skipQuotes(buf+2, *(buf+2));
            else if (strncmp(buf, " <", 2) == 0)         // take care of <repeats
                buf = skipDelim(buf+1, '<', '>');
            else
                break;
        }

        // If the string is long then it's chopped and has ... after it.
        while (*buf && *buf == '.')
            buf++;
    }

    return buf;
}

// ***************************************************************************

char *JDBParser::skipQuotes(char *buf, char quotes) const
{
    if (buf && *buf == quotes) {
        buf++;

        while (*buf) {
            if (*buf == '\\')
                buf++;             // skips \" or \' problems
            else if (*buf == quotes)
                return buf+1;

            buf++;
        }
    }

    return buf;
}

// **************************************************************************

char *JDBParser::skipDelim(char *buf, char open, char close) const
{
    if (buf && *buf == open) {
        buf++;

        while (*buf) {
            if (*buf == open)
                buf = skipDelim(buf, open, close);
            else if (*buf == close)
                return buf+1;
            else if (*buf == '\"')
                buf = skipString(buf);
            else if (*buf == '\'')
                buf = skipQuotes(buf, *buf);
            else if (*buf)
                buf++;
        }
    }
    return buf;
}

// **************************************************************************

char *JDBParser::skipTokenValue(char *buf) const
{
    if (buf) {
        while (true) {
            buf = skipTokenEnd(buf);

            char *end = buf;
            while (*end && isspace(*end) && *end != '\n')
                end++;

            if (*end == 0 || *end == ',' || *end == '\n' || *end == '=' || *end == '}')
                break;

            if (buf == end)
                break;

            buf = end;
        }
    }

    return buf;
}

// **************************************************************************

char *JDBParser::skipTokenEnd(char *buf) const
{
    if (buf) {
        switch (*buf) {
        case '"':
            return skipString(buf);
        case '\'':
            return skipQuotes(buf, *buf);
        case '{':
            return skipDelim(buf, '{', '}');
        case '<':
            return skipDelim(buf, '<', '>');
        case '(':
            return skipDelim(buf, '(', ')');
        }

        while (*buf && !isspace(*buf) && *buf != ',' && *buf != '}' && *buf != '=')
            buf++;
    }

    return buf;
}

// **************************************************************************

char *JDBParser::skipNextTokenStart(char *buf) const
{
    if (buf)
        while (*buf && (isspace(*buf) || *buf == ',' || *buf == '}' || *buf == '='))
            buf++;

    return buf;
}

}

// **************************************************************************
// **************************************************************************
// **************************************************************************
