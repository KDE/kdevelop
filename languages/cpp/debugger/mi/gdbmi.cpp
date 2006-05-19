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
 *  You should have received a copy of the GNU Library General Public      *
 *  License along with this program; if not, write to the                  *
 *  Free Software Foundation, Inc.,                                        *
 *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
 ***************************************************************************/
#include "gdbmi.h"

using namespace GDBMI;


type_error::type_error()
: std::logic_error("MI type error")
{}

QString Value::literal() const
{
    throw type_error();
}

int Value::toInt(int base) const
{
    throw type_error();
}

bool Value::hasField(const QString&) const
{
    throw type_error();
}

const Value& Value::operator[](const QString&) const
{
    throw type_error();
}

bool Value::empty() const
{
    throw type_error();
}

unsigned Value::size() const
{
    throw type_error();
}


const Value& Value::operator[](unsigned) const
{
    throw type_error();
}

QString StringLiteralValue::literal() const
{
    return literal_;
}

int StringLiteralValue::toInt(int base) const
{
    bool ok;
    int result = literal_.toInt(&ok, base);
    if (!ok)
        throw type_error();
    return result;
}

TupleValue::~TupleValue()
{
    for (QValueListIterator<Result*> it=results.begin(); it!=results.end(); ++it)
        delete *it;
}

bool TupleValue::hasField(const QString& variable) const
{
    return results_by_name.count(variable);
}

const Value& TupleValue::operator[](const QString& variable) const
{ 
    if (results_by_name.count(variable))
        return *results_by_name[variable]->value;
    else 
        throw type_error();
}

ListValue::~ListValue()
{
    for (QValueListIterator<Result*> it=results.begin(); it!=results.end(); ++it)
        delete *it;
}

bool ListValue::empty() const
{
    return results.isEmpty();
}

unsigned ListValue::size() const
{
    return results.size();
}

const Value& ListValue::operator[](unsigned index) const
{
    if (index < results.size())
    {
        return *results[index]->value;
    }
    else
        throw type_error();
}




