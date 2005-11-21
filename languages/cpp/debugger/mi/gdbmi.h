/***************************************************************************
 *   Copyright (C) 2004 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GDBMI_H
#define GDBMI_H

#include <qstring.h>
#include <qvaluelist.h>

/**
@author Roberto Raggi
*/
class GDBMI
{
public:
    inline static QString quote(const QString &text)
    { return "\"" + text + "\""; }

    struct Value
    {
        virtual ~Value() {}

        virtual QString toString() const { Q_ASSERT( 0 ); return QString::null; }

        enum { StringLiteral, Tuple, List } kind;
    };

    struct Result
    {
        inline Result() : value(0) {}
        inline ~Result() { delete value; value = 0; }

        inline QString toString() const
        {
            QString out;
            if (!!variable) {
                out += variable;
                out += '=';
            }

            if (value)
                out += value->toString();

            return out;
        }

        QString variable;
        Value *value;
    };

    struct StringLiteralValue : public Value
    {
        inline StringLiteralValue(const QString &lit)
            : literal(lit) { Value::kind = StringLiteral; }

        virtual QString toString() const { return literal; }

        QString literal;
    };

    struct TupleValue : public Value
    {
        inline TupleValue() { Value::kind = Tuple; }
        inline ~TupleValue()
        {
            for (QValueListIterator<Result*> it=results.begin(); it!=results.end(); ++it)
                delete *it;
        }

        virtual QString toString() const
        {
            QString out = "{";

            QValueListConstIterator<Result*> it=results.begin();
            while (it != results.end()) {
                out += (*it)->toString();
                ++it;

                if (it != results.end())
                    out += ", ";
            }
            out += "}";
            return out;
        }

        QValueList<Result*> results;
    };

    struct ListValue : public Value
    {
        inline ListValue() { Value::kind = List; }
        inline ~ListValue()
        {
            for (QValueListIterator<Result*> it=results.begin(); it!=results.end(); ++it)
                delete *it;
        }

        virtual QString toString() const
        {
            QString out = "[";
            QValueListConstIterator<Result*> it=results.begin();
            while (it != results.end()) {
                out += (*it)->toString();
                ++it;

                if (it != results.end())
                    out += ", ";
            }
            out += "]";
            return out;
        }

        QValueList<Result*> results;
    };

    struct Record
    {
        virtual ~Record() {}
        virtual QString toString() const { Q_ASSERT( 0 ); return QString::null; }

        enum { Prompt, Stream, Result } kind;
    };

    struct ResultRecord : public Record
    {
        inline ResultRecord() { Record::kind = Result; }
        virtual ~ResultRecord()
        {
            for (QValueListIterator<GDBMI::Result*> it=results.begin(); it!=results.end(); ++it)
                delete *it;
        }

        virtual QString toString() const
        {
            QString out;
            out += "(";
            out += reason;
            out += ", (";
            QValueListConstIterator<GDBMI::Result*> it=results.begin();
            while (it != results.end()) {
                out += (*it)->toString();
                ++it;

                if (it != results.end())
                    out += ", ";
            }
            out += "))\n";
            return out;
        }

        QString reason;
        QValueList<GDBMI::Result*> results;
    };

    struct PromptRecord : public Record
    {
        inline PromptRecord() { Record::kind = Prompt; }

        virtual QString toString() const
        { return "(prompt)\n"; }
    };

    struct StreamRecord : public Record
    {
        inline StreamRecord() : reason(0) { Record::kind = Stream; }

        virtual QString toString() const
        {
            QString out;
            out += "(";
            out += reason;
            out += ", (";
            out += quote(message);
            out += "))\n";
            return out;
        }

        char reason;
        QString message;
    };
};

#endif
