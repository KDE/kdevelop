/***************************************************************************
 *   Copyright (C) 2004 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2005-2006 by Vladimir Prus                              *
 *   ghost@cs.msu.su                                                       *
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
#include <qmap.h>

#include <stdexcept>

/**
@author Roberto Raggi
@author Vladimir Prus
*/
namespace GDBMI
{
    /** Exception that is thrown when we're trying to invoke an
        operation that is not supported by specific MI value. For
        example, trying to index a string literal.

        Such errors are conceptually the same as assert, but in GUI
        we can't use regular assert, and Q_ASSERT, which only prints
        a message, is not suitable either. We need to break processing,
        and the higher-level code can report "Internal parsing error",
        or something.

        Being glorified assert, this exception does not cary any
        useful information.
    */
    class type_error : public std::logic_error
    {
    public:
        type_error();
    };

    /** Base class for all MI values.
        MI values are of three kinds:
        - String literals
        - Lists (indexed by integer)
        - Tuple (set of named values, indexed by name)

        The structure of response to a specific gdb command is fixed.
        While any tuples in response may omit certain named fields, the
        kind of each item never changes. That is, response to specific
        command can't contains sometimes string and sometimes tuple in
        specific position.

        Because of that static structure, it's almost never needed to query
        dynamic type of a MI value. Most often we know it's say, tuple, and
        can subscripts it. 

        So, the Value class has methods for accessing all kinds of values.
        Attempting to call a method that is not applicable to specific value
        will result in exception. The client code will almost never need to
        cast from 'Value' to its derived classes.

        Note also that all methods in this class are const and return 
        const Value&. That's by design -- there's no need to modify gdb
        responses in GUI.
     */
    struct Value
    {
        Value() {}
    private: // Copy disabled to prevent slicing.
        Value(const Value&);
        Value& operator=(const Value&);

    public:

        virtual ~Value() {}

        enum { StringLiteral, Tuple, List } kind;

        /** If this value is a string literals, returns the string value.
            Othewise, throws type_error.
        */
        virtual QString literal() const;

        /** If the value is a string literal, converts it to int and
            returns. If conversion fails, or the value cannot be
            converted to int, throws type_error.
        */
        virtual int toInt(int base = 10) const;
            
        /** If this value is a tuple, returns true if the tuple
            has a field named 'variable'. Otherwise,
            throws type_error.
        */
        virtual bool hasField(const QString& variable) const;

        /** If this value is a tuple, and contains named field 'variable',
            returns it. Otherwise, throws 'type_error'.
            This method is virtual, and derived in base class, so that
            we can save on casting, when we know for sure that instance
            is TupleValue, or ListValue.
        */
        virtual const Value& operator[](const QString& variable) const;

        /** If this value is a list, returns true if the list is empty.
            If this value is not a list, throws 'type_error'.
        */
        virtual bool empty() const;

        /** If this value is a list, returns it's size.
            Otherwise, throws 'type_error'.
        */
        virtual unsigned size() const;

        /** If this value is a list, returns the element at
            'index'. Otherwise, throws 'type_error'.
        */
        virtual const Value& operator[](unsigned index) const;
    };

    /** @internal
        Internal class to represent name-value pair in tuples.
    */
    struct Result
    {
        Result() : value(0) {}
        ~Result() { delete value; value = 0; }

        QString variable;
        Value *value;
    };

    struct StringLiteralValue : public Value
    {
        StringLiteralValue(const QString &lit)
            : literal_(lit) { Value::kind = StringLiteral; }

    public: // Value overrides

        QString literal() const;
        int toInt(int base) const;
     
    private:
        QString literal_;
    };

    struct TupleValue : public Value
    {
        TupleValue() { Value::kind = Tuple; }
        ~TupleValue();

        bool hasField(const QString&) const;
        const Value& operator[](const QString& variable) const;


        QValueList<Result*> results;
        QMap<QString, GDBMI::Result*> results_by_name;
    };

    struct ListValue : public Value
    {
        ListValue() { Value::kind = List; }
        ~ListValue();

        bool empty() const;

        unsigned size() const;

        const Value& operator[](unsigned index) const;

        QValueList<Result*> results;

    };

    struct Record
    {
        virtual ~Record() {}
        virtual QString toString() const { Q_ASSERT( 0 ); return QString::null; }

        enum { Prompt, Stream, Result } kind;
    };

    struct ResultRecord : public Record, public TupleValue
    {
        ResultRecord() { Record::kind = Result; }

        QString reason;
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

        char reason;
        QString message;
    };
}

#endif
