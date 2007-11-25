// **************************************************************************
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

#include "gdbparser.h"
#include "variablewidget.h"
#include <kdebug.h>

#include <qregexp.h>

#include <ctype.h>
#include <stdlib.h>

namespace GDBDebugger
{

// **************************************************************************
// **************************************************************************
// **************************************************************************

GDBParser *GDBParser::GDBParser_ = 0;

GDBParser *GDBParser::getGDBParser()
{
  if (!GDBParser_)
    GDBParser_ = new GDBParser();

  return GDBParser_;
}

// **************************************************************************

void GDBParser::destroy()
{
    delete GDBParser_;
    GDBParser_ = 0;
}

// **************************************************************************

GDBParser::GDBParser()
{
}

// **************************************************************************

GDBParser::~GDBParser()
{
}


// **************************************************************************

QString GDBParser::getName(const char **buf)
{
    const char *start = skipNextTokenStart(*buf);
    if (*start) {
        *buf = skipTokenValue(start);
        return QCString(start, *buf - start + 1);
    } else
        *buf = start;

    return QString();
}

// **************************************************************************

QString GDBParser::getValue(const char **buf)
{
    const char *start = skipNextTokenStart(*buf);
    *buf = skipTokenValue(start);

    QString value(QCString(start, *buf - start + 1).data());
    return value;
}

QString GDBParser::undecorateValue(DataType type, const QString& s)
{
    QCString l8 = s.local8Bit();
    const char* start = l8;
    const char* end = start + s.length();

    if (*start == '{')
    {
        // Gdb uses '{' in two cases:
        // - composites (arrays and structures)
        // - pointers to functions. In this case type is
        //   enclosed in "{}". Not sure why it's so, as
        //   when printing pointer, type is in parenthesis.
        if (type == typePointer)
        {
            // Looks like type in braces at the beginning. Strip it.
            start = skipDelim(start, '{', '}');
        }
        else
        {
            // Looks like composite, strip the braces and return.
            return QCString(start+1, end - start -1);
        }
    }
    else if (*start == '(')
    {
        // Strip the type of the pointer from the value.
        //
        // When printing values of pointers, gdb prints the pointer
        // type as well. This is not necessary for kdevelop -- after
        // all, there's separate column with value type. But that behaviour
        // is not configurable. The only way to change it is to explicitly
        // pass the 'x' format specifier to the 'print' command. 
        //
        // We probably can achieve that by sending an 'print in hex' request
        // as soon as we know the type of variable, but that would be complex
        // and probably conflict with 'toggle hex/decimal' command.
        // So, address this problem as close to debugger as possible.

        // We can't find the first ')', because type can contain '(' and ')'
        // characters if its function pointer. So count opening and closing
        // parentheses.

        start = skipDelim(start, '(', ')');
    }

    QString value(QCString(start, end - start + 1).data());

    value = value.stripWhiteSpace();

    if (value[0] == '@')
    {
        // It's a reference, we need to show just the value.
        if (int i = value.find(":"))
        {
            value = value.mid(i+2);
        }
        else
        {
            // Just reference, no value at all, remove all
            value = "";
        }
    }

    if (value.find("Cannot access memory") == 0)
        value = "(inaccessible)";
  
    return value.stripWhiteSpace();
}

QString GDBParser::undecorateValue(const QString& s)
{
    DataType dataType = determineType(s.local8Bit());
    QString r = undecorateValue(dataType, s.local8Bit());
    return r;
}

// Given a value that starts with 0xNNNNNN determines if
// it looks more like pointer, or a string value.
DataType pointerOrValue(const char *buf)
{
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


DataType GDBParser::determineType(const char *buf) const
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
                    return typeArray;       // Hmm a single element array??
                if (strncmp(buf+1, " 0x", 3) == 0)
                    return typePointer;     // What about references?
                return typeUnknown;         // very odd?
            case '(':
                buf = skipDelim(buf, '(', ')');
                break;
            case '<':
                buf = skipDelim(buf, '<', '>');
                // gdb may produce this output:
                // $1 = 0x804ddf3 ' ' <repeats 20 times>, "TESTSTRING"
                // after having finished with the "repeats"-block we need
                // to check if the string continues
                if ( buf[0] == ',' && (buf[2] == '\"' || buf[2] == '\'') ) {
                    buf++; //set the buffer behind the comma to indicate that the string continues
                }
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
        return pointerOrValue(buf);
    }

    // Pointers and references - references are a bit odd
    // and cause GDB to fail to produce all the local data
    // if they haven't been initialised. but that's not our problem!!
    // (void (*)(void)) 0x804a944 <f(E *, char)> - this is a fn pointer
    if (*buf == '(') {
        buf = skipDelim(buf, '(', ')');
        // This 'if' handles values like this:
        // (int (&)[3]) @0xbffff684: {5, 6, 7}
        // which is a reference to array.
        if (buf[1] == '@')
            return typeReference;
        // This 'if' handles values like this:
        // (int (*)[3]) 0xbffff810
        if (strncmp(buf, " 0x", 3) == 0)
        {
            ++buf;
            return pointerOrValue(buf);
        }

        switch (*(buf-2)) {
        case '*':
            return typePointer;
        case '&':
            return typeReference;
        default:
            switch (*(buf-8)) {
                case '*':
                    return typePointer;
                case '&':
                    return typeReference;
            }
            return typeUnknown;
        }
    }

    buf = skipTokenValue(buf);
    if ((strncmp(buf, " = ", 3) == 0) || (*buf == '='))
        return typeName;

    return typeValue;
}

// **************************************************************************

const char *GDBParser::skipString(const char *buf) const
{
    if (buf && *buf == '\"') {
        buf = skipQuotes(buf, *buf);
        while (*buf) {
            if ((strncmp(buf, ", \"", 3) == 0) ||
                (strncmp(buf, ", '", 3) == 0))
                buf = skipQuotes(buf+2, *(buf+2));
            else if (strncmp(buf, " <", 2) == 0)  // take care of <repeats
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

const char *GDBParser::skipQuotes(const char *buf, char quotes) const
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

const char *GDBParser::skipDelim(const char *buf, char open, char close) const
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

const char *GDBParser::skipTokenValue(const char *buf) const
{
    if (buf) {
        while (true) {
            buf = skipTokenEnd(buf);

            const char *end = buf;
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

const char *GDBParser::skipTokenEnd(const char *buf) const
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
            buf = skipDelim(buf, '<', '>');
            // gdb may produce this output:
            // $1 = 0x804ddf3 ' ' <repeats 20 times>, "TESTSTRING"
            // after having finished with the "repeats"-block we need
            // to check if the string continues
            if ( buf[0] == ',' && (buf[2] == '\"' || buf[2] == '\'') ) {
                buf++; //set the buffer behind the comma to indicate that the string continues
            }
            return buf;
        case '(':
            return skipDelim(buf, '(', ')');
        }

        while (*buf && !isspace(*buf) && *buf != ',' && *buf != '}' && *buf != '=')
            buf++;
    }

    return buf;
}

// **************************************************************************

const char *GDBParser::skipNextTokenStart(const char *buf) const
{
    if (buf)
        while (*buf && (isspace(*buf) || *buf == ',' || *buf == '}' || *buf == '='))
            buf++;

    return buf;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

}
